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

    int currentterm;

    currentterm = miniterm;
    miniterm = 0;

    if (use_rxvt == 0)
	attron(COLOR_PAIR(COLOR_BLUE) | A_STANDOUT | A_BOLD);
    else
	attron(COLOR_PAIR(COLOR_BLUE) | A_STANDOUT);

    mvprintw(12, 29, "            ");
    mvprintw(12, 29, hiscall);
    refreshp();

    miniterm = currentterm;
}
