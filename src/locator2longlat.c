/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *
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

/*
 * original code in HAMLIB
 * src/locator.c
 *
 * author Stephane Fillod and the Hamlib Group
 * date 2000-2010
 *
 *  Hamlib Interface - locator, bearing, and conversion calls
 *
 *
 *  Hamlib Interface - locator and bearing conversion calls
 *  Copyright (c) 2001-2010 by Stephane Fillod
 *  Copyright (c) 2003 by Nate Bargmann
 *  Copyright (c) 2003 by Dave Hines
 *
 *
 *  Code to determine bearing and range was taken from the Great Circle,
 *  by S. R. Sampson, N5OWK.
 *  Ref: "Air Navigation", Air Force Manual 51-40, 1 February 1987
 *  Ref: "ARRL Satellite Experimenters Handbook", August 1990
 *
 *  Code to calculate distance and azimuth between two Maidenhead locators,
 *  taken from wwl, by IK0ZSN Mirko Caserta.
 *
 *  New bearing code added by N0NB was found at:
 *  http://williams.best.vwh.net/avform.htm#Crs
 *
 */


#include <ctype.h>
#include <string.h>

#define MAX_LOCATOR_PAIRS       6
#define MIN_LOCATOR_PAIRS       1


static const int loc_char_range[] = { 18, 10, 24, 10, 24, 10 };

/**
 * converts QTH locator to lattitude and longitude
 *
 * Be aware that it returns east longitudes as negative numbers, wheras dxcc cty table
 * uses negative numbers
 */
int locator2longlat(double *longitude, double *latitude, const char *locator) {
    int x_or_y, paircount;
    int locvalue, pair;
    int divisions;
    double xy[2], ordinate;

    /* bail if NULL pointers passed */
    if (!longitude || !latitude)
	return -1;

    paircount = strlen(locator) / 2;

    /* verify paircount is within limits */
    if (paircount > MAX_LOCATOR_PAIRS)
	paircount = MAX_LOCATOR_PAIRS;
    else if (paircount < MIN_LOCATOR_PAIRS)
	return -1;

    /* For x(=longitude) and y(=latitude) */
    for (x_or_y = 0;  x_or_y < 2;  ++x_or_y) {
	ordinate = -90.0;
	divisions = 1;

	for (pair = 0;  pair < paircount;  ++pair) {
	    locvalue = locator[pair * 2 + x_or_y];

	    /* Value of digit or letter */
	    locvalue -= (loc_char_range[pair] == 10) ? '0' :
			(isupper(locvalue)) ? 'A' : 'a';

	    /* Check range for non-letter/digit or out of range */
	    if ((locvalue < 0) || (locvalue >= loc_char_range[pair]))
		return -1;

	    divisions *= loc_char_range[pair];
	    ordinate += locvalue * 180.0 / divisions;
	}
	/* Center ordinate in the Maidenhead "square" or "subsquare" */
	ordinate += 90.0 / divisions;

	xy[x_or_y] = ordinate;
    }

    *longitude = xy[0] * 2.0;
    *latitude = xy[1];

    return 0;
}

/* returns true if 'qra' is valid QRA locator
 */
int check_qra(char *qra) {
    if (strlen(qra) < 4) {
	return 0;
    }
    if (qra[0] < 'A' || qra[0] > 'R' ||	qra[1] < 'A' || qra[1] > 'R' ||
	    qra[2] < '0' || qra[2] > '9' ||	qra[3] < '0' || qra[3] > '9') {
	return 0;
    }
    return 1;
}
