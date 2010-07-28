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
	 *     last 10
	 *
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "last10.h"

int last10(void)
{

    char input[85];
    char time10[6] = "";
    char time_buf[20];

    int minsbefore;
    int minsnow;
    int span;
    int counter;
    int qsocount = 0;
    int thisband;

    if (nr_qsos < 10)
	return (-1);

    thisband = atoi(band[bandinx]);

    for (counter = nr_qsos; counter >= 0; counter--) {

	if (thisband == (atoi(qsos[counter]))) {
	    qsocount++;
	    if (qsocount >= 10)
		break;
	}
    }

    if (counter > 0)
	strncpy(input, qsos[counter], 85);

    strncpy(time10, input + 17, 5);
    time10[5] = '\0';
    minsbefore = atoi(time10 + 3);
    time10[2] = '\0';
    minsbefore += (atoi(time10) * 60);

    get_time();
    strftime(time_buf, 10, "%H:%M", time_ptr);
/*				strncpy (timeptr2, time_buf, 6);
				current_hour = atoi (time_buf);
				current_hour += timeoffset;
				if (current_hour < 0) current_hour += 24;
				if (current_hour > 23) current_hour -= 24;
				sprintf(time_buf , "00");
				sprintf(time_buf , "%2d:", current_hour);
				sprintf(time_buf + 3, "%s", timeptr2 + 3);
	*/

    minsnow = atoi(time_buf + 3);
    time_buf[2] = '\0';
    minsnow += (atoi(time_buf) * 60);

    if ((minsnow - minsbefore) <= 0)
	minsnow += 1440;
    span = minsnow - minsbefore;

    return (span);
}
