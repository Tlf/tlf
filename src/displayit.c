/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2014           Thomas Beierlein <tb@forth-ev.de>
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

/* ------------------------------------------------------------------------
*    scroll  the loglines of the keyer terminal and show them
*
---------------------------------------------------------------------------*/


#include <config.h>		// Includes found ncurses.h

#include <string.h>

#include <glib.h>

#include "clear_display.h"


void displayit(void)
{
    extern char termbuf[];
    extern char backgrnd_str[];
    extern char terminal1[];
    extern char terminal2[];
    extern char terminal3[];
    extern char terminal4[];

    char term2buf[81] = "";

    g_strlcpy(term2buf, termbuf, sizeof(term2buf));
    g_strchomp(term2buf);
    g_strlcat(term2buf, backgrnd_str, sizeof(term2buf));	/* fill with blanks */

    strcpy(terminal1, terminal2);
    strcpy(terminal2, terminal3);
    strcpy(terminal3, terminal4);
    strcpy(terminal4, term2buf);
    termbuf[0] = '\0';
    mvprintw(5, 0, "");

    clear_display();
}
