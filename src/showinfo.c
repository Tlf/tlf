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
	 *
	 *              Make info string for lower status line
	 *		x - countrynumber
	 *--------------------------------------------------------------*/

#include "showinfo.h"
#include "dxcc.h"

int showinfo(int x)
{

    extern int use_rxvt;
    extern char cqzone[];
    extern char ituzone[];
    extern int cury, curx;
    extern char C_DEST_Lat[];
    extern char C_DEST_Long[];
    extern double bearing;
    extern double range;
    extern int timeoffset;
    extern int timecorr;
    extern char itustr[];
    extern int countrynr;
    extern int mycountrynr;

    char bufstr[81];
    char pxstr[16];
    char countrystr[26];
    char zonestr[3];
    char contstr[3] = "";
    char timebuff[80];

    dxcc_data *dx;
    double d;
    time_t now;
    struct tm *ptr1;

    bufstr[80] = '\0';
    bufstr[0] = ' ';

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

    now = (time(0) + ((timeoffset - d) * 3600) + timecorr);
    ptr1 = gmtime(&now);
    strftime(timebuff, 80, "%H:%M", ptr1);

    sprintf(C_DEST_Lat, "%6.2f", dx->lat);	/* where is he? */
    sprintf(C_DEST_Long, "%7.2f", dx->lon);

    if (countrynr != 0 && countrynr != mycountrynr)
	qrb();

    strncpy(contstr, dx->continent, 2);	/* continent */
    contstr[2] = '\0';

    getyx(stdscr, cury, curx);
    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);

    // x == 1 for 1A..., should be 'x==0'
    if (x == 1)
	mvprintw(24, 0,
		 "                                                                                        ");
    else {
	mvprintw(24, 0, " %s  %s             ", pxstr, countrystr);

	// check fo x are overlapping, output is redundant
	if (x > 0) {
	    mvprintw(24, 26,
		     " %s %s                                           ",
		     contstr, zonestr);

	    if (x != 0 && x != mycountrynr)
		mvprintw(24, 26, " %s %s   %.0f km/%.0f deg ", contstr,
			 zonestr, range, bearing);
	    else
		mvprintw(24, 26, " %s %s                            ",
			 contstr, zonestr);

	} else
	    mvprintw(24, 26, " %s %s                        ", contstr,
		     zonestr);

	mvprintw(24, 64, "  DX time: %s", timebuff);
    }
    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    mvprintw(cury, curx, "");

    return (0);

}
