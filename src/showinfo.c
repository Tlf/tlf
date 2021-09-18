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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------
 *
 *              Make info string for lower status line
 *		x - countrynumber
 *--------------------------------------------------------------*/

/** Show infos for selected country on bottom of screen
 *
 * Prepares info string for the selected country and shows it on the
 * bottom line of the screen.
 *
 * /param x  Country number
 */


#include "dxcc.h"
#include "getwwv.h"
#include "get_time.h"
#include "globalvars.h"
#include "qrb.h"
#include "showinfo.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"

#define LINELENGTH 80

void showinfo(int x) {
    int cury, curx;
    double bearing;
    double range;

    char timebuff[80];

    prefix_data *pfx = prefix_by_index(x);
    dxcc_data *dx = dxcc_by_index(pfx -> dxcc_index);

    getyx(stdscr, cury, curx);
    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

    mvaddstr(LINES - 1, 0, backgrnd_str);

    if (pfx->dxcc_index > 0) {
	mvprintw(LINES - 1, 0, " %s  %s", dx->pfx, dx->countryname);

	mvprintw(LINES - 1, 26, " %s %02d", pfx->continent, pfx->cq);

	if (x != 0 && x != my.countrynr && 0 == get_qrb(&range, &bearing)) {
	    mvprintw(LINES - 1, 35, "%.0f km/%.0f deg ", range, bearing);
	}

	format_time_with_offset(timebuff, sizeof(timebuff), TIME_FORMAT, pfx->timezone);
	mvprintw(LINES - 1, LINELENGTH - 17, "   DX time: %s", timebuff);
    } else {
	wwv_show_footer();
    }

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));
    move(cury, curx);
}
