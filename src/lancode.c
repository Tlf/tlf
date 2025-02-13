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

#include "clear_display.h"
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
bool cl_send_inhibit = false;
struct sockaddr_in bc_address[MAXNODES];
/* host names and UDP ports to send notifications to */
char bc_hostaddress[MAXNODES][16];
int bc_hostport[MAXNODES];
int nodes = 0;
bool using_named_nodes;
//--------------------------------------
/* default port to listen for incoming packets and to send packet to */
/* can be changed using LAN_PORT config */
int lan_port;

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
bool landebug = false;
bool time_master;
char thisnode = 'A'; 		/*  start with 'A' if not defined in
				    logcfg.dat */

//---------------------end lan globals --------------

int lan_recv_init(void) {
    int lan_bind_rc;
    long lan_save_file_flags;
    struct sockaddr_in lan_sin;

    if (!lan_active)
	return 0;

    int node = thisnode - 'A';
    int port = (bc_hostport[node] > 0 ? bc_hostport[node] : lan_port);

    bzero(&lan_sin, sizeof(lan_sin));
    lan_sin.sin_family = AF_INET;
    lan_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    lan_sin.sin_port = htons(port);

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
	cl_send_inhibit = true;	// this node does not send cluster info

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
	if (*bc_hostaddress[node] == 0) {
	    continue;
	}
	if (using_named_nodes && node == thisnode - 'A') {
	    continue;   // skip ourserlves
	}

	bc_hostbyname[node] = gethostbyname(bc_hostaddress[node]);
	if (bc_hostbyname[node] == NULL) {
	    syslog(LOG_ERR, "%s\n", "LAN: gethostbyname failed");
	    return -1;
	}

	bzero(&bc_address[node], sizeof(bc_address[node]));	/* empty data structure */
	bc_address[node].sin_family = AF_INET;
	memcpy(&bc_address[node].sin_addr.s_addr, bc_hostbyname[node]->h_addr,
	       sizeof(bc_address[node].sin_addr.s_addr));

	int port = (bc_hostport[node] > 0 ? bc_hostport[node] : lan_port);
	bc_address[node].sin_port = htons(port);

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
	if (*bc_hostaddress[node] == 0) {
	    continue;
	}
	if (using_named_nodes && node == thisnode - 'A') {
	    continue;   // skip ourserlves
	}

	bc_sendto_rc = sendto(bc_socket_descriptor[node],
			      lanbuffer, strlen(lanbuffer),
			      0, (struct sockaddr *) &bc_address[node],
			      sizeof(bc_address[node]));


	if (bc_sendto_rc == -1) {
	    if (send_error[node] >= (send_error_limit[node] + 10)) {
		TLF_SHOW_INFO("LAN: send problem...!");
		send_error_limit[node] += 10;
	    } else
		send_error[node]++;

	} else
	    send_packets[node]++;
    }

    return 0;
}

/* ----------------- send lan message ----------*/

void send_lan_message(int opcode, char *message) {
    char sendbuffer[102];

    sendbuffer[0] = thisnode;
    sendbuffer[1] = opcode;
    sendbuffer[2] = '\0';
    strncat(sendbuffer, message, 98);
    if (opcode == CLUSTERMSG) {
	if (!cl_send_inhibit) {
	    strcat(sendbuffer, "\n");
	    lan_send(sendbuffer);
	}
    }

    if (opcode == LOGENTRY) {
	strcat(sendbuffer, "\n");
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
	strcat(sendbuffer, "\n");
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

    return;
}

/* ----------------- send talk message ----------*/
void talk(void) {

    char talkline[61] = "";

    clear_line(LINES - 1);
    mvaddstr(LINES - 1, 0, "T>");
    refreshp();
    echo();
    getnstr(talkline, 60);
    noecho();

    strcat(talkline, "\n");

    send_lan_message(TLFMSG, talkline);

    talkline[0] = '\0';
    attron(COLOR_PAIR(C_HEADER));
    clear_line(LINES - 1);
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
