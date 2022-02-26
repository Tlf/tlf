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

#include "clear_display.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "log_utils.h"
#include "nicebox.h"		// Includes curses.h
#include "scroll_log.h"


void include_note(void) {

    extern char thisnode;

    char buffer[80] = "";
    char buffer2[LOGLINELEN + 1] = "";

    int i;
    FILE *fp;

    attron(A_STANDOUT);
    mvprintw(15, 1,
	     "                                                                              ");
    nicebox(14, 0, 1, 78, "Note");
    attron(A_STANDOUT);
    move(15, 1);

    echo();
    getnstr(buffer, 78);
    noecho();

    if (lan_active) {
	sprintf(buffer2, "; Node %c, %d : ", thisnode, atoi(qsonrstr) - 1);
    } else
	sprintf(buffer2, "; ");

    if (strlen(buffer) >= 1) {
	strncat(buffer2, buffer, (LOGLINELEN - 1) - strlen(buffer2));
	memset(buffer2 + strlen(buffer2), ' ',
	       (LOGLINELEN - 1) - strlen(buffer2)); /* fill spaces */
	buffer2[LOGLINELEN - 1] = '\0';

	if ((fp = fopen(logfile, "a")) == NULL) {
	    endwin();
	    fprintf(stdout, "\nnote.c: Error opening log file.\n");
	    exit(1);
	}
	fputs(buffer2, fp);
	fputs("\n", fp);

	fclose(fp);

	struct qso_t *qso = parse_qso(buffer2);
	g_ptr_array_add(qso_array, qso);
	nr_qsos++;

	scroll_log();
	clear_display();
    }

    attron(COLOR_PAIR(C_LOG | A_STANDOUT));

    for (i = 14; i <= 16; i++)
	clear_line(i);

    return;
}
