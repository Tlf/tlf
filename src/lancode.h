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


#ifndef LANCODE_H
#define LANCODE_H

#define MAXNODES 8

#define LOGENTRY 49
#define CLUSTERMSG 50
#define TLFSPOT 51
#define TLFMSG 52
#define FREQMSG 53
#define INCQSONUM 54
#define TIMESYNC 55
#define QTCRENTRY 56
#define QTCSENTRY 57
#define QTCFLAG 58

#include <hamlib/rig.h>

extern char bc_hostaddress[MAXNODES][16];
extern char bc_hostservice[MAXNODES][16];

int lan_recv_init(void);
int lan_recv_close(void);
int lan_recv(void);
int lan_send_init(void);
int lan_send_close(void);
int send_lan_message(int opcode, char *message);
void talk(void);
int send_freq(freq_t freq);
void send_time(void) ;

#endif /* LANCODE_H */
