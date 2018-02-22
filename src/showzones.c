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
 *   show zone map
 *
 *--------------------------------------------------------------*/


#include "nicebox.h"		// Includes curses.h
#include "tlf.h"
#include "tlf_panel.h"


int show_zones(int bandinx) {
    extern int zonedisplay;
    extern int zones[MAX_ZONES];

    static WINDOW *zones_win = NULL;
    static PANEL *zones_panel = NULL;

    int i = 0, j = 0;
    int xloc = 1;
    int yloc = 1;
    int zonenr;

    if (zones_panel == NULL) {
	zones_win = newwin(10, 18, 14, 22);
	if (zones_win == NULL)
	    return -1;
	zones_panel = new_panel(zones_win);
	if (zones_panel == NULL) {
	    delwin(zones_win);
	    return -1;
	}
    }

    if (zonedisplay != 1) {
	hide_panel(zones_panel);
	return (0);
    }

    show_panel(zones_panel);
    top_panel(zones_panel);
    werase(zones_win);
    wnicebox(zones_win, 0, 0, 8, 16, "Zones");

    wattron(zones_win, COLOR_PAIR(C_INPUT) | A_STANDOUT);

    for (i = 0; i < 8; i++) {

	wmove(zones_win, i + yloc, xloc);
	for (j = 0; j < 5; j++) {

	    zonenr = (i * 5) + j + 1;   /* 1.. 40 */
	    if ((zones[zonenr] & inxes[bandinx]) == 0) {
		/* still to work */
		wprintw(zones_win, " %02d", zonenr);
	    } else {
		/* already worked */
		wprintw(zones_win, "   ");
	    }
	}
	wprintw(zones_win, " ");
    }

    return (0);
}
