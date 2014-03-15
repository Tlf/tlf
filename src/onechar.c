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
	 *        Onechar handles keyboard input and takes care  of
	 *                   escape sequences
	 *--------------------------------------------------------------*/

#include "onechar.h"
#include <syslog.h>

int onechar(void)
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
	    if (x > 0) {
	      syslog(LOG_DEBUG, "1st: %d", x);
	    }
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
