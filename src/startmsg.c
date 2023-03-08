/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0r@eudxf.org>
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


#include <unistd.h>

#include "ignore_unused.h"
#include "globalvars.h"
#include "ui_utils.h"


static int linectr = 0;

void clearmsg() {
    clear();
    linectr = 0;
    refreshp();
}


void clearmsg_wait(void) {
    if (verbose) {
	move(LINES - 3, 0);
	clrtoeol();
	mvaddstr(LINES - 2, 0, "Press any key to continue!");
	move(LINES - 1, 0);
	clrtoeol();
	refreshp();
	IGNORE(key_get());
    } else {
	sleep(1);
    }
    clearmsg();
}


static int has_room_for_message() {
    if (linectr < LINES - 3)
	return 1;
    else
	return 0;
}

void showmsg(char *message) {
    if (!has_room_for_message())
	clearmsg_wait();
    mvaddstr(linectr, 0, message);
    refreshp();
    linectr++;
}
//---------------------------------------------------------------

void shownr(char *message, int nr) {
    if (!has_room_for_message())
	clearmsg_wait();
    mvprintw(linectr, 0, "%s %d", message, nr);
    refreshp();
    linectr++;
}
//----------------------------------------------------------------

void showstring(const char *message1, const char *message2) {
    if (!has_room_for_message())
	clearmsg_wait();
    mvprintw(linectr, 0, "%s %s", message1, message2);
    refreshp();
    linectr++;
}
