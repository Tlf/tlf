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

#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"


int netkeyer_port = 6789;
char netkeyer_hostaddress[16] = "127.0.0.1";

static int socket_descriptor;
static struct sockaddr_in address;

int netkeyer_init(void)
{

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

int netkeyer_close(void)
{
    int close_rc;

    close_rc = close(socket_descriptor);
    if (close_rc == -1) {
	perror("close call failed");
	return -1;
    }

    return 0;
}

  /*-------------------------end netkeyer_close---------------*/

int netkeyer(int cw_op, char *cwmessage)
{
    char buf[80] = "";
    ssize_t sendto_rc;

    switch (cw_op) {

    case K_RESET:
	buf[0] = 27;
	sprintf(buf + 1, "0");		// reset
	break;
    case K_MESSAGE:
	sprintf(buf, "%s", cwmessage);	// cw message
	break;
    case K_SPEED:
	buf[0] = 27;
	sprintf(buf + 1, "2");		// speed
	sprintf(buf + 2, "%s", cwmessage);	// cw message
	break;
    case K_TONE:			// tone
	buf[0] = 27;
	sprintf(buf + 1, "3");
	sprintf(buf + 2, "%s", cwmessage);	// cw message
	break;
    case K_ABORT:			// message abort
	buf[0] = 27;
	sprintf(buf + 1, "4");
	break;
    case K_STOP:			// keyer daemon stop
	buf[0] = 27;
	sprintf(buf + 1, "5");
	break;
    case K_WORDMODE:			// non-interruptable
	buf[0] = 27;
	sprintf(buf + 1, "6");
	break;
    case K_WEIGHT:			// set weight
	buf[0] = 27;
	sprintf(buf + 1, "7");
	sprintf(buf + 2, "%s", cwmessage);	// cw message
	break;
    case K_DEVICE:			// set device
	buf[0] = 27;
	sprintf(buf + 1, "8");
	sprintf(buf + 2, "%s", cwmessage);	// cw message
	break;
    case K_ADDRESS:			// set device
	buf[0] = 27;
	sprintf(buf + 1, "9");
	sprintf(buf + 2, "%s", cwmessage);
	break;
    case K_PTT:				// PTT on/off
	buf[0] = 27;
	sprintf(buf + 1, "a");
	sprintf(buf + 2, "%s", cwmessage);
	break;
    case K_SET14:			// set pin 14 of lp port
	buf[0] = 27;
	sprintf(buf + 1, "b");
	sprintf(buf + 2, "%s", cwmessage);
	break;
    case K_TUNE:			// tune
	buf[0] = 27;
	sprintf(buf + 1, "c");
	sprintf(buf + 2, "%s", cwmessage);
	break;
    case K_TOD:				// set Turn On Delay (TXDELAY)
	buf[0] = 27;
	sprintf(buf + 1, "d");
	sprintf(buf + 2, "%s", cwmessage);
	break;
    case K_SWITCH:			// set band switch output
	buf[0] = 27;
	sprintf(buf + 1, "e");
	sprintf(buf + 2, "%s", cwmessage);
	break;
    case K_SIDETONE:			// set sidetone output to sound card
	buf[0] = 27;
	sprintf(buf + 1, "f");
	sprintf(buf + 2, "s");
	break;
    case K_STVOLUME:			// set sound card output volume
	buf[0] = 27;
	sprintf(buf + 1, "g");
	sprintf(buf + 2, "%s", cwmessage);
	break;

    default:
	return 0;
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

