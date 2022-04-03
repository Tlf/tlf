/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012-2014 Thomas Beierlein <tb@forth-ev.de>
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

/*------------------------------------------------------------------------

    CW keyboard routine

------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "clear_display.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "netkeyer.h"
#include "nicebox.h"		// Includes curses.h
#include "sendbuf.h"
#include "speedupndown.h"
#include "stoptx.h"
#include "tlf.h"
#include "tlf_panel.h"
#include "ui_utils.h"
#include "write_keyer.h"

/* size and position of keyer window */
#define KEYER_LINE_WIDTH 60
#define KEYER_WIN_WIDTH (KEYER_LINE_WIDTH+2)
#define KEYER_WIN_HEIGHT 3
#define KEYER_X (80-KEYER_WIN_WIDTH)/2
#define KEYER_Y 7


void mfj1278_control(int x);

void keyer(void) {

    static WINDOW *win = NULL;
    static PANEL *panel = NULL;

    int x = 0, j = 0;
    int cury, curx;
    char keyerstring[KEYER_LINE_WIDTH + 1] = "";
    int keyerstringpos = 0;
    char weightbuf[15];
    const char txcontrolstring[2] = { CTRL_T, '\0' };	// ^t
    const char rxcontrolstring[2] = { CTRL_R, '\0' };	// ^r
    const char crcontrolstring[2] = { RETURN, '\0' };	// cr
    const char ctl_c_controlstring[2] = { BACKSLASH, '\0' };	// '\'

    if ((trxmode == CWMODE && cwkeyer == NO_KEYER) ||
	    (trxmode == DIGIMODE && digikeyer == NO_KEYER)) {
	return; /* no keyer present */
    }

    const cqmode_t cqmode_save = cqmode;
    cqmode = KEYBOARD;
    show_header_line();

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    if (panel == NULL) {
	win = newwin(KEYER_WIN_HEIGHT, KEYER_WIN_WIDTH, KEYER_Y, KEYER_Y);
	if (win == NULL)
	    return;
	panel = new_panel(win);
	if (panel == NULL) {
	    delwin(win);
	    return;
	}
    }

    show_panel(panel);
    werase(win);
    wnicebox(win, 0, 0, 1, KEYER_LINE_WIDTH, "CW Keyer");

    if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
	/* switch to tx */
	keyer_append(txcontrolstring);
    }

    while (1) {
	wattron(win, COLOR_PAIR(C_LOG) | A_STANDOUT);
	wmove(win, 1, 1);
	for (j = 0; j < KEYER_LINE_WIDTH; j++) {
	    waddch(win, ' ');
	}
	mvwaddstr(win, 1, 1, keyerstring);
	refreshp();

	x = key_get();

	// Send space instead of double quote.
	if (x == '"') {
	    x = ' ';
	}

	// Send space instead of newline or return.
	if (x == '\n' || x == KEY_ENTER) {
	    x = ' ';
	}

	// <Escape>, Ctrl-K (^K), Alt-k (M-k)
	if (x == ESCAPE || x == CTRL_K || x == ALT_K) {
	    if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
		/* switch back to rx */
		keyer_append(rxcontrolstring);
	    } else {
		if (x == ESCAPE) {
		    stoptx();
		}
	    }

	    break;
	}


	if (x == KEY_BACKSPACE
		|| x == KEY_DC /* delete-character key */
		|| x == KEY_LEFT) {

	    x = 0;  // default: no operation

	    if (keyer_backspace) {
		x = KEY_BACKSPACE;
		if (keyerstringpos > 0) {
		    //
		    // remove last char
		    //
		    --keyerstringpos;
		    keyerstring[keyerstringpos] = '\0';
		}
	    }
	}

	x = toupper(x);

	if ((x >= ' ' && x <= 'Z') || x == LINEFEED) { /* ~printable or LF */
	    if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
		mfj1278_control(x);
	    } else if (cwkeyer == NET_KEYER || cwkeyer == HAMLIB_KEYER) {
		keyer_append_char(x);
	    }

	    /* if display field is full move text one left */
	    if (keyerstringpos == KEYER_LINE_WIDTH - 1) {
		for (j = 0; j < KEYER_LINE_WIDTH - 1; j++) {
		    keyerstring[j] = keyerstring[j + 1];
		}
		keyerstringpos--;
	    }
	    /* add new character for display */
	    keyerstring[keyerstringpos++] = x;
	    keyerstring[keyerstringpos] = '\0';

	} else {

	    switch (x) {
		case '|': { // new line
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(crcontrolstring);
		    } else if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
			keyer_append_char('\n');
		    }
		    break;
		}

		case '{': { // start TX
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(txcontrolstring);
		    } else if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
			keyer_append_char(CTRL_T);
		    }
		    break;
		}

		case '}': { // switch to RX
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(rxcontrolstring);
		    } else if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
			keyer_append_char(CTRL_R);
		    }
		    break;
		}
		case '\\': {
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(ctl_c_controlstring);
		    }
		    break;
		}

		case ALT_W: {	// Alt-W, set weight
		    mvaddstr(1, 0, "Weight=   ");
		    refreshp();
		    move(1, 7);
		    echo();
		    getnstr(weightbuf, 2);
		    noecho();

		    weight = atoi(weightbuf);
		    netkeyer(K_WEIGHT, weightbuf);
		    break;
		}

		// <Page-Up>, increase CW speed.
		case KEY_PPAGE: {
		    speedup();
		    show_header_line();
		    break;
		}

		// <Page-Down>, decrease CW speed.
		case KEY_NPAGE: {
		    speeddown();
		    show_header_line();
		    break;
		}

		case KEY_F(1): {
		    getyx(stdscr, cury, curx);
		    move(5, 0);
		    send_keyer_message(0);	/* F1 */
		    move(cury, curx);
		    break;
		}
		case KEY_F(2): {
		    send_keyer_message(1);	/* F2 */
		    break;
		}
		case KEY_F(3): {
		    send_keyer_message(2);	/* F3 */
		    break;
		}
		case KEY_F(4): {
		    send_keyer_message(3);	/* F4 */
		    break;
		}
		case KEY_F(5): {
		    send_keyer_message(4);	/* F5 */
		    break;
		}
		case KEY_F(6): {
		    send_keyer_message(5);	/* F6 */
		    break;
		}
		case KEY_F(7): {
		    send_keyer_message(6);	/* F7 */
		    break;
		}
		case KEY_F(8): {
		    send_keyer_message(7);	/* F8 */
		    break;
		}
		case KEY_F(9): {
		    send_keyer_message(8);	/* F9 */
		    break;
		}
		case KEY_F(10): {
		    send_keyer_message(9);	/* F10 */
		    break;
		}
		case KEY_F(11): {
		    send_keyer_message(10);	/* F11 */
		    break;
		}
		case KEY_F(12): {
		    send_keyer_message(11);	/* F12 */
		    break;
		}
		case KEY_BACKSPACE: {
		    keyer_append_char('\b');    /* ASCII BS */
		    break;
		}
	    }

	    show_panel(panel);
	}
    }
    hide_panel(panel);

    cqmode = cqmode_save;

    clear_display();
}

/* ----------------  convert input for 1278 ctrl -----------------------*/

void mfj1278_control(int x) {

    if (trxmode == CWMODE || trxmode == DIGIMODE) {

	if (trxmode == DIGIMODE) {
	    if (x == LINEFEED)
		x = RETURN;     // tnc needs CR instead of LF
	}
	keyer_append_char(x);
    }
}
