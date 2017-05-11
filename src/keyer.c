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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*------------------------------------------------------------------------

    CW keyboard routine

------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>

#include "clear_display.h"
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

int keyer(void)
{

    extern int cqmode;
    extern char mode[20];
    extern char message[][80];
    extern int trxmode;
    extern int cwkeyer;
    extern int digikeyer;
    extern int weight;
    extern int keyer_backspace;

    WINDOW *win = NULL;
    PANEL *panel = NULL;

    int x = 0, j = 0;
    int cury, curx;
    char nkbuffer[2];
    char keyerstring[KEYER_LINE_WIDTH+1] = "";
    int keyerstringpos = 0;
    char weightbuf[15];
    const char txcontrolstring[2] = { 20, '\0' };	// ^t
    const char rxcontrolstring[2] = { 18, '\0' };	// ^r
    const char crcontrolstring[2] = { 13, '\0' };	// cr
    const char ctl_c_controlstring[2] = { 92, '\0' };	// '\'

    if ((trxmode == CWMODE && cwkeyer == NO_KEYER) ||
	(trxmode == DIGIMODE && digikeyer == NO_KEYER)) /* no keyer present */
	return 1;

    strcpy(mode, "Keyboard");
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    if (panel == NULL) {
	win = newwin(KEYER_WIN_HEIGHT, KEYER_WIN_WIDTH, KEYER_Y, KEYER_Y );
	if (win == NULL)
	    return 1;
	panel = new_panel(win);
	if (panel == NULL) {
	    delwin(win);
	    return 1;
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
	wmove (win, 1, 1);
	for (j = 0; j < KEYER_LINE_WIDTH; j++) {
	    waddch (win, ' ');
	}
	mvwprintw(win, 1, 1, "%s", keyerstring);
	refreshp();

	x = key_get();

	// Send space instead of double quote.
	if (x == 34) {
	    x = 32;
	}

	// Send space instead of newline or return.
	if (x == '\n' || x == KEY_ENTER)
	    x = 32;

	// <Escape>, Ctrl-K (^K), Alt-k (M-k)
	if (x == 27 || x == 11 || x == 235) {
	    if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
		/* switch back to rx */
		keyer_append(rxcontrolstring);
	    } else {
		stoptx();
	    }

	    break;
	}


        if (x == KEY_BACKSPACE
            || x == KEY_DC /* delete-character key */
            || x == KEY_LEFT) {

            x = 0;  // default: no operation

            if (keyer_backspace && keyerstringpos > 0) {
                //
                // remove last char
                //
                --keyerstringpos;
                keyerstring[keyerstringpos] = '\0';
                x = KEY_BACKSPACE;
            }
        }

	// Promote lower case to upper case.
	if (x > 96 && x < 123)
	    x = x - 32;

	if (x > 9 && x < 91) { 	/* drop all other control char... */
	    if (x > 31 || x == 10) {
		if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
		    mfj1278_control(x);
		} else if (cwkeyer == NET_KEYER) {
		    nkbuffer[0] = x;	// 1 char at the time !
		    nkbuffer[1] = '\0';
		    keyer_append(nkbuffer);
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
	    }
	} else {

	    switch (x) {
	    case '\n':
	    case 13:
	    case KEY_ENTER:
		{
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(crcontrolstring);
		    }
		    break;
		}

	    case 123:		/* { */
		{
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(txcontrolstring);
		    }
		    break;
		}
	    case 125:		/* } */
		{
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(rxcontrolstring);
		    }
		    break;
		}
	    case 92:		/* \ */
		{
		    if (cwkeyer == MFJ1278_KEYER ||
			    digikeyer == MFJ1278_KEYER) {
			sendmessage(ctl_c_controlstring);
		    }
		    break;
		}

	    case 247:		// Alt-w, set weight
		{
		    mvprintw(1, 0, "Weight=   ");
		    mvprintw(1, 7, "");
		    refreshp();
		    echo();
		    getnstr(weightbuf, 2);
		    noecho();

		    weight = atoi(weightbuf);
		    netkeyer(K_WEIGHT, weightbuf);
		    break;
		}

	    // <Page-Up>, increase CW speed.
	    case KEY_PPAGE:
		{
		    speedup();
		    clear_display();
		    break;
		}

	    // <Page-Down>, decrease CW speed.
	    case KEY_NPAGE:
		{
		    speeddown();
		    clear_display();
		    break;
		}

	    case KEY_F(1):
		{
		    getyx(stdscr, cury, curx);
		    mvprintw(5, 0, "");
		    sendmessage(message[0]);	/* F1 */
		    mvprintw(cury, curx, "");
		    break;
		}
	    case KEY_F(2):
		{
		    sendmessage(message[1]);	/* F2 */
		    break;
		}
	    case KEY_F(3):
		{
		    sendmessage(message[2]);	/* F3 */
		    break;
		}
	    case KEY_F(4):
		{
		    sendmessage(message[3]);	/* F4 */
		    break;
		}
	    case KEY_F(5):
		{
		    sendmessage(message[4]);	/* F5 */
		    break;
		}
	    case KEY_F(6):
		{
		    sendmessage(message[5]);	/* F6 */
		    break;
		}
	    case KEY_F(7):
		{
		    sendmessage(message[6]);	/* F7 */
		    break;
		}
	    case KEY_F(8):
		{
		    sendmessage(message[7]);	/* F8 */
		    break;
		}
	    case KEY_F(9):
		{
		    sendmessage(message[8]);	/* F9 */
		    break;
		}
	    case KEY_F(10):
		{
		    sendmessage(message[9]);	/* F10 */
		    break;
		}

	    case KEY_F(11):
		{
		    sendmessage(message[10]);	/* F11 */
		    break;
		}
	    case KEY_F(12):
		{

		    sendmessage(message[11]);	/* F12 */
		    break;
		}
	    case KEY_BACKSPACE:
		{
		    sendmessage("\b");          /* ASCII BS */
		    break;
		}

	    default:
		x = x;
	    }

	}
    }
    hide_panel(panel);

    if (cqmode == CQ)
        strcpy(mode, "Log     ");
    else
        strcpy(mode, "S&P     ");

    clear_display();

    return 0;			/* show end of keyer  routine */
}

/* ----------------  convert input for 1278 ctrl -----------------------*/

void mfj1278_control(int x)
{
    extern int trxmode;

    char buffer[2];

    if (trxmode == CWMODE || trxmode == DIGIMODE) {

	if (trxmode == DIGIMODE) {
	    if (x == 10)
		x = 13;		// tnc needs CR instead of LF
	}
	buffer[0] = x;		// 1 char at the time !
	buffer[1] = '\0';
	keyer_append(buffer);
    }
}
