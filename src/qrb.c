/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *               2014           Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "globalvars.h"
#include "qrb.h"


/* Compute the Bearing and Range */

int get_qrb(double *range, double *bearing) {

    extern double DEST_Lat;
    extern double DEST_Long;

    if (*current_qso.call == '\0')
	return -1;

    /* positive numbers are N and E
     *
     * be aware that dxcc counts east longitudes as negative numbers
     */
    return qrb(-1.0 * my.Long, my.Lat, -1.0 * DEST_Long, DEST_Lat,
	       range, bearing);
}

bool get_qrb_for_locator(const char *locator, double *range, double *bearing) {
    double dest_long, dest_lat;

    if (RIG_OK != locator2longlat(&dest_long, &dest_lat, locator)) {
	return false;   // invalid locator
    }

    return RIG_OK == qrb(my.Long, my.Lat, -dest_long, dest_lat,
			 range, bearing);
}
