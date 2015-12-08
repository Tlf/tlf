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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <unistd.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_NCURSES_NCURSES_H
# include <ncurses/ncurses.h>
#elif defined HAVE_NCURSES_CURSES_H
# include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
# include <ncurses.h>
#elif defined HAVE_CURSES_H
# include <curses.h>
#endif

#include "tlf.h"


static int linectr; // global

void showmsg(char *message) {

extern int verbose;

 if (linectr == 24) linectr = 1;

 mvprintw(linectr, 0, message);
 refreshp();
 if (verbose == 1) sleep(1);
 linectr++;
}
//---------------------------------------------------------------

void shownr(char *message, int nr) {

extern int verbose;

 if (linectr == 24) linectr = 1;

 mvprintw(linectr, 0, "%s %d", message, nr);
 refreshp();
 if (verbose == 1) sleep(1);
 linectr++;
}
//----------------------------------------------------------------

void showstring(char *message1, char *message2) {

  extern int verbose;

 if (linectr == 24) linectr = 1;

 mvprintw(linectr, 0, "%s %s", message1, message2);
 refreshp();
 if (verbose == 1) sleep(1);
 linectr++;
}
