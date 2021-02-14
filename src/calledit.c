/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011           Thomas Beierlein <tb@forth-ev.deY
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
 *        Edit call input field
 *
 *--------------------------------------------------------------*/


#include <string.h>

#include "getctydata.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "searchlog.h"		// Includes glib.h
#include "showinfo.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"


void calledit(void) {

    int i = 0, l, b;
    int j = 0;
    int x = 0;
    int cnt = 0, insertflg = 0;
    char call1[30], call2[10];

    l = strlen(hiscall);
    b = l - 1;


    while ((i != ESCAPE) && (b <= strlen(hiscall))) {

	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvprintw(12, 29, "            ");
	mvprintw(12, 29, hiscall);
	mvprintw(12, 29 + b, "");
	/* no refreshp() here as getch() calls wrefresh() for the
	 * panel with last output (whre the cursor should go */

	i = key_get();

	// <Delete> or <Insert>
	if ((i == KEY_DC) || (i == KEY_IC))
	    cnt++;
	else {
	    if (i != ESCAPE)
		cnt = 0;
	}

	// <Tab>
	if (i == TAB)
	    block_part = 1;
	else
	    block_part = 0;

	// Ctrl-A (^A) or <Home>, move to head of callsign field.
	if (i == CTRL_A || i == KEY_HOME) {
	    b = 0;
	    x = 0;
	}

	// Ctrl-E (^E) or <End>, move to end of callsign field, exit edit mode.
	if (i == CTRL_E || i == KEY_END) {
	    break;		/* stop edit */
	}

	// Left arrow
	if (i == KEY_LEFT) {

	    if (b > 0)
		b--;

	    // Right arrow
	} else if (i == KEY_RIGHT) {
	    if (b < strlen(hiscall) - 1) {
		b++;
	    } else
		break;		/* stop edit */

	    // <Delete>
	} else if (i == KEY_DC) {

	    l = strlen(hiscall);

	    for (j = b; j <= l; j++) {
		hiscall[j] = hiscall[j + 1];	/* move to left incl. \0 */
	    }

	    showinfo(getctydata_pfx(hiscall));

	    if (cnt > 1)
		searchlog();

	    // <Backspace>
	} else if (i == KEY_BACKSPACE) {

	    if (b > 0) {

		b--;

		l = strlen(hiscall);

		for (j = b; j <= l; j++) {
		    hiscall[j] = hiscall[j + 1];
		}

		showinfo(getctydata_pfx(hiscall));

		if (cnt > 1)
		    searchlog();
	    }

	    // <Insert>
	} else if (i == KEY_IC) {
	    if (insertflg == 0)
		insertflg = 1;
	    else
		insertflg = 0;

	    // Any character left other than <Escape>.
	} else if (i != ESCAPE) {

	    // Promote lower case to upper case.
	    if ((i >= 97) && (i <= 122))
		i = i - 32;

	    // Accept A-Z or / and 1-9
	    if (((i >= 65) && (i <= 90)) || ((i >= 47) && (i <= 57))) {

		call2[0] = '\0';

		if (b <= 12) {
		    strncpy(call1, hiscall, b);
		    strncpy(call2, hiscall + b, strlen(hiscall) - (b - 1));
		}

		if (strlen(hiscall) + 1 == 12)
		    break;	// leave insert mode

		call1[b] = i;
		call1[b + 1] = '\0';
		if ((strlen(call1) + strlen(call2)) < 12) {
		    strcat(call1, call2);
		    if (strlen(call1) >= 12)
			break;
		    strcpy(hiscall, call1);
		}

		if ((b < strlen(hiscall) - 1) && (b <= 12))
		    b++;
		else
		    break;

		showinfo(getctydata_pfx(hiscall));

		searchlog();

	    } else if (x != 0)
		i = ESCAPE;

	} else
	    i = ESCAPE;

    }

    attroff(A_STANDOUT);
    attron(COLOR_PAIR(C_HEADER));

    mvprintw(12, 29, "            ");
    mvprintw(12, 29, hiscall);
    refreshp();

    attron(A_STANDOUT);
    searchlog();
}

int insert_char(int curposition) {

    char call1[30], call2[10];
    int ichr = 0;

    attroff(A_STANDOUT);
    attron(COLOR_PAIR(C_HEADER));

    call1[0] = '\0';
    call2[0] = '\0';

    while (ichr != ESCAPE) {

	ichr = key_get();

	// Leave insert mode if <Tab>, <Enter>, or <Delete> are received.
	if ((ichr == TAB) || (ichr == '\n') || (ichr == KEY_ENTER) || (ichr == DELETE))
	    break;

	// Promote lower case to upper case.
	if ((ichr >= 97) && (ichr <= 122))
	    ichr = ichr - 32;

	if (curposition <= 10) {
	    strncpy(call1, hiscall, curposition);
	}

	if (curposition <= 10) {
	    strncpy(call2, hiscall + curposition,
		    strlen(hiscall) - (curposition - 1));
	}

	// Too long!
	if (strlen(hiscall) + 1 == 13)
	    break;		// leave insert mode

	// Accept A-Z or / and 1-9
	if (((ichr >= 65) && (ichr <= 90))
		|| ((ichr >= 47) && (ichr <= 57))) {
	    call1[curposition] = ichr;
	    call1[curposition + 1] = '\0';
	    if ((strlen(call1) + strlen(call2)) < 12) {
		strcat(call1, call2);
		if (strlen(call1) + strlen(hiscall) >= 12)
		    break;
		strcpy(hiscall, call1);
	    }
	} else
	    break;

	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvprintw(12, 29, hiscall);
	curposition++;
	mvprintw(12, 29 + curposition, "");
	refreshp();

    }
    ichr = ESCAPE;

    return (ichr);
}
