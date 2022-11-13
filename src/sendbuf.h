/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2014           Thomas Beierlein <tb@forth-ev.de>
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


#ifndef SENDBUF_H
#define SENDBUF_H

typedef void (*ExpandMacro_t)(void);

void ExpandMacro_CurrentQso(void);
void ExpandMacro_PreviousQso(void);

char short_number(char c);
void sendmessage(const char *msg);
void send_standard_message(int msg);
void send_standard_message_prev_qso(int msg);
void send_keyer_message(int msg);

void replace_n(char *buf, int size, const char *what, const char *rep,
	       int count);

#endif /*  SENDBUF_H */
