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
#ifndef NETKEYER_H
#define NETKEYER_H

#define K_RESET 0
#define K_MESSAGE 1
#define K_SPEED 2
#define K_TONE 3
#define K_ABORT 4
#define K_STOP 5
#define K_WORDMODE 6
#define K_WEIGHT 7
#define K_DEVICE 8
#define K_TOD 9			// set txdelay (turn on delay)
#define K_ADDRESS 10	// set port address of device
#define K_SET14 11 		// set pin 14 on lpt
#define K_TUNE 12		// tune
#define K_PTT 13		// PTT on/off
#define K_SWITCH 14		// set band switch output pins 2,7,8,9 on lpt
#define K_SIDETONE 15	// set sidetone to sound card
#define K_STVOLUME 16	// set sidetone volume

int netkeyer(int cw_op, char *cwmessage);
int netkeyer_close(void);
int netkeyer_init (void);

#endif /* NETKEYER_H */
