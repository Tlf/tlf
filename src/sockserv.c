/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <address@hidden>
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


/* Socket server (and client!) utilities, */
/* intended to simplify the porting of JNOS servers and clients to Unix */
/* Written by N2RJT - Dave Brown */


#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curses.h>

#include "sockserv.h"


/* This structure holds the buffers for each open socket.  It was an */
/* afterthought, so it is not used for anything else.  The array is only */
/* allocated when the socket is opened, and freed when the socket is closed. */
struct sockbuffer {
    char *buf;
    int buflen;
    int fragment;
    int whole_lines;
    int cr_translation;
} sockbuf[FD_SETSIZE];

/* This array holds the SERVED sockets, that can be connected to. */
/* It does not hold CLIENT sockets. */
static int lsock[MAX_SERVED_SOCKETS];
static int initialized = 0;
static int nlsock = 0;
static struct timeval *socktimeval = NULL;
static struct timeval *selecttimeval = NULL;
static struct sockaddr_in udp_peer;
static int udpport = 0;
static unsigned int peerlen = 0;
int udp_socket = -1;

static fd_set readfds, openfds;
static int nfds = 0;
static int ifds = -1;

#define SOBUF 512
#define NULLCHAR (char *) NULL
#define myperror perror

void setlinemode(int s, int tf)
{
    sockbuf[s].whole_lines = tf;
}

int close_s(int s)
{
    FD_CLR(s, &openfds);
	if ( 0 != sockbuf[s].buflen )
		 free(sockbuf[s].buf);
    sockbuf[s].buflen = 0;
    sockbuf[s].fragment = 0;
    shutdown(s, 2);
    return close(s);
}

void fds_copy(fd_set *tofd, fd_set *fmfd)
{
    memcpy(tofd, fmfd, sizeof(fd_set));
}

int usputs(int s, char *buf)
{
    int len, i;
    len = strlen(buf);
    if (sockbuf[s].cr_translation) {
	for (i = 0; i < len; i++) {
	    if (buf[i] == '\n')
		usputb(s, "\r\n", 2);
	    else
		usputb(s, buf + i, 1);
	}
	return len;
    } else
	return usputb(s, buf, len);
}

int usputb(int s, char *buf, int buflen)
{

extern WINDOW *sclwin;

    strcpy(sockserv_error, "");
    if (udp_socket == s) {
	peerlen = sizeof(udp_peer);
	if (sendto(s, buf, buflen, 0, (struct sockaddr *) &udp_peer,
                   peerlen) < 0) {
	    myperror("usputb:sendto");
	    return -1;
	} else
	    return buflen;
    } else {
	if (write(s, buf, buflen) < 0) {
//	    myperror("usputb:write");
	    wprintw(sclwin, "Not connected !!");
	     wrefresh(sclwin);
	    sleep(2);
	    return -1;
	} else
	    return buflen;
    }
}

int usvprintf(int s, char *fmt, va_list args)
{
    int len, withargs;
    char *buf;

    if (strchr(fmt, '%') == NULLCHAR) {
	/* Common case optimization: no args */
	withargs = 0;
	buf = fmt;
	len = strlen(fmt);
    } else {
	/* Use a default value that is huge */
	withargs = 1;
	buf = (char *) malloc(SOBUF);
	if (buf == NULL) {
	    /* no memory available -> just ignore the output to the socket */
	    return 0;
	}
	if ((len = vsprintf(buf, fmt, args)) >= SOBUF) {
	    /* It's too late to be sorry.  He's dead, Jim */
	    fprintf(stderr, "usprintf() exceeded %d bytes (%d bytes)\n",
		    SOBUF, len);
	    exit(1);
	}
    }
    len = usputs(s, buf);
    if (withargs)
	free(buf);
    return len;
}

int usprintf(int s, char *fmt,...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = usvprintf(s, fmt, args);
    va_end(args);
    return len;
}

int tprintf(char *fmt,...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = usvprintf(ifds, fmt, args);
    va_end(args);
    return len;
}

int tputstr(char *buf)
{
    return usputs(ifds, buf);
}

int tputc(char c)
{
    char ic;
    ic = c;
    return usputb(ifds, &ic, 1);
}

static void (*login[MAX_SERVED_SOCKETS]) (int i);
char sockserv_error[80];
static int initialize(void)
{
    int i;
    strcpy(sockserv_error, "");
    /* First-time initialization */
    if (!initialized) {
	initialized = 1;
	for (i = 0; i < FD_SETSIZE; i++) {
	    sockbuf[i].buf = NULL;
	    sockbuf[i].buflen = 0;
	    sockbuf[i].fragment = 0;
	    sockbuf[i].whole_lines = 0;
	    sockbuf[i].cr_translation = 0;
	}
	FD_ZERO(&openfds);
    }
    return 0;

}

int startup(int portnum, void (*newin) (int))
{
    struct sockaddr_in sin;

    initialize();
    while ((lsock[nlsock] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	if (errno != EINTR) {
	    myperror("startup: socket");
	    exit(1);
	}
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(portnum);
    setsockopt(lsock[nlsock], SOL_SOCKET, SO_REUSEADDR, (char *) 0, 0);
    while (bind(lsock[nlsock], (struct sockaddr *) &sin, sizeof(sin)) == -1) {
	if (errno != EINTR) {
	    myperror("startup: bind");
	    exit(1);
	}
    }
    while (listen(lsock[nlsock], 5) == -1) {
	if (errno != EINTR) {
	    myperror("startup: listen");
	    exit(1);
	}
    }

    login[nlsock] = newin;
    FD_SET(lsock[nlsock], &openfds);
    sockbuf[lsock[nlsock]].buf = (char *) malloc(sizeof(char) * SOBUF);
    sockbuf[lsock[nlsock]].buflen = 0;
    sockbuf[lsock[nlsock]].fragment = 0;
    sockbuf[lsock[nlsock]].whole_lines = 0;
    sockbuf[lsock[nlsock]].cr_translation = 0;
    if (nfds <= lsock[nlsock])
	nfds = lsock[nlsock] + 1;
    if (ifds == -1)
	ifds = nfds - 1;
    nlsock++;
    return lsock[nlsock-1];
}

int startup_udp(int portnum)
{
    struct sockaddr_in sin;

    initialize();
    if (udp_socket == -1) {
	while ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    if (errno != EINTR) {
		myperror("startup_udp: socket");
		exit(1);
	    }
	}
    }
    if (portnum && !udpport) {
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(portnum);
	while (bind(udp_socket, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
	    if (errno != EINTR) {
		myperror("startup_udp: bind");
		exit(1);
	    }
	}
    }
    udpport = portnum;

    FD_SET(udp_socket, &openfds);
    sockbuf[udp_socket].buf = (char *) malloc(sizeof(char) * SOBUF);
    sockbuf[udp_socket].buflen = 0;
    sockbuf[udp_socket].fragment = 0;
    sockbuf[udp_socket].whole_lines = 0;
    sockbuf[udp_socket].cr_translation = 0;
    if (nfds <= udp_socket)
	nfds = udp_socket + 1;
    if (ifds == -1)
	ifds = nfds - 1;
    return udp_socket;
}

int recvline(int *fd, char *buf, int buflen)
{
    unsigned int len;
    int ns, i;
    struct sockaddr_in client;
    char *nl;

    strcpy(sockserv_error, "");

    if (selecttimeval != NULL && socktimeval != NULL) {
	selecttimeval->tv_sec = socktimeval->tv_sec;
	selecttimeval->tv_usec = socktimeval->tv_usec;
    }
    if (ifds == -1)
	ifds = nfds - 1;
    while (1) {
	if (++ifds == nfds) {
	    fds_copy(&readfds, &openfds);
	    while ((ifds = select(nfds, &readfds, (fd_set *) NULL, (fd_set *) NULL,
				  selecttimeval)) < 0) {
		if (errno != EINTR) {
		    myperror("recvline: select");
		    exit(1);
		}
		fds_copy(&readfds, &openfds);
	    }
	    if (!ifds)
		return -2;
	    ifds = 0;
	} else if (FD_ISSET(ifds, &readfds)) {
	    for (i = 0; i < nlsock; i++)
		if (lsock[i] == ifds)
		    break;
	    if (i < nlsock) {
		if (FD_ISSET(lsock[i], &readfds)) {
		    len = sizeof(client);
		    while ((ns = accept(lsock[i], (struct sockaddr *) &client,
                                        &len)) == -1) {
			if (errno != EINTR) {
			    myperror("recvline: accept");
			    exit(1);
			}
		    }
		    if (nfds <= ns)
			nfds = ns + 1;
		    FD_SET(ns, &openfds);
		    sockbuf[ns].buf = (char *) malloc(sizeof(char) * SOBUF);
		    sockbuf[ns].buflen = 0;
		    sockbuf[ns].fragment = 0;
		    sockbuf[ns].whole_lines = sockbuf[lsock[i]].whole_lines;
		    sockbuf[ns].cr_translation = 0;
		    (*login[i]) (ns);
		}
		FD_CLR(lsock[i], &readfds);
	    } else {
		if (!sockbuf[ifds].buflen) {
		    if (ifds == udp_socket) {
			peerlen = sizeof(udp_peer);
			while ((sockbuf[ifds].buflen =
				recvfrom(ifds, sockbuf[ifds].buf + sockbuf[ifds].fragment,
			     SOBUF - 1, 0, (struct sockaddr *) &udp_peer, &peerlen)) == -1) {
			    if (errno != EINTR) {
				break;
			    }
			    peerlen = sizeof(udp_peer);
			}
		    } else {
			while ((sockbuf[ifds].buflen =
				read(ifds, sockbuf[ifds].buf + sockbuf[ifds].fragment,
				     SOBUF - 1)) == -1) {
			    if (errno != EINTR) {
				break;
			    }
			}
		    }
		    if (sockbuf[ifds].buflen <= 0) {
			if (ifds != udp_socket) {
			    FD_CLR(ifds, &openfds);
			    free(sockbuf[ifds].buf);
			}
			sockbuf[ifds].buflen = 0;
			*fd = ifds;
			buf[0] = '\0';
			return -1;
		    } else {
			sockbuf[ifds].buflen += sockbuf[ifds].fragment;
			sockbuf[ifds].fragment = 0;
			sockbuf[ifds].buf[sockbuf[ifds].buflen] = '\0';
		    }
		}
		nl = strchr(sockbuf[ifds].buf, '\n');
		if (nl == NULL && sockbuf[ifds].whole_lines) {
		    nl = strchr(sockbuf[ifds].buf, '\r');
		    if (nl) {
			sockbuf[ifds].cr_translation = 1;
			printf("Enabling CR translation for socket %d\n", ifds);
		    }
		}
		if (nl == NULL && sockbuf[ifds].whole_lines) {
		    sockbuf[ifds].fragment = sockbuf[ifds].buflen;
		    sockbuf[ifds].buflen = 0;
		    continue;
		} else {
		    if (sockbuf[ifds].whole_lines) {
			*nl = '\0';
			len = strlen(sockbuf[ifds].buf) + 1;
		    } else if (sockbuf[ifds].buflen > buflen)
			len = buflen;
		    else
			len = sockbuf[ifds].buflen;
		    memcpy(buf, sockbuf[ifds].buf, len);
		    if (sockbuf[ifds].buflen > len)
			memcpy(sockbuf[ifds].buf, sockbuf[ifds].buf + len,
			       sockbuf[ifds].buflen - len);
		    sockbuf[ifds].buflen -= len;
		    *fd = ifds;
		    if (sockbuf[ifds].buflen)
			ifds--;
		}
		return len;
	    }
	}
    }
}

long resolve(char *hostname)
{
    unsigned long int haddr;
    unsigned char a[4];
    int i;
    char *s, *d, *c;
    int valid = 1;

    if (initialize())
	return -1L;

    s = hostname;
    for (i = 0; i < 4; i++) {
	if (s) {
	    d = strchr(s, '.');
	    if (!d)
		d = s + strlen(s);
	    for (c = s; c < d; c++)
		if (*c < '0' || *c > '9')
		    valid = 0;
	    if (!valid)
		break;
	    a[i] = atoi(s);
	    s = strchr(s, '.');
	    if (s)
		s++;
	} else {
	    valid = 0;
	    break;
	    a[i] = 0;
	}
    }
    if (valid) {
	haddr = a[3] + 256 * (a[2] + 256 * (a[1] + 256 * a[0]));
	haddr = htonl(haddr);
    } else {
	struct hostent *hp;

	if ((hp = gethostbyname(hostname)) == NULL) {
	    herror("resolve: gethostbyname");
	    return -1L;
	}
	memcpy(&haddr, hp->h_addr, hp->h_length);

    }

    strcpy(sockserv_error, "");
    return (haddr);
}

int startcliaddr(int family, unsigned long int addr, unsigned short int portnum)
{
	extern WINDOW *sclwin;

	int s;
    struct sockaddr_in sin;

    initialize();
    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = addr;
    sin.sin_family = family;
    sin.sin_port = htons(portnum);

    while ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	if (errno != EINTR) {
	    wprintw(sclwin, "socket failure");
	    wrefresh(sclwin);
	    sleep(1);
	    return -1;
	}
    }

    while (connect(s, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
	if (errno != EINTR) {

	    return (-1);
	}
    }

    if (nfds <= s)
	nfds = s + 1;
    if (ifds == -1)
	ifds = nfds - 1;
    FD_SET(s, &openfds);

    wprintw(sclwin, "still here...");
    wrefresh(sclwin);
    sleep(2);

    sockbuf[s].buf = (char *) malloc(sizeof(char) * SOBUF);

//	sockbuf[s].buf = (char *)socketbuffer;	// debug
    sockbuf[s].buflen = 0;
    sockbuf[s].fragment = 0;
    sockbuf[s].whole_lines = 0;

    wprintw(sclwin, "not dead...");
    wrefresh(sclwin);
    sleep(1);

    return s;
}

int startcli(void)
{
	extern char pr_hostaddress[];
	 extern int portnum;

	unsigned long int haddr;
    int addrtype;
    haddr = resolve(pr_hostaddress);
    addrtype = AF_INET;

    return (startcliaddr(addrtype, haddr, (short) portnum));
}

void socktimeout(int msec)
{
    if (!socktimeval)
	socktimeval = (struct timeval *) malloc(sizeof(struct timeval));
    if (!selecttimeval)
	selecttimeval = (struct timeval *) malloc(sizeof(struct timeval));
    socktimeval->tv_sec = msec / 1000L;
    socktimeval->tv_usec = (msec % 1000L) * 1000L;
}

void nosocktimeout(void)
{
    free(socktimeval);
    socktimeval = NULL;
    free(selecttimeval);
    selecttimeval = NULL;
}

void set_udp_peer(long address, int portnum)
{
    memset(&udp_peer, 0, sizeof(udp_peer));
    udp_peer.sin_addr.s_addr = address;
    udp_peer.sin_family = AF_INET;
    udp_peer.sin_port = htons(portnum);
    peerlen = sizeof(udp_peer);
}

void get_udp_peer(long *address, int *portnum)
{
    *address = udp_peer.sin_addr.s_addr;
    *portnum = 0;
    *portnum = ntohs(udp_peer.sin_port);
}
