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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * qrb() function comes from HAMLIB src/locator.c
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


#include <math.h>

#include "qrb.h"


/* Compute the Bearing and Range */

int get_qrb(double *range, double *bearing) {

    extern double QTH_Lat;
    extern double QTH_Long;
    extern double DEST_Lat;
    extern double DEST_Long;
    extern char hiscall[];

    if (*hiscall == '\0')
	return -1;

    return qrb(-1.0 * QTH_Long, QTH_Lat, -1.0 * DEST_Long, DEST_Lat,
	       range, bearing);
}

/* positive numbers are N and E
 *
 * be aware that dxcc counts east longitudes as negative numbers
 */
int qrb(double lon1, double lat1, double lon2, double lat2, double *distance,
	double *azimuth) {
    double delta_long, tmp, arc, az;

    /* bail if NULL pointers passed */
    if (!distance || !azimuth)
	return -1;

    if ((lat1 > 90.0 || lat1 < -90.0) || (lat2 > 90.0 || lat2 < -90.0))
	return -1;

    if ((lon1 > 180.0 || lon1 < -180.0) || (lon2 > 180.0 || lon2 < -180.0))
	return -1;

    /* Prevent ACOS() Domain Error */
    if (lat1 == 90.0)
	lat1 = 89.999999999;
    else if (lat1 == -90.0)
	lat1 = -89.999999999;

    if (lat2 == 90.0)
	lat2 = 89.999999999;
    else if (lat2 == -90.0)
	lat2 = -89.999999999;

    /* Convert variables to Radians */
    lat1	/= RADIAN;
    lon1	/= RADIAN;
    lat2	/= RADIAN;
    lon2	/= RADIAN;

    delta_long = lon2 - lon1;

    tmp = sin(lat1) * sin(lat2) +
	  cos(lat1) * cos(lat2) * cos(delta_long);

    if (tmp > .999999999999999) {
	/* Station points coincide, use an Omni! */
	*distance = 0.0;
	*azimuth = 0.0;
	return 0;
    }

    if (tmp < -.999999) {
	/*
	 * points are antipodal, it's straight down.
	 * Station is equal distance in all Azimuths.
	 * So take 180 Degrees of arc times 60 nm,
	 * and you get 10800 nm, or whatever units...
	 */
	*distance = 180.0 * ARC_IN_KM;
	*azimuth = 0.0;
	return 0;
    }

    arc = acos(tmp);

    /*
     * One degree of arc is 60 Nautical miles
     * at the surface of the earth, 111.2 km, or 69.1 sm
     * This method is easier than the one in the handbook
     */


    *distance = ARC_IN_KM * RADIAN * arc;

    /* Short Path */
    /* Change to azimuth computation by Dave Freese, W1HKJ */

    az = RADIAN * atan2(sin(lon2 - lon1) * cos(lat2),
			(cos(lat1) * sin(lat2) -
			 sin(lat1) * cos(lat2) * cos(lon2 - lon1)));

    az = fmod(360.0 + az, 360.0);
    if (az < 0.0)
	az += 360.0;
    else if (az >= 360.0)
	az -= 360.0;

    *azimuth = floor(az + 0.5);
    return 0;
}
