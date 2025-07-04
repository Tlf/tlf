/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2022	        Thomas Beierlein <dl1jbe@darc.de>
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
 *       Include note  in log
 *
 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "clear_display.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "log_utils.h"
#include "nicebox.h"		// Includes curses.h
#include "scroll_log.h"
#include "store_qso.h"
#include "ui_utils.h"


void include_note(void) {

    extern char thisnode;

    char buffer[80] = "";
    char buffer2[LOGLINELEN + 1] = "";

    int i;

    attron(A_STANDOUT);
    mvaddstr(15, 1, spaces(78));
    nicebox(14, 0, 1, 78, "Note (leave blank to ignore)");
    attron(A_STANDOUT);
    move(15, 1);

    echo();
    getnstr(buffer, 78);
    noecho();

    // replace non-printable chars with space and trim buffer
    for (char *p = buffer; *p; p++) {
	if (!isprint(*p)) {
	    *p = ' ';
	}
    }
    g_strstrip(buffer);

    if (lan_active) {
	sprintf(buffer2, "; Node %c, %d : ", thisnode, atoi(qsonrstr) - 1);
    } else
	sprintf(buffer2, "; ");

    if (strlen(buffer) >= 1) {
	strncat(buffer2, buffer, (LOGLINELEN - 1) - strlen(buffer2));
	memset(buffer2 + strlen(buffer2), ' ',
	       (LOGLINELEN - 1) - strlen(buffer2)); /* fill with spaces */
	buffer2[LOGLINELEN - 1] = '\0';

	store_qso(logfile, buffer2);

	struct qso_t *qso = parse_qso(buffer2);
	g_ptr_array_add(qso_array, qso);

	scroll_log();
	clear_display();
    }

    attron(COLOR_PAIR(C_LOG | A_STANDOUT));

    for (i = 14; i <= 16; i++)
	clear_line(i);

}
