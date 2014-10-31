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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <panel.h>
#include "tlf.h"
#include "clear_display.h"
#include "onechar.h"
#include "stoptx.h"
#include "displayit.h"
#include "speedupndown.h"
#include "sendbuf.h"
#include "netkeyer.h"
#include "nicebox.h"

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
    extern char wkeyerbuffer[];
    extern int data_ready;
    extern int keyerport;
    extern int weight;

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

    if (keyerport == NO_KEYER)	/* no keyer present */
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

    if (keyerport == MFJ1278_KEYER) {
	if (data_ready != 1) { 		/* switch to tx */
	    strcat(wkeyerbuffer, txcontrolstring);
	    data_ready = 1;
	}
    }

    while (1) {
	wattron(win, COLOR_PAIR(C_LOG) | A_STANDOUT);
	wmove (win, 1, 1);
	for (j = 0; j < KEYER_LINE_WIDTH; j++) {
	    waddch (win, ' ');
	}
	mvwprintw(win, 1, 1, "%s", keyerstring);
	refreshp();

	x = onechar();

	if (x == 34) {		/* skip " */
	    x = 32;
	}

	if (x == '\n')
	    x = 32;

	if (x == 27 || x == 11 || x == 235) {	//      esc, ctrl-k,  alt-k
	    if (keyerport == MFJ1278_KEYER) {
		if (data_ready != 1) { 	/* switch back to rx */
		    strcat(wkeyerbuffer, rxcontrolstring);
		    data_ready = 1;
		}
	    } else {
		stoptx();
	    }

	    break;
	}

	if (x > 96 && x < 123)	/* upper case only */
	    x = x - 32;

	if (x > 9 && x < 91) { 	/* drop all other control char... */
	    if (x > 31 || x == 10) {
		if (keyerport == MFJ1278_KEYER) {
		    mfj1278_control(x);
		} else if (keyerport == NET_KEYER) {
		    nkbuffer[0] = x;	// 1 char at the time !
		    nkbuffer[1] = '\0';
		    netkeyer(K_MESSAGE, nkbuffer);
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
		{
		    if (keyerport == MFJ1278_KEYER) {
			sendmessage(crcontrolstring);
		    }
		    break;
		}

	    case 123:		/* { */
		{
		    if (keyerport == MFJ1278_KEYER) {
			sendmessage(txcontrolstring);
		    }
		    break;
		}
	    case 125:		/* } */
		{
		    if (keyerport == MFJ1278_KEYER) {
			sendmessage(rxcontrolstring);
		    }
		    break;
		}
	    case 92:		/* \ */
		{
		    if (keyerport == MFJ1278_KEYER) {
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
	    case 156:
		{
		    speedup();
		    clear_display();
		    break;
		}
	    case 157:
		{
		    speeddown();
		    clear_display();
		    break;
		}

	    case 129:
		{
		    getyx(stdscr, cury, curx);
		    mvprintw(5, 0, "");
		    sendmessage(message[0]);	/* F1 */
		    mvprintw(cury, curx, "");
		    break;
		}
	    case 130:
		{
		    sendmessage(message[1]);	/* F2 */
		    break;
		}
	    case 131:
		{
		    sendmessage(message[2]);	/* F3 */
		    break;
		}
	    case 132:
		{
		    sendmessage(message[3]);	/* F4 */
		    break;
		}
	    case 133:
		{
		    sendmessage(message[4]);	/* F5 */
		    break;
		}
	    case 134:
		{
		    sendmessage(message[5]);	/* F6 */
		    break;
		}
	    case 135:
		{
		    sendmessage(message[6]);	/* F7 */
		    break;
		}
	    case 136:
		{
		    sendmessage(message[7]);	/* F8 */
		    break;
		}
	    case 137:
		{
		    sendmessage(message[8]);	/* F9 */
		    break;
		}
	    case 138:
		{
		    sendmessage(message[9]);	/* F10 */
		    break;
		}

	    case 140:
		{
		    sendmessage(message[10]);	/* F11 */
		    break;
		}
	    case 141:
		{

		    sendmessage(message[11]);	/* F12 */
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
    extern char wkeyerbuffer[];
    extern int data_ready;

    char buffer[2];

    if (trxmode == CWMODE || trxmode == DIGIMODE) {

	if (trxmode == DIGIMODE) {
	    if (x == 10)
		x = 13;		// tnc needs CR instead of LF
	}
	buffer[0] = x;		// 1 char at the time !
	buffer[1] = '\0';
	if (data_ready != 1) {
	    strcat(wkeyerbuffer, buffer);
	    data_ready = 1;
	}
	buffer[0] = '\0';
    }
}
