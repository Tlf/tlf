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

    CW keyboard and common key handling

------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

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
#include "audio.h"
#include "change_rst.h"
#include "cw_utils.h"
#include "sendspcall.h"
#include "cqww_simulator.h"

/* size and position of keyer window */
#define KEYER_LINE_WIDTH 60
#define KEYER_WIN_WIDTH (KEYER_LINE_WIDTH+2)
#define KEYER_WIN_HEIGHT 3
#define KEYER_X (80-KEYER_WIN_WIDTH)/2
#define KEYER_Y 7


void mfj1278_control(int x);

static void tune() {
    int count;
    int count2;
    gchar *buff;

    count2 = tune_seconds;
    while (count2 > 0) {
	if (count2 >= 10) {
	    count = 10;
	} else {
	    count = count2;
	}
	count2 -= count;
	buff = g_strdup_printf("%d", count);
	netkeyer(K_TUNE, buff);	// cw on
	g_free(buff);

	count = count * 4;    // sleeping 1/4 second units between keypress-checks
	while (count > 0) {
	    usleep(250000);
	    if (key_poll() != -1) {	// any key pressed ?
		count2 = 0;    // destroy outer loop as well
		break;
	    }
	    count--;
	}
    }

    netkeyer(K_ABORT, "");	// cw abort
}


//
// handle common keys
// F1..F11, Alt-0..9, _ (underscore), PgUp, PgDn, Alt-W, Alt-T
//
// returns 0:   if the key was handled
//         key: if the key was not handled
//
int handle_common_key(int key) {
    bool handled = true;
    switch (key) {
	// F1, send CQ or S&P call message. (???)
	case KEY_F(1): {
	    if (trxmode == CWMODE || trxmode == DIGIMODE) {
		//FIXME when called from getexchange then just send my.call
		if (cqmode == S_P) {
		    sendspcall();
		} else {
		    send_standard_message(0);	/* CQ */
		}

		set_simulator_state(CALL);

	    } else {
		if (cqmode == S_P)
		    vk_play_file(ph_message[5]);	/* S&P */
		else
		    vk_play_file(ph_message[0]);
	    }
	    break;
	}

	// F2-F11, send messages 2 through 11.
	case KEY_F(2) ... KEY_F(11): {
	    // F2...F11 - F1 = 1...10
	    if (*current_qso.call == '\0') {
		send_standard_message_prev_qso(key - KEY_F(1));
	    } else {
		send_standard_message(key - KEY_F(1));
	    }

	    break;
	}

	// Underscore, confirm last exchange.
	case '_': {
	    if (S_P == cqmode) {
		send_standard_message_prev_qso(SP_TU_MSG);
	    } else {
		send_standard_message_prev_qso(2);
	    }

	    break;
	}

	// <Page-Up>, change RST if call field not empty, else increase CW speed.
	case KEY_PPAGE: {
	    if (change_rst && (strlen(current_qso.call) != 0)) {	// change RST

		rst_sent_up();

		if (!no_rst)
		    mvaddstr(12, 44, sent_rst);
		mvaddstr(12, 29, current_qso.call);

	    } else {	// change cw speed
		speedup();

		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 14, "%2u", GetCWSpeed());
	    }

	    break;
	}

	// Alt-0 to Alt-9 (M-0...M-9), send CW/Digimode messages 15-24.
	case 128+'0' ... 128+'9': {
	    int index = key - (128 + '0') + CQ_TU_MSG + 1;
	    if (*current_qso.call == '\0') {
		send_standard_message_prev_qso(index);
	    } else {
		send_standard_message(index);
	    }

	    break;
	}

	// <Page-Down>, change RST if call field not empty, else decrease CW speed.
	case KEY_NPAGE: {
	    if (change_rst && (strlen(current_qso.call) != 0)) {

		rst_sent_down();

		if (!no_rst)
		    mvaddstr(12, 44, sent_rst);
		mvaddstr(12, 29, current_qso.call);

	    } else {

		speeddown();

		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 14, "%2u", GetCWSpeed());
	    }
	    break;
	}

	// Alt-W (M-W), set Morse weight.
	case ALT_W: {
	    char weightbuf[5] = "";
	    char *end;

	    mvaddstr(12, 29, "Wght: -50..50");

	    nicebox(1, 1, 2, 12, "CW");
	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
	    mvprintw(2, 2, "Speed:   %2u ", GetCWSpeed());
	    mvprintw(3, 2, "Weight: %3d ", weight);
	    move(3, 10);
	    refreshp();

	    usleep(800000);
	    mvaddstr(3, 10, spaces(3));

	    echo();
	    mvgetnstr(3, 10, weightbuf, 3);
	    noecho();

	    g_strchomp(weightbuf);

	    int tmp = strtol(weightbuf, &end, 10);

	    if (weightbuf[0] != '\0' && *end == '\0') {
		/* successful conversion */
		if (tmp >= -50 && tmp <= 50) {
		    weight = tmp;
		    netkeyer(K_WEIGHT, weightbuf);
		}
	    }
	    clear_display();

	    break;
	}

	// Alt-T (M-T), tune xcvr via cwdaemon.
	case ALT_T: {
	    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
	    mvaddstr(0, 2, "Tune     ");
	    move(12, 29);
	    refreshp();

	    tune(); //FIXME defined in callinput.c

	    show_header_line();
	    refreshp();

	    break;
	}

	default:
	    handled = false;
    }

    return handled ? 0 : key;
}


void keyer(void) {

    static WINDOW *win = NULL;
    static PANEL *panel = NULL;

    int x = 0, j = 0;
    char keyerstring[KEYER_LINE_WIDTH + 1] = "";
    int keyerstringpos = 0;
    const char txcontrolstring[2] = { CTRL_T, '\0' };	// ^t
    const char rxcontrolstring[2] = { CTRL_R, '\0' };	// ^r
    const char crcontrolstring[2] = { RETURN, '\0' };	// cr
    const char ctl_c_controlstring[2] = { BACKSLASH, '\0' };	// '\'

    if ((trxmode == CWMODE && cwkeyer == NO_KEYER) ||
	    (trxmode == DIGIMODE && digikeyer == NO_KEYER)) {
	return; /* no keyer present */
    }

    keyboard_mode = true;
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

	// <Escape>, Ctrl-K (^K), Alt-k (M-k), ` (Grave Accent)
	if (x == ESCAPE || x == CTRL_K || x == ALT_K || x == '`') {
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

	x = handle_common_key(x);

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
	    // special keys
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

		case KEY_BACKSPACE: {
		    keyer_append_char('\b');    /* ASCII BS */
		    break;
		}
	    }

	    show_panel(panel);
	}
    }
    hide_panel(panel);

    keyboard_mode = false;

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
