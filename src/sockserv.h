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


#ifndef SOCKSERV_H
#define SOCKSERV_H

/* External socket services: supported by sockserv.c */
/* Written by N2RJT */
/* Note that socket numbers are file descriptors, typically small integers */

#define MAX_SERVED_SOCKETS 4

/* Output functions: return number of bytes transmitted, or -1 on error */
extern int usputb(int s, char *buf, int buflen);
extern int usputs(int s, char *buf);
extern int usprintf(int s, char *fmt,...);
extern int tprintf(char *fmt,...);

/* Close a socket */
extern int close_s(int s);

/*
   ** Startup a server program
   ** portnum = TCP port to listen to
   ** login   = procedure to be called when a new socket is accepted;
   **           should be declared as:  void login(int s);
   **           where: s is the new socket number
   ** The logout condition is detected by a -1 returned from recvline().
 */
extern int startup(int portnum, void (*login) (int));

/*
   ** Set timeout for recvline call
 */
void socktimeout(int msec);
void nosocktimeout(void);

/*
   ** Set line mode: TRUE - only give me complete lines.
   **                FALSE- give me any data received, even multiple lines.
   ** The default is FALSE.
 */
void setlinemode(int s, int tf);

/*
   ** Receive a line from a socket
   ** s      = [OUT] the socket data was received on;
   ** buf    = [OUT] character buffer containing socket data;
   **                if linemode = TRUE, buffer is terminated with null character
   ** buflen = [IN]  length of the character buffer;
   ** returns the number of bytes read, or -1 indicating EOF on the socket.
   **         or -2 indicating timeout (if enabled).
 */
extern int recvline(int *s, char *buf, int buflen);
extern int startcli(void);
extern int startcliaddr(int family, unsigned long int addr, unsigned short int portnum);
extern long resolve(char *hostname);

extern int startup_udp(int portnum);
extern void set_udp_peer(long address, int portnum);
extern void get_udp_peer(long *address, int *portnum);
extern int udp_socket;
extern char sockserv_error[];

#endif /* SOCKSERV_H */
