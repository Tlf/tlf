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
 *         Change CW messages
 *         last change: 25.2.02 11:50
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "clear_display.h"
#include "globalvars.h"
#include "nicebox.h"		// Includes curses.h
#include "tlf.h"
#include "ui_utils.h"
#include "writeparas.h"


int message_change(int x) {
    extern char message[][80];

    int j;
    int count;
    int mes_length;
    int bufnr = 0;
    char printbuf[80];
    char *msg;

    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }

    nicebox(14, 3, 2, 60, "Enter message (F1-12, C, S)");

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    while (1) {
	bufnr = toupper(key_get());

	if ((bufnr == 'C') || (bufnr == 'S'))
	    break;

	if (bufnr >= KEY_F(1) && bufnr <= KEY_F(12))
	    break;
    }

    if (bufnr == 'S') {
	bufnr = 12;
    } else if (bufnr == 'C') {
	bufnr = 13;
    } else {
	bufnr = bufnr - KEY_F(1);
    }

    if (trxmode == DIGIMODE)
	msg = digi_message[bufnr];
    else
	msg = message[bufnr];

    g_strlcpy(printbuf, msg, sizeof(printbuf));
    mvprintw(15, 4, "%s", printbuf);
    refreshp();

    mvprintw(16, 4, "");
    msg[0] = '\0';

    echo();
    getnstr(printbuf, 60);
    noecho();

    if (trxmode == DIGIMODE)
	strcat(printbuf, " ");
    else
	strcat(printbuf, "\n");
    mes_length = strlen(printbuf);

    if (mes_length < 2) {
	clear_display();
	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	for (j = 13; j <= 23; j++) {
	    mvprintw(j, 0, backgrnd_str);
	}

	if (trxmode == DIGIMODE) {
	    strcat(printbuf, " ");
	    free(digi_message[bufnr]);
	    digi_message[bufnr] = strdup(printbuf);
        }
	else
	    strcpy(message[bufnr], printbuf);

	return (1);
    }

    for (count = 0; count <= mes_length; count++) {
	if ((printbuf[count] > 96)
		&& (printbuf[count] < 123))
	    printbuf[count] = printbuf[count] - 32;
    }

    if (trxmode == DIGIMODE) {
	strcat(printbuf, " ");
	free(digi_message[bufnr]);
	digi_message[bufnr] = strdup(printbuf);
    }
    else
	strcpy(message[bufnr], printbuf);

    mvprintw(12, 29, "");
    refreshp();
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }

    writeparas();

    return (0);
}
