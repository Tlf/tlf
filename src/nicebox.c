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
 	*          Nicebox draws  a  box with  a header
 	*
 	*--------------------------------------------------------------*/

#include "tlf.h"
#include "nicebox.h"

void wnicebox(WINDOW *win, int y, int x, int height, int width, char *boxname)
{
    extern int use_rxvt;

    height += 1;
    width += 1;

    if (use_rxvt == 0)
	wattrset(win, COLOR_PAIR(C_BORDER) | A_BOLD);
    else
	wattrset(win, COLOR_PAIR(C_BORDER));

    mvwaddch(win, y, x, ACS_ULCORNER);
    whline(win, ACS_HLINE, width);
    mvwaddch(win, y, x + width, ACS_URCORNER);
    mvwaddch(win, y + height, x, ACS_LLCORNER);
    whline(win, ACS_HLINE, width);
    mvwaddch(win, y + height, x + width, ACS_LRCORNER);
    mvwvline(win, y + 1, x + width, ACS_VLINE, height - 1);
    mvwvline(win, y + 1, x, ACS_VLINE, height - 1);
    mvwprintw(win, y, x + 2, boxname);

    return;
}

void nicebox(int y, int x, int height, int width, char *boxname)
{
	wnicebox( stdscr, y, x, height, width, boxname);
}

