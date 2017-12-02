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


int show_zones(int bandinx)
{

    extern int zonedisplay;
    extern int bandindex;
    extern int zones[MAX_ZONES];

    static WINDOW *zones_win = NULL;
    static PANEL *zones_panel = NULL;

    int i = 0, j = 0;
    int xloc = -2;
    int yloc = 1;

    if (zones_panel == NULL) {
	zones_win = newwin(10, 18, 14, 22 );
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

    show_panel( zones_panel );
    top_panel( zones_panel );
    werase(zones_win);
    wnicebox(zones_win, 0, 0, 8, 16, "Zones");

    wattron(zones_win, COLOR_PAIR(C_INPUT) | A_STANDOUT);

    switch (bandinx) {
    case 0:{
	    bandindex = BAND160;
	    break;
	}
    case 1:{
	    bandindex = BAND80;
	    break;
	}
    case 2:{
	    bandindex = BAND40;
	    break;
	}
    case 4:{
	    bandindex = BAND20;
	    break;
	}
    case 6:{
	    bandindex = BAND15;
	    break;
	}
    case 8:
	bandindex = BAND10;
    }


    for (i = 0; i <= 7; i++) {

	for (j = 1; j <= 5; j++) {

	    if ((zones[(i * 5) + j] & bandindex) == 0) {

		mvwprintw(zones_win, i + yloc, (j * 3) + xloc, " %02d", (i * 5) + j);

	    } else {

		mvwprintw(zones_win, i + yloc, (j * 3) + xloc, "   ");

	    }
	}
	wprintw(zones_win, " ");
    }

    return (0);
}
