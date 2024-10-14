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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#ifndef LANCODE_H
#define LANCODE_H

#define MAXNODES 8

#define LOGENTRY '1'
#define CLUSTERMSG '2'
#define TLFSPOT '3'
#define TLFMSG '4'
#define FREQMSG '5'
#define INCQSONUM '6'
#define TIMESYNC '7'
#define QTCRENTRY '8'
#define QTCSENTRY '9'
#define QTCFLAG ':'

#include <hamlib/rig.h>

extern char bc_hostaddress[MAXNODES][16];
extern char bc_hostservice[MAXNODES][16];
extern char talkarray[5][62];
extern char thisnode;
extern char lan_message[256];
extern int recv_error;
extern freq_t node_frequencies[MAXNODES];
extern int recv_packets;
extern int send_packets[MAXNODES];
extern int send_error[MAXNODES];

int lan_recv_init(void);
int lan_recv_close(void);
int lan_recv(void);
int lan_send_init(void);
int lan_send_close(void);
void send_lan_message(int opcode, char *message);
void talk(void);
int send_freq(freq_t freq);
void send_time(void) ;

#endif /* LANCODE_H */
