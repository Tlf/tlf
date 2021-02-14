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
/* ------------------------------------------------------------
 *       Include note  in log
 *
 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>

#include "clear_display.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "nicebox.h"		// Includes curses.h
#include "scroll_log.h"


int include_note(void) {

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
    mvprintw(15, 1, "");

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

	g_strlcpy(qsos[nr_qsos], buffer2, LOGLINELEN);
	nr_qsos++;

	scroll_log();
	g_strlcpy(logline4, buffer2, 81);  /* max. 80 columns */
	clear_display();

    }

    attron(COLOR_PAIR(C_LOG | A_STANDOUT));

    for (i = 14; i <= 16; i++)
	mvprintw(i, 0, backgrnd_str);

    return (0);
}
