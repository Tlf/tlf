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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------
*          Nicebox draws  a  box with  a header
*
*--------------------------------------------------------------*/


#include "nicebox.h"		// Includes curses.h
#include "globalvars.h"
#include "ui_utils.h"


void wnicebox(WINDOW *win, int y, int x, int height, int width, char *boxname) {

    height += 1;
    width += 1;

    wattrset(win, modify_attr(COLOR_PAIR(C_BORDER)));

    mvwaddch(win, y, x, ACS_ULCORNER);
    whline(win, ACS_HLINE, width);
    mvwaddch(win, y, x + width, ACS_URCORNER);
    mvwaddch(win, y + height, x, ACS_LLCORNER);
    whline(win, ACS_HLINE, width);
    mvwaddch(win, y + height, x + width, ACS_LRCORNER);
    mvwvline(win, y + 1, x + width, ACS_VLINE, height - 1);
    mvwvline(win, y + 1, x, ACS_VLINE, height - 1);
    mvwprintw(win, y, x + 2, "%s", boxname);

    return;
}

void nicebox(int y, int x, int height, int width, char *boxname) {
    wnicebox(stdscr, y, x, height, width, boxname);
}

void ask(char *buffer, char *what) {

    attron(A_STANDOUT);
    mvaddstr(15, 1, spaces(78));
    nicebox(14, 0, 1, 78, what);
    attron(A_STANDOUT);
    move(15, 1);

    echo();
    getnstr(buffer, 78);
    noecho();
    g_strstrip(buffer);
}
