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
	 *      printcall
	 *
	 *--------------------------------------------------------------*/

#include "tlf.h"
#include "printcall.h"

void printcall(void)
{

    extern int use_rxvt;
    extern char hiscall[];
    extern int miniterm;
    extern int cqmode;
    extern int cwstart;

    int currentterm;
    attr_t attrib = A_STANDOUT;

    currentterm = miniterm;
    miniterm = 0;

    if (use_rxvt == 0)
	attrib |= A_BOLD;

    attron(COLOR_PAIR(C_INPUT) | attrib);

    mvprintw(12, 29, "            ");
    mvprintw(12, 29, hiscall);
    if ((cqmode == CQ) && (cwstart != 0))
    	mvchgat(12, 29 + cwstart, 12 - cwstart,
		attrib | A_UNDERLINE, C_INPUT, NULL);
    refreshp();

    miniterm = currentterm;
}
