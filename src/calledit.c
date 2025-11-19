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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------
 *        Edit call input field
 *
 *--------------------------------------------------------------*/


#include <string.h>

#include "callinput.h"
#include "getctydata.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "searchlog.h"		// Includes glib.h
#include "showinfo.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"


int calledit(void) {

    int i = 0, l, b;
    int j = 0;
    int x = -1;
    int cnt = 0, insertflg = 0;
    char call1[30], call2[10];

    l = strlen(current_qso.call);
    b = l - 1;


    while ((i != ESCAPE) && (b <= strlen(current_qso.call))) {

	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvaddstr(12, 29, "            ");
	mvaddstr(12, 29, current_qso.call);
	move(12, 29 + b);
	/* no refreshp() here as getch() calls wrefresh() for the
	 * panel with last output (where the cursor should go */

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
	    i = 0;
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
	    if (b < strlen(current_qso.call) - 1) {
		b++;
	    } else
		break;		/* stop edit */

	    // <Delete>
	} else if (i == KEY_DC) {

	    l = strlen(current_qso.call);

	    for (j = b; j <= l; j++) {
		current_qso.call[j] = current_qso.call[j + 1];	/* move to left incl. \0 */
	    }

	    update_info_line();

	    if (cnt > 1)
		searchlog();

	    // <Backspace>
	} else if (i == KEY_BACKSPACE) {

	    if (b > 0) {

		b--;

		l = strlen(current_qso.call);

		for (j = b; j <= l; j++) {
		    current_qso.call[j] = current_qso.call[j + 1];
		}

		update_info_line();

		if (cnt > 1)
		    searchlog();
	    }

	    // <Insert>
	} else if (i == KEY_IC) {
	    if (insertflg == 0)
		insertflg = 1;
	    else
		insertflg = 0;

	    // these keys terminate the callinput() loop so they should also
	    // terminate calledit(); pass them through
	} else if (i == '\n' || i == KEY_ENTER || i == SPACE || i == TAB
		   || i == CTRL_K || i == ',' || i == BACKSLASH) {
	    x = i;
	    break;

	    // Any character left other than <Escape>.
	} else if (i != ESCAPE) {


	    if (valid_call_char(i)) {

		call2[0] = '\0';

		// Promote lower case to upper case.
		i = g_ascii_toupper(i);

		if (b <= 12) {
		    strncpy(call1, current_qso.call, b);
		    strncpy(call2, current_qso.call + b, strlen(current_qso.call) - (b - 1));
		}

		if (strlen(current_qso.call) + 1 == 12)
		    break;	// leave insert mode

		call1[b] = i;
		call1[b + 1] = '\0';
		if ((strlen(call1) + strlen(call2)) < 12) {
		    strcat(call1, call2);
		    if (strlen(call1) >= 12)
			break;
		    strcpy(current_qso.call, call1);
		}

		if ((b < strlen(current_qso.call) - 1) && (b <= 12))
		    b++;
		else
		    break;

		update_info_line();

		searchlog();

	    } else if (i != 0)
		i = ESCAPE;

	} else
	    i = ESCAPE;

    }

    attroff(A_STANDOUT);
    attron(COLOR_PAIR(C_HEADER));

    mvaddstr(12, 29, "            ");
    mvaddstr(12, 29, current_qso.call);
    refreshp();

    attron(A_STANDOUT);
    searchlog();

    return x;
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
	    strncpy(call1, current_qso.call, curposition);
	}

	if (curposition <= 10) {
	    strncpy(call2, current_qso.call + curposition,
		    strlen(current_qso.call) - (curposition - 1));
	}

	// Too long!
	if (strlen(current_qso.call) + 1 == MAX_CALL_LENGTH)
	    break;		// leave insert mode

	// Accept A-Z or / and 1-9
	if (((ichr >= 65) && (ichr <= 90))
		|| ((ichr >= 47) && (ichr <= 57))) {
	    call1[curposition] = ichr;
	    call1[curposition + 1] = '\0';
	    if ((strlen(call1) + strlen(call2)) < 12) {
		strcat(call1, call2);
		if (strlen(call1) + strlen(current_qso.call) >= 12)
		    break;
		strcpy(current_qso.call, call1);
	    }
	} else
	    break;

	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvaddstr(12, 29, current_qso.call);
	curposition++;
	move(12, 29 + curposition);
	refreshp();

    }
    ichr = ESCAPE;

    return (ichr);
}
