/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 		 2013           Thomas Beierlein <tb@forth-ev.de
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
 *       Clear_display takes  care of status lines  and
 *       backgrounds (general repaint of the screen)
 *--------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>

#include "cqww_simulator.h"
#include "cw_utils.h"
#include "change_rst.h"
#include "get_time.h"
#include "getwwv.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "muf.h"
#include "printcall.h"
#include "qsonr_to_str.h"
#include "searchlog.h"		// Includes glib.h
#include "setcontest.h"
#include "showscore.h"
#include "time_update.h"
#include "tlf_curses.h"
#include "ui_utils.h"


void show_header_line() {
    char *mode = "";
    switch (cqmode) {
	case CQ:
	    mode = (simulator ? "Sim" : "Log");
	    break;
	case S_P:
	    mode = "S&P";
	    break;
	case AUTO_CQ:
	    mode = "AUTO_CQ";
	    break;
	case KEYBOARD:
	    mode = "Keyboard";
	    break;
	default:
	    ;   // should not happen
    }

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvaddstr(0, 0, spaces(29));
    mvprintw(0, 0, "  %-8s  S=%2i D=%i ", mode, GetCWSpeed(), cqdelay);
    mvaddstr(0, 21, headerline);
}


void clear_display(void) {
    extern char terminal1[];
    extern char terminal2[];
    extern char terminal3[];
    extern char terminal4[];

    int cury, curx;

    getyx(stdscr, cury, curx);

    show_header_line();

    attron(modify_attr(COLOR_PAIR(C_LOG) | A_STANDOUT));
    mvaddstr(1, 0, terminal1);
    mvaddstr(2, 0, terminal2);
    mvaddstr(3, 0, terminal3);
    mvaddstr(4, 0, terminal4);
    mvaddstr(5, 0, backgrnd_str);

    attron(COLOR_PAIR(C_HEADER));
    mvaddstr(6, 0, backgrnd_str);
    mvprintw(6, (80 - strlen(whichcontest)) / 2 - 4, " == %s == ", whichcontest);

    showscore();

    attron(modify_attr(COLOR_PAIR(C_LOG) | A_STANDOUT));
    mvaddstr(7, 0, logline0);
    mvaddstr(8, 0, logline1);
    mvaddstr(9, 0, logline2);
    mvaddstr(10, 0, logline3);
    mvaddstr(11, 0, logline4);

    attron(COLOR_PAIR(C_WINDOW));
    mvaddstr(12, 0, backgrnd_str);

    mvaddstr(12, 0, band[bandinx]);

    get_time();

    if (trxmode == CWMODE)
	mvaddstr(12, 3, "CW");
    else if (trxmode == SSBMODE)
	mvaddstr(12, 3, "SSB");
    else
	mvaddstr(12, 3, "DIG");

    char time_buf[20];
    format_time(time_buf, sizeof(time_buf), DATE_TIME_FORMAT);
    update_line(time_buf);

    qsonr_to_str();
    mvaddstr(12, 23, qsonrstr);

    if (no_rst) {
	mvaddstr(12, 44, "   ");
	mvaddstr(12, 49, "   ");
    } else {
	rst_set_strings();
	mvaddstr(12, 44, sent_rst);
	mvaddstr(12, 49, recvd_rst);
    }

    if (CONTEST_IS(CQWW)) {
	attron(modify_attr(COLOR_PAIR(NORMCOLOR)));
	mvaddstr(12, 54, comment);
    }

    if (CONTEST_IS(ARRLDX_USA)) {
	attron(modify_attr(COLOR_PAIR(NORMCOLOR)));
	mvaddstr(12, 54, comment);
    }

    if (searchflg == SEARCHWINDOW)
	searchlog();

    printcall();

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvaddstr(LINES - 1, 0, backgrnd_str);
    wwv_show_footer();

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));
    move(cury, curx);
    refreshp();
}
