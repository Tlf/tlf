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


#include <string.h>
#include <math.h>

#include "bandmap.h"
#include "clusterinfo.h"
#include "freq_display.h"
#include "get_time.h"
#include "getwwv.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "lancode.h"
#include "printcall.h"
#include "showscore.h"
#include "tlf_curses.h"
#include "trx_memory.h"
#include "ui_utils.h"

/** broadcast to LAN
 *
 * every 120s broadcast frequency via LAN and
 * act as time master if allowed */
void broadcast_lan(void) {

    extern int time_master;
    static int frcounter = 0;

    frcounter++;

    if (frcounter >= 60) {	// every 60 calls
	frcounter = 0;
	if (lan_active != 0) {
	    send_freq(freq);
	    if (time_master == 1)
		send_time();
	}
    }
}


/** update band, date and time */
void update_line(char *timestr) {

    extern struct tm *time_ptr;

    static int daysecs = 0;
    char time_buf[40] = "";

    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
    strncpy(time_buf, timestr, 8);
    mvaddstr(12, 0, band[bandinx]);
    mvprintw(12, 17, time_buf);

    daysecs++;

    if (daysecs >= 60) {		// update the date 1x per minute
	daysecs = 0;
	get_time();
	strftime(time_buf, 60, "%d-%b-%y", time_ptr);
	mvprintw(12, 7, time_buf);
    }
}

const char *FREQ_DISPLAY_FORMAT = " %s: %7.1f";

bool force_show_freq = false;

/** show frequency and memory if rig control is active */
void show_freq(void) {

    extern int trx_control;

    attron(modify_attr(COLOR_PAIR(C_LOG)));

    freq_t memfreq = 0;

    if (trx_control) {
	mvprintw(13, 67, FREQ_DISPLAY_FORMAT, "TRX", freq / 1000.0);
	memfreq = memory_get_freq();
    } else {
	mvprintw(13, 67, spaces(80 - 67));
    }

    if (memfreq > 0) {
	mvprintw(14, 67, FREQ_DISPLAY_FORMAT, "MEM", memfreq / 1000.0);
    } else {
	mvprintw(14, 67, spaces(80 - 67));
    }

}


void time_update(void) {

    extern struct tm *time_ptr;
    extern char qsonrstr[];
    extern int bandinx;
    extern int this_second;
    extern int system_secs;
    extern int miniterm;

    char time_buf[11];
    int currentterm = 0;
    static int s = 0;
    static int m = 0;
    static int bm_timeout = 0;
    static int oldsecs = -1;  	/* trigger immediate update */

    get_time();
    this_second = time_ptr->tm_sec;		/* seconds */
    // used in background_process
    system_secs = time_ptr->tm_min * 60 + time_ptr->tm_sec;

    // force frequency display if it has changed (don't wait until next second)
    static freq_t old_freq = 0;
    if (freq > 0 && fabs(freq - old_freq) >= 100) {
	force_show_freq = true;
	old_freq = freq;
    }

    if (force_show_freq) {
	show_freq();
	clusterinfo();
    }

    if (this_second == oldsecs) {   // still in the same second, no action
	force_show_freq = false;
	return;
    }

    /* do it every second */
    oldsecs = this_second;

    if (wpx == 1) {
	if (minute_timer > 0)
	    minute_timer--;
    }

    if (!force_show_freq) {     // do not show again if it was forced
	show_freq();
    }
    force_show_freq = false;

    bandmap_age();		/* age bandmap spots every second */
    clusterinfo();		/* update cluster and bandmap display */

    /* write bandmap spots to file every 10s */
    bm_timeout = (bm_timeout + 1) % 10;
    if (bm_timeout == 0) {

	bmdata_write_file();
    }

    s = (s + 1) % 2;
    if (s > 0) {		/* every 2 seconds */

	strftime(time_buf, 10, "%H:%M:%S", time_ptr);
	time_buf[5] = '\0';

	if ((time_buf[6] == '1') && (m >= 30)) {

	    m = 0;
	    getwwv();
	    printcall();

	} else {
	    m++;
	}

	currentterm = miniterm;
	miniterm = 0;

	broadcast_lan();
	update_line(time_buf);

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	mvprintw(7, 0, logline0);
	mvprintw(8, 0, logline1);
	mvprintw(9, 0, logline2);
	mvprintw(10, 0, logline3);
	mvprintw(11, 0, logline4);
	mvprintw(13, 0, spaces(67));
	attron(COLOR_PAIR(C_WINDOW));
	mvprintw(12, 23, qsonrstr);
	printcall();

	showscore();	/* update  score  window every 2 seconds */
	miniterm = currentterm;
    }
}
