/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
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


#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"


int netkeyer_port = 6789;
char netkeyer_hostaddress[16] = "127.0.0.1";

static int socket_descriptor;
static struct sockaddr_in address;

int netkeyer_init(void) {

    /*
       Translate a host name to IP address
    */
    struct hostent *hostbyname;
    hostbyname = gethostbyname(netkeyer_hostaddress);
    if (hostbyname == NULL) {
	perror("gethostbyname failed");
	return -1;
    }
    /*
       Initialize socket address structure for Internet Protocols
       The address comes from the datastructure returned by gethostbyname()
    */
    bzero(&address, sizeof(address));	/* empty data structure */
    address.sin_family = AF_INET;
    memcpy(&address.sin_addr.s_addr, hostbyname->h_addr,
	   sizeof(address.sin_addr.s_addr));
    address.sin_port = htons(netkeyer_port);
    /*
       Create a UDP socket
    */
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor == -1) {
	perror("socket call failed");
	return -1;
    }

    return 0;
}

/*-------------------------end netkeyer_init---------------*/

int netkeyer_close(void) {
    int close_rc;

    close_rc = close(socket_descriptor);
    if (close_rc == -1) {
	perror("close call failed");
	return -1;
    }

    return 0;
}

/*-------------------------end netkeyer_close---------------*/

#define BUFSIZE 81

#define CMD(x) { do { 	buf[0] = '\e'; \
			buf[1] = x; \
			buf[2] = 0; } while(0); }

int netkeyer(int cw_op, char *cwmessage) {
    char buf[BUFSIZE] = "";
    ssize_t sendto_rc = 0;
    int add_message = 0;

    switch (cw_op) {

	case K_RESET:
	    CMD('0');       // reset: <ESC>0
	    break;
	case K_MESSAGE:
	    buf[0] = 0;
	    add_message = 1;    // play cw message
	    break;
	case K_SPEED:
	    CMD('2');       // speed: <ESC>2NN
	    add_message = 1;
	    break;
	case K_TONE:
	    CMD('3');       // tone: <ESC>3NN
	    add_message = 1;
	    break;
	case K_ABORT:
	    CMD('4');       // message abort: <ESC>4
	    break;
	case K_STOP:
	    CMD('5');       // keyer daemon stop: <ESC>5
	    break;
	case K_WORDMODE:
	    CMD('6');       // non-interruptible: <ESC>6
	    break;
	case K_WEIGHT:
	    CMD('7');       // set weight: <ESC>7NN
	    add_message = 1;
	    break;
	case K_DEVICE:
	    CMD('8');       // set device: <ESC>8NN
	    add_message = 1;
	    break;
	case K_PTT:
	    CMD('a');       // PTT on/off: <ESC>aNN
	    add_message = 1;
	    break;
	case K_SET14:
	    CMD('b');       // set pin 14 of lp port: <ESC>bNN
	    add_message = 1;
	    break;
	case K_TUNE:
	    CMD('c');       // tune: <ESC>cNN
	    add_message = 1;
	    break;
	case K_TOD:
	    CMD('d');       // set Turn On Delay (TXDELAY): <ESC>dNN
	    add_message = 1;
	    break;
	case K_SWITCH:
	    CMD('e');       // set band switch output: <ESC>eNN
	    add_message = 1;
	    break;
	case K_SIDETONE:
	    CMD('f');       // set sidetone output to sound card: <ESC>fs
	    buf[2] = 's';
	    break;
	case K_STVOLUME:
	    CMD('g');       // set sound card output volume: <ESC>gNN
	    add_message = 1;
	    break;

	default:
	    return 0;
    }

    if (add_message) {
	g_strlcat(buf, cwmessage, BUFSIZE);
    }

    sendto_rc = sendto(socket_descriptor, buf, strlen(buf) + 1,
		       0, (struct sockaddr *) &address,
		       sizeof(address));
    if (sendto_rc == -1) {
	mvprintw(24, 0, "Keyer send failed...!");
	refreshp();
	sleep(2);
	return -1;
    }

    return 0;
}

