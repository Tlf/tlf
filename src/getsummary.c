/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012           Thomas Beierlein <tb@forth-ev.de>
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
 *   write cabrillo header
 *
 *--------------------------------------------------------------*/


#include <stdio.h>

#include <glib.h>

#include "globalvars.h"
#include "nicebox.h"		// Includes curses.h
#include "ui_utils.h"


void ask(char *buffer, char *what) {

    attron(A_STANDOUT);
    mvprintw(15, 1, spaces(78));
    nicebox(14, 0, 1, 78, what);
    attron(A_STANDOUT);
    mvprintw(15, 1, "");

    echo();
    getnstr(buffer, 78);
    noecho();
    g_strstrip(buffer);
}

