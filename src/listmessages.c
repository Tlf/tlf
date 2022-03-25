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
/* ------------------------------------------------------------
*        List  CW messages
*
*--------------------------------------------------------------*/


#include <string.h>

#include <glib.h>

#include "clear_display.h"
#include "globalvars.h"
#include "nicebox.h"		// Includes curses.h
#include "tlf.h"
#include "ui_utils.h"

#define LIST_HEIGHT 15
#define LIST_WIDTH  78
#define LIST_UPPER  7
#define LIST_LEFT   0

char  printbuffer[160];

char *formatMessage(int i) {

    /* copy the message string */
    if (trxmode == DIGIMODE)
	g_strlcpy(printbuffer,  digi_message[i],  sizeof(printbuffer));
    else
	g_strlcpy(printbuffer,  message[i],  sizeof(printbuffer));

    /* and strip off trailing newline/whitespace */
    g_strchomp(printbuffer);

    /* and fill up with spaces */
    strcat(printbuffer, backgrnd_str);
    printbuffer[LIST_WIDTH - 7] = '\0';

    return printbuffer;
}

void listmessages(void) {

    int i;

    nicebox(LIST_UPPER, LIST_LEFT, LIST_HEIGHT, LIST_WIDTH, "Messages");
    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);

    for (i = 0  ; i  < 12 ; i++) {
	mvprintw(i + LIST_UPPER + 1, 1, " %2i  : %s",  i + 1, formatMessage(i));
    }

    mvprintw(12 + LIST_UPPER + 1, 1, " SPmg: %s", formatMessage(SP_TU_MSG));
    mvprintw(13 + LIST_UPPER + 1, 1, " CQmg: %s", formatMessage(CQ_TU_MSG));
    mvprintw(14 + LIST_UPPER + 1, 1, " SPCa: %s", formatMessage(SP_CALL_MSG));

    attroff(A_STANDOUT);
    mvaddstr(23, 30,  "Press any key");
    refreshp();

    (void)key_get();

    clear_display();

    attron(COLOR_PAIR(C_LOG)  |  A_STANDOUT);
    for (i = 13 ;  i  <= 23 ; i++) {
	clear_line(i);
    }

    refreshp();

    return;
}

