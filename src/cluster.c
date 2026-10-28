#include <ctype.h>
#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/tcp.h>

#include "tlf.h"
#include "cluster.h"
#include "getwwv.h"
#include "globalvars.h"
#include "showmsg.h"
#include "err_utils.h"
#include "lancode.h"

pthread_mutex_t spot_ptr_mutex = PTHREAD_MUTEX_INITIALIZER;
int cluster_state = CLUSTER_DISCONNECTED;

static bool initialized = false;

static GAsyncQueue *rx_queue;
static GAsyncQueue *tx_queue;

static bool go = true;
static char line_data[100];

#define READ_SIZE               1024

#define POLL_TIMEOUT_MS         100
#define CONNECT_TIMEOUT_MS      3000
#define RECONNECT_DELAY_S       10

#define TCP_KEEPIDLE_VALUE      30  // idle seconds before starting keepalive
#define TCP_KEEPINTVL_VALUE     10  // interval between probes
#define TCP_KEEPCNT_VALUE       3   // number of failed probes before declaring dead


//
// return: to be g_free()'d by the caller
//
char *get_cluster_target() {
    char *target = NULL;
    switch (packetinterface) {
	case TELNET_INTERFACE:
	    target = g_strdup_printf("%s:%d", pr_hostaddress, portnum);
	    break;

	case TNC_INTERFACE:
	    target = g_strdup_printf("%s at %d Baud", tncportname, tnc_serial_rate);
	    break;

	case FIFO_INTERFACE:
	    target = g_strdup("FIFO");
	    break;

	default:
	    target = g_strdup("n/a");   // unknown interface, shall not happen
    }
    return target;
}

//
// to be called at the init phase
//
void cluster_init() {
    if (initialized) {
	return;
    }

    initialized = true;

    rx_queue = g_async_queue_new_full(g_free);
    tx_queue = g_async_queue_new_full(g_free);

    char *target = get_cluster_target();
    showstring("Cluster is", target);
    g_free(target);
}

void cleanup_telnet(void) { // fix name: cleanup_packet
    if (!initialized) {
	return;
    }

    g_async_queue_unref(rx_queue);
    rx_queue = NULL;
    g_async_queue_unref(tx_queue);
    tx_queue = NULL;

    //... CFIXME

    initialized = false;
}

// CFIXME
void send_to_cluster(const char *line) {
#if 0
    if (packetinterface == TNC_INTERFACE) {
	line[strlen(line) - 1] = '\r';
	line[strlen(line)] = '\0';	/* not needed */
	IGNORE(write(fdSertnc, line, strlen(line)));;

    } else if ((packetinterface == TELNET_INTERFACE) && (prsock > 0))
	usputs(prsock, line);
#endif
}

const char CLUSTER_TEXT = 'C';
const char OTHER_TEXT = '-';

//
// add text to cluster RX queue
//
// 'type' controls whether the text is propagated
// to other LAN nodes during processing.
//
static void add_text(const char *s, char type) {
    g_async_queue_push(rx_queue, g_strdup_printf("%c%s", type, line_data));
}

void add_cluster_text(const char *s) {
    add_text(s, CLUSTER_TEXT);
}

void add_other_text(const char *s) {
    add_text(s, OTHER_TEXT);
}

//
// process a line from cluster
// (called from receive_packet() in background thread)
//
static void process_cluster_record(const char *record) {

    if (strlen(record) < 1) {   // shall not happen
	return;
    }

    const char *s = record + 1;

    if (record[0] == CLUSTER_TEXT && strlen(s) > 20) {
	send_lan_message(CLUSTERMSG, s);
    }

    pthread_mutex_lock(&spot_ptr_mutex);

    g_strlcpy(spot_ptr[nr_of_spots], s, sizeof(spot_ptr[0]));
    nr_of_spots++;

    if (nr_of_spots > MAX_SPOTS - 1) {
	int idx;
	for (idx = 10; idx <= MAX_SPOTS - 1; idx++)
	    memcpy(spot_ptr[idx - 10], spot_ptr[idx], sizeof(spot_ptr[0])); // check this
	for (idx = MAX_SPOTS - 1; idx >= MAX_SPOTS - 11; idx--)
	    spot_ptr[idx][0] = '\0';
	nr_of_spots = MAX_SPOTS - 10;
    }

    pthread_mutex_unlock(&spot_ptr_mutex);

    if (clusterlog) {
	FILE *fp;
	if ((fp = fopen("clusterlog", "a")) == NULL) {
	    TLF_LOG_ERR("Opening clusterlog not possible.");
	} else {
	    if (strlen(s) > 20) {
		fputs(s, fp);
		fputs("\n", fp);
	    }

	    fclose(fp);
	}
    }

    // \todo drop it later tb mar11
    bm_add(s);

    wwv_add(s);
}

//
// process all available records from rx_queue
// (called from background thread)
//
void receive_packet(void) {
    if (rx_queue == NULL) {
	return;
    }
    char *record;
    while ((record = g_async_queue_try_pop(rx_queue)) != NULL) {
	process_cluster_record(record);
	g_free(record);
    }
}

// --- start private functions ---

static void flush_queue(GAsyncQueue *queue) {
    char *data;
    while ((data = g_async_queue_try_pop(queue)) != NULL) {
	g_free(data);
    }
}


static int create_socket(int domain, int type, int protocol) {

    int sock = socket(domain, type | SOCK_CLOEXEC, protocol);
    if (sock < 0) {
	return -1;
    }

    /* Enable TCP keepalive */
    int enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

    int idle = TCP_KEEPIDLE_VALUE;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));

    int intvl = TCP_KEEPINTVL_VALUE;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));

    int cnt = TCP_KEEPCNT_VALUE;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));

    return sock;
}


static int socket_connect_with_timeout(int sock, const struct sockaddr *addr,
				       socklen_t addrlen) {

    struct pollfd pfd;
    pfd.fd = sock;

    fcntl(sock, F_SETFL, O_NONBLOCK);
    connect(sock, addr, addrlen);   // start a non-blocking connect

    const int iterations = CONNECT_TIMEOUT_MS / POLL_TIMEOUT_MS;

    for (int i = 0; i < iterations && go; ++i) {

	pfd.events = POLLOUT;

	int rc = poll(&pfd, 1, POLL_TIMEOUT_MS);

	if (rc < 0) {   // can't poll, quit the loop
	    break;
	}

	if (rc == 0) {  // timeout, try again
	    continue;
	}

	if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
	    break;      // some error happened, quit the loop
	}

	if (pfd.revents & POLLOUT) {
	    int fl = fcntl(sock, F_GETFL);
	    fcntl(sock, F_SETFL, fl & ~O_NONBLOCK); // put back to blocking
	    return 0;   // success
	}
    }

    return -1;
}

static int tcp_connect(char *hostname, int portno) {
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    char portstr[20];
    sprintf(portstr, "%d", portno); // use g_sprintf?

    int rc = getaddrinfo(hostname, portstr, &hints, &result);

    if (rc != 0) {
	//fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
	return -1;
    }

    int sock = -1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {

	sock = create_socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

	if (sock < 0) {
	    continue;
	}

	if (socket_connect_with_timeout(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
	    break;
	}

	close(sock);
	sock = -1;
    }

    freeaddrinfo(result);

    if (sock >= 0) {
	TLF_LOG_DEBUG("connect OK");
    } else {
	TLF_LOG_DEBUG("connect failed");
    }

    return sock;
}

static int get_serial_speed() {
    switch (tnc_serial_rate) {
	case 1200:  return B1200;
	case 2400:  return B2400;
	case 4800:  return B4800;
	case 9600:  return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
    }

    TLF_LOG_ERR("Invalid serial speed %d", tnc_serial_rate);

    return -1;
}

static int serial_open(char *filename) {
    int speed = get_serial_speed();
    if (speed < 0) {
	return -1;
    }

    int fd;
    if ((fd = open(tncportname, O_RDWR | O_NONBLOCK | FD_CLOEXEC)) < 0) {
	return -1;
    }

    struct termios termattribs;
    termattribs.c_iflag = IGNBRK | IGNPAR | IMAXBEL | IXOFF;
    termattribs.c_oflag = 0;
    termattribs.c_cflag = CS8 | CSTOPB | CREAD | CLOCAL;
    termattribs.c_lflag = 0;

    termattribs.c_cc[VMIN] = 1;
    termattribs.c_cc[VTIME] = 0;

    cfsetispeed(&termattribs, speed);
    cfsetospeed(&termattribs, speed);

    tcsetattr(fd, TCSANOW, &termattribs);   // set-up the serial port

    return fd;
}


#define FIFO_FILE   "clfile"

static int fifo_open() {

    // ensure the file is either an existing FIFO or does not exist
    struct stat sb;
    if (stat(FIFO_FILE, &sb) == 0 && !S_ISFIFO(sb.st_mode)) {
	return -1; // exists, but it's not a FIFO
    }

    const mode_t mode = S_IRUSR | S_IWUSR;

    if (mkfifo("clfile", mode) < 0) {
	TLF_LOG_DEBUG("FIFO " FIFO_FILE " exists");
    } else {
	TLF_LOG_DEBUG("FIFO " FIFO_FILE " created");
    }

    // open in RW mode; this ensures the file descriptor remains valid and
    // avoids EOF (and unnecessary reconnection) after the writer terminates
    // note: do not actually write to it, as it'll be read back
    int fd = open(FIFO_FILE, O_RDWR | O_NONBLOCK | FD_CLOEXEC);

    if (fd >= 0) {
	TLF_LOG_DEBUG("FIFO open OK");
    } else {
	TLF_LOG_DEBUG("FIFO open failed");
    }

    return fd;
}

static int cluster_connect() {
    if (packetinterface == TELNET_INTERFACE) {
	return tcp_connect(pr_hostaddress, portnum);
    }
    if (packetinterface == TNC_INTERFACE) {
	return serial_open(tncportname);
    }
    if (packetinterface == FIFO_INTERFACE) {
	return fifo_open();
    }

    return -1;
}

//
// enqueue data line-by-line into rx_queue
//
static void process_data(char *data, int size) {

    int len = strlen(line_data);    // the rest from previous call

    for (int i = 0; i < size; ++i) {
	bool flush = false;
	char c = data[i];
	if (c == '\r' || c == '\n') {
	    flush = true;
	} else {
	    if (!isascii(c)) {
		c = '.';
	    }
	    line_data[len] = c;      // append character
	    line_data[len + 1] = 0;
	    ++len;
	    if (len == sizeof(line_data) - 1) {
		flush = true;
	    }
	}

	if (flush && len > 0) {
	    TLF_LOG_DEBUG("rcv: |%s|", line_data);
	    // enqueue line, unless queue is too long
	    if (g_async_queue_length(rx_queue) < 1000) {
		add_cluster_text(line_data);
	    }

	    line_data[0] = 0;
	    len = 0;
	}
    }
}

//
// process data from fd, exit on connection closed or error
//
static void client_loop(int fd) {
    struct pollfd pfd;
    pfd.fd = fd;

    // clear queues and line buffer
    flush_queue(rx_queue);
    flush_queue(tx_queue);
    line_data[0] = 0;

    if (packetinterface == TELNET_INTERFACE && strlen(clusterlogin) != 0) {
	// NOTE: static string
	g_async_queue_push(tx_queue, g_strdup(clusterlogin));
    }

    while (go) {

	pfd.events = POLLIN |
		     (g_async_queue_length(tx_queue) > 0 ? POLLOUT : 0);

	int rc = poll(&pfd, 1, POLL_TIMEOUT_MS);

	if (rc < 0) {   // can't poll, quit the loop
	    break;
	}

	if (rc == 0) {  // timeout, try again
	    continue;
	}

	if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
	    break;      // some error happened, quit the loop
	}

	if (pfd.revents & POLLIN) {

	    char buffer[READ_SIZE];
	    ssize_t received = read(fd, buffer, sizeof(buffer));

	    if (received <= 0) {    // EOF or error
		break;
	    }

	    process_data(buffer, received);
	}

	if (pfd.revents & POLLOUT) {

	    char *data = g_async_queue_try_pop(tx_queue);

	    // empty data: disconnect
	    if (strlen(data) == 0) {
		g_free(data);
		break;
	    }

	    int rc = write(fd, data, strlen(data));
	    // send \n ?
	    g_free(data);
	    if (rc < 0) {
		break;
	    }
	    rc = write(fd, "\r", 1);
	    if (rc < 0) {
		break;
	    }
	}
    }
}

static void interruptible_sleep(int secs) {
    for (int i = 0; i < (secs * 1000) / POLL_TIMEOUT_MS; ++i) {
	if (!go) {
	    break;
	}
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 1000 * 1000 * POLL_TIMEOUT_MS;
	if (nanosleep(&ts, NULL) != 0) {
	    break;
	}
    }
}

// --- end private functions ---

void cluster_disable() {
    cluster_state = CLUSTER_DISABLED;
}

void terminate_cluster_process() {
    go = false;
}

void *cluster_process(void *ptr) {

    int delay = 0;

    while (go) {

	if (delay != 0) {
	    TLF_LOG_DEBUG("sleeping %d", delay);
	    cluster_state = CLUSTER_SLEEP;
	    interruptible_sleep(delay);
	}

	cluster_state = CLUSTER_CONNECTING;

	int fd = cluster_connect();

	delay = RECONNECT_DELAY_S;  // for an eventual reconnect

	if (fd < 0) {
	    continue;
	}

	cluster_state = CLUSTER_CONNECTED;

	client_loop(fd);

	close(fd);
	cluster_state = CLUSTER_DISCONNECTED;
    }

    pthread_exit(NULL);
}

