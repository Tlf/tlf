/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 		 2013 		Fred DH5FS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "err_utils.h"
#include "get_time.h"
#include "globalvars.h"
#include "lancode.h"
#include "tlf.h"
#include "tlf_curses.h"


int lan_socket_descriptor;
char lan_message[256];
//--------------------------------------
int bc_socket_descriptor[MAXNODES];
ssize_t bc_sendto_rc;
int cl_send_inhibit = 0;
struct sockaddr_in bc_address[MAXNODES];
/* host names and UDP ports to send notifications to */
char bc_hostaddress[MAXNODES][16];
char bc_hostservice[MAXNODES][16] = {
    [0 ... MAXNODES - 1] = { [0 ... 15] = 0 }
};
int nodes = 0;
//--------------------------------------
/* default port to listen for incomming packets and to send packet to */
char default_lan_service[16] = "6788";
/* lan port parsed from config */
int lan_port = 6788;

bool lan_active = false;
int send_error[MAXNODES];
int send_error_limit[MAXNODES];
int lan_mutex = 0;
int send_packets[MAXNODES];
int recv_error;
int recv_packets;
char talkarray[5][62];
freq_t node_frequencies[MAXNODES];
int highqsonr;
int landebug = 0;
int time_master;
char thisnode = 'A'; 		/*  start with 'A' if not defined in
				    logcfg.dat */

//---------------------end lan globals --------------

int resolveService(const char *service) {
    struct servent *service_ent;
    service_ent = getservbyname(service, "udp");
    int port = 0;
    if (service_ent != NULL) {
	port = ntohs(service_ent->s_port);
    } else if (strlen(service) > 0) {
	port = atoi(service);
    }
    if (port == 0) {
	port = atoi(default_lan_service);
    }
    return port;
}

int lan_recv_init(void) {
    int lan_bind_rc;
    long lan_save_file_flags;
    struct sockaddr_in lan_sin;

    if (!lan_active)
	return 0;

    sprintf(default_lan_service, "%d", lan_port);
    bzero(&lan_sin, sizeof(lan_sin));
    lan_sin.sin_family = AF_INET;
    lan_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    lan_sin.sin_port = htons(resolveService(default_lan_service));

    lan_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (lan_socket_descriptor == -1) {
	syslog(LOG_ERR, "%s\n", "LAN: socket");
	return -1;
    }

    lan_bind_rc =
	bind(lan_socket_descriptor, (struct sockaddr *) &lan_sin,
	     sizeof(lan_sin));
    if (lan_bind_rc == -1) {
	syslog(LOG_ERR, "%s\n", "LAN: bind");
	return -2;
    }

    lan_save_file_flags = fcntl(lan_socket_descriptor, F_GETFL);
    lan_save_file_flags |= O_NONBLOCK;
    if (fcntl(lan_socket_descriptor, F_SETFL, lan_save_file_flags) == -1) {
	syslog(LOG_ERR, "%s\n", "trying non-blocking");
	return -3;
    }
    return 0;
}

int lan_recv_close(void) {
    int lan_close_rc;

    if (!lan_active)
	return 0;

    lan_close_rc = close(lan_socket_descriptor);
    if (lan_close_rc == -1) {
	syslog(LOG_ERR, "%s\n", "LAN: close call failed");
	return errno;
    }

    return 0;
}

int lan_recv(void) {
    ssize_t lan_recv_rc;
    struct sockaddr_in lan_sin;
    unsigned int lan_sin_len = sizeof(lan_sin);
    char lan_recv_message[256];

    if (!lan_active)
	return 0;

    lan_recv_message[0] = '\0';

    lan_recv_rc =
	recvfrom(lan_socket_descriptor, lan_recv_message,
		 sizeof(lan_recv_message), 0, (struct sockaddr *) &lan_sin,
		 &lan_sin_len);

    if (lan_recv_rc == -1 && errno != EAGAIN) {
	recv_error++;
	return errno;
    }

    errno = 0;			/* clear the error */

    if (lan_recv_message[1] == CLUSTERMSG)
	cl_send_inhibit = 1;	// this node does not send cluster info

    if (lan_recv_rc > 0)
	recv_packets++;

    strcpy(lan_message, lan_recv_message);

    return 0;
}

// ----------------send routines --------------------------

int lan_send_init(void) {
    struct hostent *bc_hostbyname[MAXNODES];

    if (!lan_active)
	return 0;

    for (int node = 0; node < nodes; node++) {

	bc_hostbyname[node] = gethostbyname(bc_hostaddress[node]);
	if (bc_hostbyname[node] == NULL) {
	    syslog(LOG_ERR, "%s\n", "LAN: gethostbyname failed");
	    return -1;
	}

	bzero(&bc_address[node], sizeof(bc_address[node]));	/* empty data structure */
	bc_address[node].sin_family = AF_INET;
	memcpy(&bc_address[node].sin_addr.s_addr, bc_hostbyname[node]->h_addr,
	       sizeof(bc_address[node].sin_addr.s_addr));

	bc_address[node].sin_port = htons(resolveService(bc_hostservice[node]));

	syslog(LOG_INFO, "open socket: to %d.%d.%d.%d:%d\n",
	       (ntohl(bc_address[node].sin_addr.s_addr) & 0xff000000) >> 24,
	       (ntohl(bc_address[node].sin_addr.s_addr) & 0x00ff0000) >> 16,
	       (ntohl(bc_address[node].sin_addr.s_addr) & 0x0000ff00) >> 8,
	       (ntohl(bc_address[node].sin_addr.s_addr) & 0x000000ff) >> 0,
	       ntohs(bc_address[node].sin_port));

	bc_socket_descriptor[node] = socket(AF_INET, SOCK_DGRAM, 0);

	if (bc_socket_descriptor[node] == -1) {
	    syslog(LOG_ERR, "%s\n", "LAN: socket call failed");
	    return -1;
	}
    }

    return 0;
}

int lan_send_close(void) {
    int bc_close_rc;

    if (!lan_active)
	return 0;

    for (int node = 0; node < nodes; node++) {

	bc_close_rc = close(bc_socket_descriptor[node]);
	if (bc_close_rc == -1) {
	    syslog(LOG_ERR, "%s\n", "LAN: close call failed");
	    return -1;
	}
    }

    return 0;
}

static int lan_send(char *lanbuffer) {

    if (!lan_active)
	return 0;

    if (lanbuffer[0] == 0) {
	return 0;       // nothing to send
    }

    for (int node = 0; node < nodes; node++) {

	bc_sendto_rc = sendto(bc_socket_descriptor[node],
			      lanbuffer, strlen(lanbuffer),
			      0, (struct sockaddr *) &bc_address[node],
			      sizeof(bc_address[node]));


	if (bc_sendto_rc == -1) {
	    if (send_error[node] >= (send_error_limit[node] + 10)) {
		TLF_LOG_INFO("LAN: send problem...!");
		send_error_limit[node] += 10;
	    } else
		send_error[node]++;

	} else
	    send_packets[node]++;
    }

    return 0;
}

/* ----------------- send lan message ----------*/

int send_lan_message(int opcode, char *message) {
    char sendbuffer[102];

    sendbuffer[0] = thisnode;
    sendbuffer[1] = opcode;
    sendbuffer[2] = '\0';
    strncat(sendbuffer, message, 98);
    if (opcode == CLUSTERMSG) {
	if (cl_send_inhibit == 0) {
	    strcat(sendbuffer, "\n");
	    lan_send(sendbuffer);
	}
    }

    if (opcode == LOGENTRY) {
	sendbuffer[82] = '\0';

	lan_send(sendbuffer);
    }

    if (opcode == TLFSPOT) {
	sendbuffer[82] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == TLFMSG) {
	sendbuffer[82] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == FREQMSG) {
	sendbuffer[10] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == INCQSONUM) {
	strcat(sendbuffer, "\n");
	lan_send(sendbuffer);
    }
    if (opcode == TIMESYNC) {
	sendbuffer[14] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == QTCRENTRY) {
	strcat(sendbuffer, "\n");
	sendbuffer[94] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == QTCSENTRY) {
	strcat(sendbuffer, "\n");
	sendbuffer[100] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == QTCFLAG) {
	strcat(sendbuffer, "\n");
	lan_send(sendbuffer);
    }

    return 0;
}

/* ----------------- send talk message ----------*/
void talk(void) {

    char talkline[61] = "";

    mvprintw(LINES - 1, 0, "%s", backgrnd_str);
    mvprintw(LINES - 1, 0, "T>");
    refreshp();
    echo();
    getnstr(talkline, 60);
    noecho();

    strcat(talkline, "\n");

    send_lan_message(TLFMSG, talkline);

    talkline[0] = '\0';
    attron(COLOR_PAIR(C_HEADER));
    mvprintw(LINES - 1, 0, "%s", backgrnd_str);
    refreshp();
}

/* ----------------- send freq. message ----------*/

int send_freq(freq_t freq) {

    char fbuffer[8];

    if (trx_control) {
	sprintf(fbuffer, "%7.1f", freq / 1000.0);
    } else if (bandinx < BANDINDEX_OOB) {
	sprintf(fbuffer, "  %s.0", band[bandinx]);
    } else {
	sprintf(fbuffer, "     ");
    }

    send_lan_message(FREQMSG, fbuffer);

    return 0;
}

/* ----------------- send time message ----------*/

void send_time(void) {

    char timebuffer[14];

    time_t now = get_time();    // note: time master send UTC (timecorr=0)

    sprintf(timebuffer, "%ld ", now);
    send_lan_message(TIMESYNC, timebuffer);
}
