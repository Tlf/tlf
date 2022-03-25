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
 *         Change CW/DIGI messages
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "clear_display.h"
#include "globalvars.h"
#include "nicebox.h"		// Includes curses.h
#include "tlf.h"
#include "ui_utils.h"
#include "keystroke_names.h"

static void enter_message(int bufnr) {

    char printbuf[80];
    char *msg;

    if (trxmode == DIGIMODE)
	msg = digi_message[bufnr];
    else
	msg = message[bufnr];

    g_strlcpy(printbuf, msg, sizeof(printbuf));

    g_strchomp(printbuf);

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvaddstr(15, 4, printbuf);
    refreshp();

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    move(16, 4);

    echo();
    getnstr(printbuf, 60);
    noecho();

    if (printbuf[0] == ESCAPE) {
	return; // user didn't wish to change the message
    }

    for (char *p = printbuf; *p; p++) {
	*p = toupper(*p);
    }

    if (trxmode == DIGIMODE) {
	free(digi_message[bufnr]);
	digi_message[bufnr] = strdup(printbuf);
    } else
	strcpy(message[bufnr], printbuf);
}


void message_change() {

    int bufnr;

    clear_display();

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    for (int y = 13; y < LINES - 1; y++) {
	clear_line(y);
    }

    nicebox(14, 3, 2, 60, "Enter message (F1-12, C, S)");

    while (1) {
	bufnr = toupper(key_get());

	if ((bufnr == 'C') || (bufnr == 'S'))
	    break;

	if (bufnr >= KEY_F(1) && bufnr <= KEY_F(12))
	    break;

	if (bufnr == ESCAPE)
	    break;
    }

    if (bufnr == 'S') {
	bufnr = 12;
    } else if (bufnr == 'C') {
	bufnr = 13;
    } else if (bufnr == ESCAPE) {
	bufnr = -1;
    } else {
	bufnr = bufnr - KEY_F(1);
    }

    if (bufnr >= 0) {
	enter_message(bufnr);
    }

    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

}
