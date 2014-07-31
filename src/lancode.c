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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "tlf.h"
#include "lancode.h"

int lan_socket_descriptor;
struct sockaddr_in lan_sin;
int lan_bind_rc, lan_close_rc;
ssize_t lan_recv_rc;
long lan_save_file_flags;
char lan_recv_message[256];
char lan_message[256];
char lan_logline[256];
unsigned int lan_sin_len;
//--------------------------------------
int bc_socket_descriptor[MAXNODES];
ssize_t bc_sendto_rc;
int bc_close_rc;
int cl_send_inhibit = 0;
char lanbuffer[255];
struct sockaddr_in bc_address[MAXNODES];
struct hostent *bc_hostbyname[MAXNODES];
/* host names and UDP ports to send notifications to */
char bc_hostaddress[MAXNODES][16];
char bc_hostservice[MAXNODES][16] =
	{ [0 ... MAXNODES - 1] = { [0 ... 15] = 0 } };
char sendbuffer[256];
int nodes = 0;
int node;
int send_error_limit[MAXNODES];
//--------------------------------------
/* default port to listen for incomming packets and to send packet to */
char default_lan_service[16] = "6788";

int lan_active = 0;
int send_error[MAXNODES];
int lan_mutex = 0;
int send_packets[MAXNODES];
int recv_error;
int recv_packets;
int buflen;
char talkarray[5][62];
float node_frequencies[MAXNODES];
int lanqsos;
char lastqsonr[5];
int highqsonr;
int landebug = 0;
long lantime;
long timecorr;
int time_master;
char thisnode = 'A'; 		/*  start with 'A' if not defined in
				    logcfg.dat */

  //---------------------end lan globals --------------

int resolveService(const char * service) {
	struct servent * service_ent;
	service_ent = getservbyname(service, "udp");
	int port = 0;
	if (service_ent != NULL) {
		port = service_ent->s_port;
	} else if (strlen(service) > 0) {
		port = atoi(service);
	}
	if (port == 0) {
		port = atoi(default_lan_service);
	}
	return port;
}

int lanrecv_init(void) {
    if (lan_active == 0)
	return (1);

    bzero(&lan_sin, sizeof(lan_sin));
    lan_sin.sin_family = AF_INET;
    lan_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    lan_sin.sin_port = htons(resolveService(default_lan_service));
    lan_sin_len = sizeof(lan_sin);

    lan_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (lan_socket_descriptor == -1) {
	syslog(LOG_ERR, "%s\n", "LAN: socket");
	return (-1);
    }

    lan_bind_rc =
	bind(lan_socket_descriptor, (struct sockaddr *) &lan_sin,
	     sizeof(lan_sin));
    if (lan_bind_rc == -1) {
	syslog(LOG_ERR, "%s\n", "LAN: bind");
	return (-2);
    }

    lan_save_file_flags = fcntl(lan_socket_descriptor, F_GETFL);
    lan_save_file_flags |= O_NONBLOCK;
    if (fcntl(lan_socket_descriptor, F_SETFL, lan_save_file_flags) == -1) {
	syslog(LOG_ERR, "%s\n", "trying non-blocking");
	return (-3);
    }
    return (0);
}

int lan_recv_close(void)
{

    if (lan_active == 0)
	return (-1);

    lan_close_rc = close(lan_socket_descriptor);
    if (lan_close_rc == -1) {
	syslog(LOG_ERR, "%s\n", "LAN: close call failed");
	return (errno);
    }

    return (0);
}

int lan_recv(void)
{

    if (lan_active == 0)
	return (-1);

    lan_recv_message[0] = '\0';

    lan_recv_rc =
	recvfrom(lan_socket_descriptor, lan_recv_message,
		 sizeof(lan_recv_message), 0, (struct sockaddr *) &lan_sin,
		 &lan_sin_len);

    if (lan_recv_rc == -1 && errno != EAGAIN) {
	recv_error++;
	return (errno);
    } else if (lan_recv_rc == 0 || errno == EAGAIN) {	/* no data */
	errno = 0;		/* clear the error */
    }

    errno = 0;			/* clear the error */

    if (lan_recv_message[1] == CLUSTERMSG)
	cl_send_inhibit = 1;	// this node does not send cluster info

    if (lan_recv_rc > 0)
	recv_packets++;

    strcpy(lan_message, lan_recv_message);
    if (lan_recv_rc > buflen)
	buflen = lan_recv_rc;

    return (0);
}

// ----------------send routines --------------------------

int lan_send_init(void)
{

    if (lan_active == 0)
	return (1);

    for (node = 0; node < nodes; node++) {

	bc_hostbyname[node] = gethostbyname(bc_hostaddress[node]);
	if (bc_hostbyname[node] == NULL) {
	    syslog(LOG_ERR, "%s\n", "LAN: gethostbyname failed");
	    return (-1);
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
	    return (-1);
	}
    }

    return (0);
}

int lan_send_close(void)
{

    if (lan_active == 0)
	return (-1);

    for (node = 0; node < nodes; node++) {

	bc_close_rc = close(bc_socket_descriptor[node]);
	if (bc_close_rc == -1) {
	    syslog(LOG_ERR, "%s\n", "LAN: close call failed");
	    return (-1);
	}
    }

    return (0);
}

int lan_send(char *lanbuffer)
{

    if (lan_active == 0)
	return (-1);

    for (node = 0; node < nodes; node++) {

	if (lanbuffer[0] != '\0') {
	    bc_sendto_rc = sendto(bc_socket_descriptor[node],
				  lanbuffer, 256,
				  0, (struct sockaddr *) &bc_address[node],
				  sizeof(bc_address[node]));

	}

	if (bc_sendto_rc == -1) {
	    if (send_error[node] >= (send_error_limit[node] + 10)) {
		mvprintw(24, 0, "LAN: send problem...!");
		refreshp();
		send_error_limit[node] += 10;
	    } else
		send_error[node]++;

	} else
	    send_packets[node]++;
    }

    lanbuffer[0] = '\0';

    return (0);
}

/* ----------------- send lan message ----------*/

int send_lan_message(int opcode, char *message)
{

    char sendbuffer[84];

    sendbuffer[0] = thisnode;
    sendbuffer[1] = opcode;
    sendbuffer[2] = '\0';
    strncat(sendbuffer, message, 80);
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
	sendbuffer[82] = '\0';
	lan_send(sendbuffer);
    }
    if (opcode == QTCSENTRY) {
	sendbuffer[87] = '\0';
	lan_send(sendbuffer);
    }

    return (0);
}

/* ----------------- send talk message ----------*/

int talk(void)
{

    char talkline[61] = "";

    mvprintw(24, 0,
	     "                                                                           ");
    mvprintw(24, 0, "T>");
    refreshp();
    echo();
    getnstr(talkline, 60);
    noecho();

    strcat(talkline, "\n");

    send_lan_message(TLFMSG, talkline);

    talkline[0] = '\0';
    attron(COLOR_PAIR(C_HEADER));
    mvprintw(24, 0,
	     "                                                                               ");
    refreshp();

    return (0);
}

/* ----------------- send freq. message ----------*/

int send_freq(float freq)
{

    extern int bandinx;
    extern int trx_control;

    char fbuffer[8];

    if (trx_control == 1) {

	sprintf(fbuffer, "%7.1f", freq);
    } else {
	switch (bandinx) {

	case 0:
	    sprintf(fbuffer, " 160.0");
	    break;

	case 1:
	    sprintf(fbuffer, "  80.0");
	    break;

	case 2:
	    sprintf(fbuffer, "  40.0");
	    break;

	case 3:
	    sprintf(fbuffer, "  30.0");
	    break;

	case 4:
	    sprintf(fbuffer, "  20.0");
	    break;

	case 5:
	    sprintf(fbuffer, "  17.0");
	    break;

	case 6:
	    sprintf(fbuffer, "  15.0");
	    break;

	case 7:
	    sprintf(fbuffer, "  12.0");
	    break;

	case 8:
	    sprintf(fbuffer, "  10.0");
	    break;

	default:
	    sprintf(fbuffer, "     ");
	}
    }

    send_lan_message(FREQMSG, fbuffer);

    return (0);
}

/* ----------------- send time message ----------*/

int send_time(void)
{

    extern int timeoffset;

    long now;
    char timebuffer[14];

    now = (long) (time(0) + (timeoffset * 3600));

    sprintf(timebuffer, "%ld", now);
    strcat(timebuffer, " ");
    send_lan_message(TIMESYNC, timebuffer);

    return (0);
}
