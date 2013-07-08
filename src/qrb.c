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

#include "qrb.h"
/* Compute the Bearing and Range */

int qrb(void)
{

    extern char C_QTH_Lat[];
    extern char C_QTH_Long[];
    extern char C_DEST_Lat[];
    extern char C_DEST_Long[];
    extern char hiscall[];

    extern double range;
    extern double bearing;

    double tmp;
    double dist;
    double Delta_Long;
    double QTH_Lat;
    double QTH_Long;
    double DEST_Lat;
    double DEST_Long;

    if (strlen(hiscall) < 1)
	return (0);

    QTH_Lat = atof(C_QTH_Lat);
    QTH_Long = atof(C_QTH_Long);
    DEST_Lat = atof(C_DEST_Lat);
    DEST_Long = atof(C_DEST_Long);

    Delta_Long = DEST_Long - QTH_Long;

    QTH_Lat /= RADIAN;		/* Convert variables to Radians */
    QTH_Long /= RADIAN;
    DEST_Lat /= RADIAN;
    DEST_Long /= RADIAN;
    Delta_Long /= RADIAN;

    tmp =
	(sin(QTH_Lat) * sin(DEST_Lat)) +
	(cos(QTH_Lat) * cos(DEST_Lat) * cos(Delta_Long));

    dist = acos(tmp);
    range = 60.0 * (dist * RADIAN) * 1.8;

    tmp = (sin(DEST_Lat) - (sin(QTH_Lat) * cos(dist))) /
	(sin(dist) * cos(QTH_Lat));

    bearing = acos(tmp) * RADIAN;

    if (Delta_Long > 0.0)
	bearing = 360.0 - bearing;

    return (0);
}
