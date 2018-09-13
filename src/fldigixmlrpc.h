/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2014           Ervin Hegedus <airween@gmail.com>
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


#ifndef FLDIGIXMLRPC_H
#define FLDIGIXMLRPC_H

#define FLDIGI_TX   1
#define FLDIGI_RX   2

int fldigi_xmlrpc_init();
int fldigi_xmlrpc_cleanup();

int fldigi_xmlrpc_get_carrier(void);
int fldigi_get_carrier();
int fldigi_get_shift_freq();
int fldigi_get_rx_text(char *line, int len);
int fldigi_send_text(char *line);
void fldigi_to_rx();
void xmlrpc_showinfo();
int fldigi_get_log_call();
int fldigi_get_log_serial_number();
void fldigi_clear_connerr();
int fldigi_toggle(void);
int fldigi_get(void);

#endif /* end of include guard: FLDIGIXMLRPC_H */
