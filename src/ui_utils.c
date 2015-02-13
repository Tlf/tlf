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

#include <curses.h>
#include "onechar.h"

extern int use_rxvt;

static int getkey(int wait);


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


