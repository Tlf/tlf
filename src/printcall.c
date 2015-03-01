/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Thomas Beierlein <tb@forth-ev.de>
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
	 *      printcall
	 *
	 *--------------------------------------------------------------*/

#include "tlf.h"
#include "printcall.h"
#include <curses.h>
#include "ui_utils.h"

void printcall(void)
{
    extern char hiscall[];
    extern int miniterm;
    extern int cqmode;
    extern int cwstart;

    int currentterm;
    attr_t attrib = modify_attr(A_STANDOUT);

    currentterm = miniterm;
    miniterm = 0;

    attron(COLOR_PAIR(C_INPUT) | attrib);

    mvprintw(12, 29, "            ");
    mvprintw(12, 29, hiscall);
    if ((cqmode == CQ) && (cwstart != 0))
    	mvchgat(12, 29 + cwstart, 12 - cwstart,
		attrib | A_UNDERLINE, C_INPUT, NULL);
    refreshp();

    miniterm = currentterm;
}

/** highlight the first n characters of the call input field
 *
 * \param n number of characters to highlight
 */
void highlightCall(unsigned int n) {
    attr_t attrib = modify_attr(A_NORMAL);
				/* use NORMAL here as normal display
				   uses STANDOUT */

    mvchgat(12, 29, n, attrib, C_INPUT, NULL);
}
