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
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <curses.h>
#include <time.h>

#define MAXNODES 8

#define LOGENTRY 49
#define CLUSTERMSG 50
#define TLFSPOT 51
#define TLFMSG 52
#define FREQMSG 53
#define INCQSONUM 54
#define TIMESYNC 55

int lanrecv_init(void);
int lan_recv_close(void);
int lan_recv(void);
int lan_send_init (void);
int lan_send_close(void);
int lan_send(char *buffer) ;
int send_lan_message(int opcode , char *message);
int talk(void);
int send_freq(float freq);
int send_time(void) ;
