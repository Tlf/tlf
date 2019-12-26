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
*        View Log using "less" function
*
*--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>

#include "clear_display.h"
#include "ignore_unused.h"
#include "tlf.h"
#include "tlf_curses.h"


int logview(void) {
    extern char logfile[];

    char comstr[40]  = "";
    int j;

    strcat(comstr,  "less  +G ");
    strcat(comstr,  logfile);

    endwin();
    IGNORE(system(comstr));;
    refreshp();

    clear_display();
    attron(COLOR_PAIR(C_LOG)  |  A_STANDOUT);

    for (j = 13 ;  j  <= 23 ; j++) {
	mvprintw(j, 0, backgrnd_str);
    }

    refreshp();

    return (0);
}

