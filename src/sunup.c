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
#include "sunup.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <curses.h>

/* Compute sun up and down at destination  */

int sunup (void) {


extern char C_DEST_Lat[];
extern struct tm *time_ptr;
extern double sunrise;
extern double sundown;

double  DEST_Lat;
char date_buf[20];

char c_day[3];
int day;
char c_month[3];
int month;
double total_days;
double sunshine;

        DEST_Lat = atof(C_DEST_Lat);

        DEST_Lat   /= RADIAN;

        get_time();
        strftime(date_buf, 5, "%d%m", time_ptr);

        strncpy(c_day, date_buf, 2);
        day = atoi(c_day);
        strncpy(c_month, date_buf+2, 2);
        month = atoi(c_month);
        total_days = (month - 1) * 30.4 + day + 10;

        if (total_days >= 365.25)
        	total_days -= 365.25;
        if (total_days <= 0.0)
        	total_days += 365.25;
        	
         sunshine = (24.0/180.0) * RADIAN * acos(cos(((360.0  * 32.0) / 365.25)/RADIAN)* tan(DEST_Lat) * tan(23/RADIAN));

         sunrise = 12.0 - sunshine / 2;
         sundown = 12.0 + sunshine / 2;

        return(0);
}


