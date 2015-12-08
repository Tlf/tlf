/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2015 Thomas Beierlein <tb@forth-ev.de>
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

/* User Interface helpers for ncurses based user interface */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_NCURSES_NCURSES_H
# include <ncurses/ncurses.h>
#elif defined HAVE_NCURSES_CURSES_H
# include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
# include <ncurses.h>
#elif defined HAVE_CURSES_H
# include <curses.h>
#endif

#include "stoptx.h"


extern int use_rxvt;

static int getkey(int wait);
static int onechar(void);


/** add A_BOLD to attributes if 'use_rxvt' is not set */
int modify_attr( int attr ) {

    if (use_rxvt == 0)
	attr |= A_BOLD;

    return attr;
}

/** key_get  wait for next key from terminal
 *
 */
int key_get()
{
    return getkey(1);
}

/** key_poll return next key from terminal if there is one
 *
 */
int key_poll()
{
    return getkey(0);
}


/* helper function to set 'nodelay' mode according to 'wait'
 * parameter and then ask for the next character
 * leaves 'nodelay' afterwards always as FALSE (meaning: wait for
 * character
 */
static int getkey(int wait)
{
    int x = 0;

    nodelay(stdscr, wait ? FALSE : TRUE);

    x = onechar();

    nodelay(stdscr, FALSE);

    return x;
}

/* Original key input routine moved here to make it static. Usage is
 * hidden by the new key_poll() and key_get() functions.
 * Partially decodes the ESC key sequences for different terminals.
 *
 * The routine will be replaced in near future by switching to the curses
 * keypad mode. That has some advantages:
 * - more terminal types can be handled
 * - we get a SIGWINCH code for resizing of the terminal
 * - we can handle mouse input via curses
 */
static int onechar(void)
{
    extern int use_xterm;

    int x = 0;
    int trash = 0;

    x = getch();

    if (x == 8)
	x = 127;			/* replace Ctrl-H bei Backspace */

    if (x == 27) {
	nodelay(stdscr, TRUE);
	x = getch();

	if (x != 91) {
	    switch (x) {

	    case 79:
		x = getch();
		if (x >= 80 && x <= 84)
		    x += 49;
		break;

	    case 32 ... 57:	//   alt-space to alt-9,   160 - 186
	    case 97 ... 122:	//   alt-a to alt-z,     225 -  250
		x += 128;
		break;
	    case 65 ... 78:	//   alt-A to alt-Z,     225 -  250
	    case 80 ... 90:	//   alt-A to alt-Z,     225 -  250
		x += 160;
		break;

	    default:{
		    x = 27;
		    stoptx();
		}
	    }
	    nodelay(stdscr, FALSE);

	} else {
	    nodelay(stdscr, FALSE);

	    x = getch();	/* remove '91 */

	    switch (x) {
	    case 49:
		{
		    x = getch();

		    if (x == 126) {
			x = 158;	/* home */
			break;
		    } else {
			x = x + 79;

			if (use_xterm == 1 && x <= 132)
			    x++;

			trash = getch();
			break;	/* F6 F7 F8, 134 135 136 */
		    }
		}
	    case 50:
		{
		    x = getch();
		    if (x == 126) {
			x = 160;	/* insert */
			break;
		    } else {
			x = x + 89;
			trash = getch();
			break;	/* F9 - SF4, 137, 138, 140, 141; 142, 143, 145, 146 */

		    }
		}
	    case 51:
		{
		    x = getch();
		    if (x == 126) {
			x = 161;	/* delete */
			break;
		    } else {
			x = x + 98;
			trash = getch();
			break;	/* SF5 - SF8, 147, 148, 149, 150  */
		    }
		}
	    case 52:		/* end */
		{
		    x = 159;
		    trash = getch();
		    break;
		}
	    case 53:		/* pgup */
		{
		    x = 156;

		    trash = getch();
		    if (use_xterm == 0) {
			if (trash == 94)
			    x = x + 256;	// 412, ctrl-pgup
		    } else {

			if (trash == 59) {
			    x = x + 256;
			    trash = getch();
			    trash = getch();
			}
		    }
		    break;
		}
	    case 54:		/* pgdwn */
		{
		    x = 157;
		    trash = getch();
		    if (use_xterm == 0) {
			if (trash == 94)
			    x = x + 256;	// 413, ctrl-pgup
		    } else {
			if (trash == 59) {
			    x = x + 256;
			    trash = getch();
			    trash = getch();
			}
		    }
		    break;
		}
	    case 65:
		{
		    x = 152;	/* up */
		    break;
		}
	    case 66:
		{
		    x = 153;	/* dwn */
		    break;
		}
	    case 67:
		{
		    x = 154;	/* right */
		    break;
		}
	    case 68:
		{
		    x = 155;	/* left */
		    break;
		}
	    case 91:
		{
		    if (use_xterm == 0) {
			x = getch();
			if (x >= 65 && x <= 69) {	/* F1 - F5, 129 - 134 */
			    x = x + 64;
			    break;
			}
		    }
		}
	    default:
		{
		    x = x;

		}
	    }			// end switch
	}			// end else
    }				// end if x=27

    return (x);
}
