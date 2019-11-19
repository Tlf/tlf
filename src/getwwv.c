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


#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dxcc.h"
#include "printcall.h"
#include "tlf.h"
#include "tlf_curses.h"

extern int ymax;

#define LINELENGTH 80

int getwwv(void) {

    extern char lastwwv[];
    extern const char backgrnd_str[];
    extern double r;
    extern int mycountrynr;
    extern int timeoffset;

    char printbuffer[81] = "";
    char *i;
    char r_value[6];
    char sf_value[6];
    char timebuff[80];
    double sfi, d;

    time_t now;
    struct tm *ptr1;

    if (strlen(lastwwv) >= 2) {

	lastwwv[78] = '\0';	/* cut the bell chars */

	if ((strncmp(lastwwv, "WCY", 3) == 0)
		|| (strncmp(lastwwv, "WWV", 3) == 0)) {

	    strcat(printbuffer, "Condx: ");

	    i = strstr(lastwwv, "<");

	    if (i != NULL) {
		strncat(printbuffer, i + 1, 2);
		strcat(printbuffer, " GMT              ");
	    }

	    i = strstr(lastwwv, "R=");
	    if (i != NULL) {
		strncat(printbuffer, i, 5);
		r_value[0] = '\0';
		strncat(r_value, i + 2, 3);
		r = atof(r_value);
	    }
	    strcat(printbuffer, "    ");

	    i = strstr(lastwwv, "SFI=");
	    if (i != NULL) {
		strncat(printbuffer, i, 7);

		sf_value[0] = '\0';
		strncat(sf_value, i + 4, 3);
		sfi = atof(sf_value);
		r = ((sfi - 70.0) * (200.0 / 180.0));
	    }

	    i = strstr(lastwwv, "eru");
	    if (i != NULL)
		strcat(printbuffer, "     eruptive  ");

	    i = strstr(lastwwv, "act");
	    if (i != NULL)
		strcat(printbuffer, " act  ");

	    i = strstr(lastwwv, "Au=au");
	    if (i != NULL)
		strcat(printbuffer, "   AURORA!");

	    strcpy(lastwwv, printbuffer);


	    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
	    mvprintw(LINES - 1, 0, backgrnd_str);

	    mvprintw(LINES - 1, 0, printbuffer);	/* print WWV info  */
	    printw(" ");

	    d = dxcc_by_index(mycountrynr) -> timezone;

	    now = (time(0) + (long)((timeoffset - d) * 3600));
	    ptr1 = gmtime(&now);
	    strftime(timebuff, 80, "%H:%M", ptr1);

	    mvprintw(LINES - 1, LINELENGTH - 17, " local time %s", timebuff);

	    refreshp();
	}
    }
    return 0;
}
