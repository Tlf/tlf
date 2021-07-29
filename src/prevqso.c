/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2014, 2015     Thomas Beierlein <tb@forth-ev.de>
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
/* ------------------------------------------------------------
 *     repeat previous_qsonr
 *
 *--------------------------------------------------------------*/


#include <string.h>

#include <glib.h>

#include "globalvars.h"
#include "sendbuf.h"


void prev_qso(void) {

    int i;
    char *str;

    str = g_strdup_printf("%3s %03d ", last_rst, qsonum - 1);
    for (i = 0; i < strlen(str); i++) {
	str[i] = short_number(str[i]);
    }
    sendmessage(str);
    g_free(str);
}
