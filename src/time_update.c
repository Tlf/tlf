/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011-2012 Thomas Beierlein <tb@forth-ev.de>
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
	 *
	 *              Update time
	 *
	 *              also updates windows every second
	 *--------------------------------------------------------------*/
#include "globalvars.h"
#include "time_update.h"

void time_update(void)
{
    extern struct tm *time_ptr;
    extern char qsonrstr[];
    extern int bandinx;
    extern int this_second;
    extern long system_secs;
    extern int miniterm;

    char time_buf[11];
    int currentterm = 0;
    static int s = 0;
    static int m = 0;
    static int oldsecs = -1;  	/* trigger immediate update */

    get_time();
    this_second = time_ptr->tm_sec;		/* seconds */
    system_secs = time_ptr->tm_min * 60 + time_ptr->tm_sec;

    if (this_second != oldsecs) {
	/* do it every second */
	oldsecs = this_second;

	if (wpx == 1) {
	    if (minute_timer > 0)
		minute_timer--;
	}

	s = (s + 1) % 2;
	if (s > 0) {		/* every 2 seconds */

	    strftime(time_buf, 10, "%H:%M:%S", time_ptr);
	    time_buf[5] = '\0';

	    if ((time_buf[6] == '1') && (m >= 30)) {

		m = 0;
		getwwv();

	    } else {
		m++;
	    }

	    currentterm = miniterm;
	    miniterm = 0;

	    clusterinfo(time_buf);	/* update cluster info (2 seconds) */

	    attron(COLOR_PAIR(7) | A_STANDOUT);

	    mvprintw(7, 0, logline0);
	    mvprintw(8, 0, logline1);
	    mvprintw(9, 0, logline2);
	    mvprintw(10, 0, logline3);
	    mvprintw(11, 0, logline4);
	    mvprintw(13, 0, 
		    "                                                                    ");
	    attron(COLOR_PAIR(COLOR_CYAN));
	    mvprintw(12, 23, qsonrstr);
	    printcall();

	    showscore();	/* update  score  window every 2 seconds */
	    show_zones(bandinx);
	    miniterm = currentterm;
	}
    }
}
