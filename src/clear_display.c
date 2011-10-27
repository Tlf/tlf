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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	/* ------------------------------------------------------------
	 *       Clear_display takes  care of status lines  and
	 *       backgrounds (general repaint of the screen)
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "clear_display.h"

void clear_display(void)
{
    extern int use_rxvt;
    extern char speedstr[];
    extern int speed;
    extern char mode[];
    extern int cqdelay;
    extern char headerline[];
    extern char terminal1[];
    extern char terminal2[];
    extern char terminal3[];
    extern char terminal4[];
    extern char backgrnd_str[];
    extern char band[9][4];
    extern int bandinx;
    extern int trxmode;
    extern char my_rst[];
    extern char his_rst[];
    extern char qsonrstr[];
    extern int cqww;
    extern int arrldx_usa;
    extern char comment[];
    extern char hiscall[];
    extern int searchflg;
    extern int m;
    extern struct tm *time_ptr;

    char time_buf[80];
    char speedbuf[4] = "  ";
    int cury, curx;

    strncpy(speedbuf, speedstr + (2 * speed), 2);
    speedbuf[2] = '\0';
    getyx(stdscr, cury, curx);

    mvprintw(0, 0, "");
    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
    mvprintw(0, 0, "                             ");
    mvprintw(0, 0, "  %s  S=%s D=%i ", mode, speedbuf, cqdelay);
    mvprintw(0, 21, headerline);

    if (use_rxvt == 0)
	attron(COLOR_PAIR(COLOR_WHITE | A_BOLD | A_STANDOUT));
    else
	attron(COLOR_PAIR(COLOR_WHITE | A_STANDOUT));

    mvaddstr(1, 0, terminal1);
    mvaddstr(2, 0, terminal2);
    mvaddstr(3, 0, terminal3);
    mvaddstr(4, 0, terminal4);
    mvaddstr(5, 0, backgrnd_str);
    mvprintw(6, 0, "");
    mvaddstr(6, 0, "");
    attron(COLOR_PAIR(COLOR_GREEN));
//    hline(ACS_HLINE, 80);
    mvaddstr(6, 0, backgrnd_str);

    showscore();

    if (use_rxvt == 0)
	attron(COLOR_PAIR(COLOR_WHITE) | A_BOLD | A_STANDOUT);
    else
	attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);

    mvaddstr(7, 0, logline0);
    mvaddstr(8, 0, logline1);
    mvaddstr(9, 0, logline2);
    mvaddstr(10, 0, logline3);
    mvaddstr(11, 0, logline4);
    attron(COLOR_PAIR(COLOR_CYAN));
    mvaddstr(12, 0, backgrnd_str);
    mvaddstr(12, 0, band[bandinx]);

    get_time();

    if (trxmode == CWMODE)
	strftime(time_buf, 60, "CW  %d-%b-%y %H:%M ", time_ptr);
    else if (trxmode == SSBMODE)
	strftime(time_buf, 60, "SSB %d-%b-%y %H:%M ", time_ptr);
    else
	strftime(time_buf, 60, "DIG %d-%b-%y %H:%M ", time_ptr);

    m = time_ptr->tm_mon;	/* month for muf calc */

    mvprintw(12, 3, time_buf);

    qsonr_to_str();
    mvaddstr(12, 23, qsonrstr);

    if (trxmode != SSBMODE) {

	my_rst[2] = '9';
	his_rst[2] = '9';
    } else {
	my_rst[2] = ' ';
	his_rst[2] = ' ';

    }

    mvaddstr(12, 44, his_rst);
    mvaddstr(12, 49, my_rst);

    if (cqww == 1) {
	if (use_rxvt == 0)
	    attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
	else
	    attron(COLOR_PAIR(NORMCOLOR));
	mvaddstr(12, 54, comment);
    }

    if (arrldx_usa == 1) {
	if (use_rxvt == 0)
	    attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
	else
	    attron(COLOR_PAIR(NORMCOLOR));
	mvaddstr(12, 54, comment);
    }

    if (searchflg == SEARCHWINDOW)
	searchlog(hiscall);

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    mvaddstr(12, 29, hiscall);

    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
    mvprintw(24, 0, backgrnd_str);

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    mvprintw(cury, curx, "");
    refreshp();
}
