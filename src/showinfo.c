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


#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dxcc.h"
#include "qrb.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"


int showinfo(int x)
{
    extern char cqzone[];
    extern char ituzone[];
    extern double DEST_Lat;
    extern double DEST_Long;
    extern int timeoffset;
    extern long timecorr;
    extern char itustr[];
    extern int mycountrynr;

    int cury, curx;
    char pxstr[16];
    char countrystr[26];
    char zonestr[3];
    char contstr[3] = "";
    double bearing;
    double range;

    char timebuff[80];

    dxcc_data *dx;
    double d;
    time_t now;
    struct tm *ptr1;

    dx = dxcc_by_index(x);

    strcpy(pxstr, dx->pfx);
    strcpy(countrystr, dx->countryname);	/* country */

    if (strlen(cqzone) < 2) {
	sprintf(zonestr, "%02d", dx->cq); 	/* cqzone */
	strcpy(cqzone, zonestr);
    } else {
	strncpy(zonestr, cqzone, 2);
	zonestr[2] = '\0';
    }

    if (strlen(ituzone) < 2) {
	sprintf(itustr, "%02d", dx->itu);	/* itu zone */
    } else {
	strncpy(itustr, ituzone, 2);
	itustr[2] = '\0';
    }

    d = dx->timezone;				/* GMT difference */

    now = (time(0) + (long)((timeoffset - d) * 3600) + timecorr);
    ptr1 = gmtime(&now);
    strftime(timebuff, 80, "%H:%M", ptr1);

    DEST_Lat = dx->lat;				/* where is he? */
    DEST_Long = dx->lon;

    strncpy(contstr, dx->continent, 2);	/* continent */
    contstr[2] = '\0';

    getyx(stdscr, cury, curx);
    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

    mvprintw(24, 0, " %s  %s             ", pxstr, countrystr);

    mvprintw(24, 26,
	 " %s %s                                           ",
	 contstr, zonestr);

    if (x != 0 && x != mycountrynr) {
	qrb_(&range, &bearing);
	mvprintw(24, 35, "%.0f km/%.0f deg ", range, bearing);
    }

    mvprintw(24, 64, "  DX time: %s", timebuff);

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvprintw(cury, curx, "");

    return (0);
}
