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

/*------------------------------------------------------------------------

    CW keyboard routine

------------------------------------------------------------------------*/
#include "keyer.h"
#include "netkeyer.h"

int mfj1278_control(int x);

int keyer(void)
{

    extern char mode[20];
    extern int cury, curx;
    extern int bufloc;
    extern char buffer[];
    extern char termbuf[];
    extern int keyspeed;
    extern char message[15][80];
    extern char wkeyerbuffer[];
    extern int data_ready;
    extern int keyerport;
    extern int weight;

    int x = 0, i = 0, j = 0;
    char nkbuffer[2];
    char keyerstring[30] = "                              ";
    char weightbuf[15];
    const char txcontrolstring[2] = { 20, '\0' };
    const char rxcontrolstring[2] = { 18, '\0' };
    const char crcontrolstring[2] = { 13, '\0' };
    const char ctl_c_controlstring[2] = { 92, '\0' };

    strcpy(mode, "Keyboard");
    clear_display();
    attron(COLOR_PAIR(7) | A_STANDOUT);

    if (keyerport == NO_KEYER)	/* no keyer present */
	return (1);

    if (keyerport == MFJ1278_KEYER) {
	buffer[0] = 20;		// 1 char at the time !
	buffer[1] = '\0';
	if (data_ready != 1) {
	    strcat(wkeyerbuffer, buffer);
	    data_ready = 1;
	}
	buffer[0] = '\0';
    }

    while (1) {
	x = onechar();

	if (x == 34) {		/* bug fix */// "
	    x = 32;
	}

	if (x == '\n')
	    x = 32;

	if (x == 27 || x == 11 || x == 235) {	//      esc, ctrl-k,  alt-k
	    if (keyerport == MFJ1278_KEYER) {
		buffer[0] = 18;	// 1 char at the time !
		buffer[1] = '\0';
		if (data_ready != 1) {
		    strcat(wkeyerbuffer, buffer);
		    data_ready = 1;
		}
	    } else {
		stoptx();
	    }

	    buffer[0] = '\0';
	    break;
	}

	if (x >= 32 && x <= 125) {	// display              space ... }
	    addch(x);
	    refresh();
	    i++;
	    if ((i >= 40)) {
		i = 0;
		mvprintw(4, 0, "                            ");
		mvprintw(4, 0, "");
		refresh();
		displayit();

	    }

	    refresh();
	}

	if (x == 127 && (strlen(buffer) >= 1)) {	/* erase  */

	    getyx(stdscr, cury, curx);
	    mvprintw(5, curx - 1, " ");
	    mvprintw(5, curx - 1, "");
	    buffer[strlen(buffer) - 1] = '\0';
	    bufloc--;
	}

	if (x > 96 && x < 123)	/* upper case only */
	    x = x - 32;

	if (x > 9 && x < 91) {

	    if (bufloc >= 38)	// maximum buffer = 39
	    {
		bufloc = 38;
		printw("\nBuffer overflow !, bufloc = %d\n", bufloc);
		refresh();
	    } else {
		if (x > 31 || x == 10) {
		    if (keyerport == MFJ1278_KEYER) {
			mfj1278_control(x);
		    } else if (keyerport == NET_KEYER) {
			nkbuffer[0] = x;	// 1 char at the time !
			nkbuffer[1] = '\0';
			netkeyer(K_MESSAGE, nkbuffer);
			nkbuffer[0] = '\0';
			for (j = 0; j < 29; j++) {
			    keyerstring[j] = keyerstring[j + 1];
			}
			keyerstring[28] = x;
			keyerstring[29] = '\0';

			attron(COLOR_PAIR(7) | A_STANDOUT);
			mvprintw(5, 0, "%s", keyerstring);
			refresh();
		    } else if (keyerport == ORION_KEYER) {
			nkbuffer[0] = x;
			nkbuffer[1] = '\0';
			strcat(wkeyerbuffer, nkbuffer);
			sendbuf();
			nkbuffer[0] = '\0';
			for (j = 0; j < 29; j++) {
			    keyerstring[j] = keyerstring[j + 1];
			}
			keyerstring[28] = x;
			keyerstring[29] = '\0';

			attron(COLOR_PAIR(7) | A_STANDOUT);
			mvprintw(5, 0, "%s", keyerstring);
			refresh();

		    }
		} else		// control char...
		{

		    if (data_ready != 1) {
			strcat(wkeyerbuffer, buffer);
			data_ready = 1;
		    } else
			buffer[0] = '\0';

		    getyx(stdscr, cury, curx);
		    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
		    mvaddstr(0, 0, "  ");
		    attron(COLOR_PAIR(7));
		    mvaddstr(cury, curx, "");
		    refresh();

		    strcat(termbuf, buffer);
		    strcat(termbuf, " ");
		    mvprintw(5, 0, termbuf);
		    refresh();

		    if ((strlen(buffer) + strlen(termbuf) > 39)
			|| x == '=') {
			i = 0;
			mvprintw(5, 0, "                         ");
			mvprintw(5, 0, "");
			refresh();
			displayit();
		    }

		    bufloc = 0;
		    buffer[bufloc] = '\0';
		}
	    }
	} else {

	    switch (x) {
	    case 9:
		{
		    bufloc = 0;
		    buffer[bufloc] = '\0';
		    strcpy(mode, "Log     ");
		    clear_display();
		    return (2);
		}
	    case '\n':
	    case 13:
		{
		    if (keyerport == MFJ1278_KEYER && strlen(buffer) < 39) {
			strcat(buffer, crcontrolstring);
			sendbuf();
			bufloc = 0;
		    }
		    break;
		}

	    case 27:
	    case 11:
		{
		    stoptx();
		    bufloc = 0;
		    buffer[bufloc] = '\0';
		    strcpy(mode, "Log     ");
		    clear_display();
		    return (2);
		}
	    case 123:
		{
		    if (keyerport == MFJ1278_KEYER) {
			strcat(buffer, txcontrolstring);
			sendbuf();
		    }
		    break;
		}
	    case 125:
		{
		    if (keyerport == MFJ1278_KEYER) {
			strcat(buffer, rxcontrolstring);
			sendbuf();
		    }
		    break;
		}
	    case 92:
		{
		    if (keyerport == MFJ1278_KEYER) {
			strcat(buffer, ctl_c_controlstring);
			sendbuf();
		    }
		    break;
		}

	    case 247:		// Alt-w, set weight
		{
		    mvprintw(1, 0, "Weight=   ");
		    mvprintw(1, 7, "");
		    refresh();
		    echo();
		    getnstr(weightbuf, 2);
		    noecho();

		    weight = atoi(weightbuf);
		    netkeyer(K_WEIGHT, weightbuf);
		    break;
		}
	    case 156:
		{
		    keyspeed = speedup();
		    clear_display();
		    break;
		}
	    case 157:
		{
		    keyspeed = speeddown();
		    clear_display();
		    break;
		}

	    case 129:
		{
		    strcat(buffer, message[0]);	/* F1 */
		    getyx(stdscr, cury, curx);
		    mvprintw(5, 0, "");
		    sendbuf();
		    mvprintw(cury, curx, "");
		    break;
		}
	    case 130:
		{
		    strcat(buffer, message[1]);	/* F2 */
		    sendbuf();
		    break;
		}
	    case 131:
		{
		    strcat(buffer, message[2]);	/* F3 */
		    sendbuf();
		    break;
		}
	    case 132:
		{
		    strcat(buffer, message[3]);	/* F4 */
		    sendbuf();
		    break;
		}
	    case 133:
		{
		    strcat(buffer, message[4]);	/* F5 */
		    sendbuf();
		    break;
		}
	    case 134:
		{
		    strcat(buffer, message[5]);	/* F6 */
		    sendbuf();
		    break;
		}
	    case 135:
		{
		    strcat(buffer, message[6]);	/* F7 */
		    sendbuf();
		    break;
		}
	    case 136:
		{
		    strcat(buffer, message[7]);	/* F8 */
		    sendbuf();
		    break;
		}
	    case 137:
		{
		    strcat(buffer, message[8]);	/* F9 */
		    sendbuf();
		    break;
		}
	    case 138:
		{
		    strcat(buffer, message[9]);	/* F10 */
		    sendbuf();
		    break;
		}

	    case 140:
		{
		    strcat(buffer, message[10]);	/* F11 */
		    sendbuf();
		    break;
		}
	    case 141:
		{

		    strcat(buffer, message[11]);	/* F12 */
		    sendbuf();
		    break;
		}

	    case 142 ... 150:	/* CTRL  O */

		{
		    message_change(x);

		    break;

		}

	    default:
		x = x;
	    }

	}
    }

    strcpy(mode, "Log     ");
    clear_display();

    return (2);			/* show end of keyer  routine */
}

/*  ------------------------------------------  convert input for 1278 ctrl -----------------------*/

int mfj1278_control(int x)
{

    extern int trxmode;
    extern char wkeyerbuffer[];
    extern int data_ready;

    int y = 0;
    char buffer[2];

    if (trxmode == CWMODE || trxmode == DIGIMODE) {

	if (trxmode == DIGIMODE) {
	    if (x == 10)
		x = 13;		// tnc needs CR
	}
	buffer[0] = x;		// 1 char at the time !
	buffer[1] = '\0';
	if (data_ready != 1) {
	    strcat(wkeyerbuffer, buffer);
	    data_ready = 1;
	}
	buffer[0] = '\0';

    }

    y = x;

    return (y);
}
