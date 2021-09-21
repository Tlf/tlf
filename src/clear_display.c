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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------
 *       Clear_display takes  care of status lines  and
 *       backgrounds (general repaint of the screen)
 *--------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "cqww_simulator.h"
#include "cw_utils.h"
#include "change_rst.h"
#include "get_time.h"
#include "getctydata.h"
#include "getwwv.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "logit.h"
#include "muf.h"
#include "printcall.h"
#include "qsonr_to_str.h"
#include "searchlog.h"		// Includes glib.h
#include "setcontest.h"
#include "showinfo.h"
#include "showscore.h"
#include "time_update.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"

static char terminal1[88] = "";
static char terminal2[88] = "";
static char terminal3[88] = "";
static char terminal4[88] = "";


void init_terminal_strings(void) {
    strcat(terminal1, backgrnd_str);
    strcat(terminal2, backgrnd_str);
    strcat(terminal3, backgrnd_str);
    strcat(terminal4, backgrnd_str);
}


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
    mvaddstr(0, 21, fkey_header);
}


void clear_display(void) {
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

    if (searchflg == SEARCHWINDOW)
	searchlog();

    printcall();
    refresh_comment();

    update_info_line();

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));
    move(cury, curx);
    refreshp();
}

/*
 *  scroll the loglines of the keyer terminal and show them
 */
void displayit(void) {
    extern char termbuf[];

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
