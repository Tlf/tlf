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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
	/* ------------------------------------------------------------
	 *     previous_qsonr
	 *
	 *--------------------------------------------------------------*/

#include "prevqso.h"
#include "qsonr_to_str.h"
#include "sendbuf.h"
#include <string.h>

int prev_qso(void)
{

    extern int qsonum;
    extern char qsonrstr[];

    char nr_buffer[5];

    qsonum--;
    qsonr_to_str();

    if (qsonrstr[0] != '0') {
	strncpy(nr_buffer, qsonrstr, 4);
	nr_buffer[4] = '\0';
    } else if (qsonrstr[1] != '0') {
	strncpy(nr_buffer, qsonrstr + 1, 3);
	nr_buffer[3] = '\0';
    } else if (qsonrstr[2] != '0') {
	strncpy(nr_buffer, qsonrstr + 2, 2);
	nr_buffer[2] = '\0';
    } else {
	strncpy(nr_buffer, qsonrstr + 3, 1);
	nr_buffer[1] = '\0';
    }

    strcat(buffer, "NR ");
    strcat(buffer, nr_buffer);
    strcat(buffer, " ");

    qsonum++;
    qsonr_to_str();

    sendbuf();

    return (0);
}
