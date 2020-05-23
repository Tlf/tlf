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


#include <math.h>
#include <time.h>

#include "get_time.h"

#define RADIAN  (180.0 / M_PI)


/** Compute sun up and down at given lattitude
 *
 * \param lat - Lattitude
 * \param sunrise - local sunrise in hours
 * \param sundown - local sundown in hours
 */

void sunup(double DEST_Lat, double *sunrise, double *sundown) {


    double lat;
    double sun_lat;
    double total_days;
    double sunshine;

    lat = DEST_Lat / RADIAN;

    time_t now = get_time();
    struct tm time_tm;
    gmtime_r(&now, &time_tm);
    total_days = time_tm.tm_yday + 10; /* days after lower culmination
						   of the sun */
    if (total_days >= 365.25)
	total_days -= 365.25;
    if (total_days <= 0.0)
	total_days += 365.25;

    /* calculate todays lattitude of the sun */
    sun_lat = asin(sin(23.439 / RADIAN) *
		   sin(((total_days - 90.086) / 365.25) * 360 / RADIAN)) * RADIAN;

    /* sunshine period today at given lat */
    sunshine =
	(24.0 / 180.0) * RADIAN *
	acos(-tan(lat) * tan(sun_lat / RADIAN));

    *sunrise = 12.0 - sunshine / 2;
    *sundown = 12.0 + sunshine / 2;
}
