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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
	/* ------------------------------------------------------------
	 *         Change CW messages
	 *         last change: 25.2.02 11:50
	 *--------------------------------------------------------------*/

#include "messagechange.h"

int message_change(int x)
{
    extern char backgrnd_str[];
    extern char message[15][80];
    extern char sp_return[];
    extern char cq_return[];

    int j;
    int count;
    int mes_length;
    int bufnr = 0;
    char printbuf[80];

    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }
    if (x == 6) {
	nicebox(14, 3, 2, 60, "Enter message (F1-12, C, S)");

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	while (1) {
	    bufnr = onechar();

	    if ((bufnr == 'C') || (bufnr == 'S'))
		break;

	    if (bufnr >= 129 && bufnr <= 141)
		break;
	}

	if (bufnr == 'S') {
	    bufnr = 12;
	} else if (bufnr == 'C') {
	    bufnr = 13;
	} else {

	    if (bufnr <= 138) {
		bufnr = bufnr - 129;
	    } else {
		bufnr = bufnr - 130;
	    }
	}

	printbuf[0] = '\0';
	strncat(printbuf, message[bufnr], strlen(message[bufnr]) - 1);
	mvprintw(15, 4, "%s", printbuf);
	refreshp();

	mvprintw(16, 4, "");
	message[bufnr][0] = '\0';

	echo();
	getnstr(message[bufnr], 60);
	noecho();

	strcat(message[bufnr], "\n");
	mes_length = strlen(message[bufnr]);

	if (mes_length < 2) {
	    clear_display();
	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	    for (j = 13; j <= 23; j++) {
		mvprintw(j, 0, backgrnd_str);
	    }

	    return (1);
	}

	for (count = 0; count <= mes_length; count++) {
	    if ((message[bufnr][count] > 96)
		&& (message[bufnr][count] < 123))
		message[bufnr][count] = message[bufnr][count] - 32;
	}

    } else {			// from shift F1 - F8     (x = 142 - 150)
	if ((x > 141) && (x < 151)) {
	    bufnr = x - 142;

	    if (bufnr >= 3)
		bufnr--;

	    nicebox(14, 3, 2, 60, "Edit message");

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	    printbuf[0] = '\0';
	    strncat(printbuf, message[bufnr], strlen(message[bufnr]) - 1);

	    mvprintw(15, 4, "%s", printbuf);
	    refreshp();

	    mvprintw(16, 4, "");
	    message[bufnr][0] = '\0';

	    echo();
	    getnstr(message[bufnr], 60);
	    noecho();

	    strcat(message[bufnr], "\n");
	    mes_length = strlen(message[bufnr]);

	    if (mes_length < 2) {
		clear_display();
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

		for (j = 13; j <= 23; j++) {
		    mvprintw(j, 0, backgrnd_str);
		}

		return (1);
	    }

	    for (count = 0; count <= mes_length; count++) {
		if ((message[bufnr][count] > 96)
		    && (message[bufnr][count] < 123))
		    message[bufnr][count] = message[bufnr][count] - 32;
	    }

	}

    }
    mvprintw(12, 29, "");
    refreshp();
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }

    writeparas();

    strncpy(sp_return, message[12], 79);
    strncpy(cq_return, message[13], 79);

    return (0);
}
