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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------
 *     last 10 - return time (in mins) for last 10 QSOs on
 *		 actual band
 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>

#include "get_time.h"
#include "globalvars.h"		// Includes glib.h and tlf.h

int last10(void) {

    int minsbefore;
    int minsnow;
    int span;
    int index;
    int qsocount = 0;
    int thisband;
    struct qso_t *qso;

    if (nr_qsos < 10)
	return (-1);

    thisband = atoi(band[bandinx]);

    /* look backwards in actual band for QSOs */
    for (index = nr_qsos - 1; index >= 0; index--) {

	qso = g_ptr_array_index(qso_array, index);
	if (thisband == qso->band) {
	    qsocount++;
	    if (qsocount >= 10)		/* stop after 10 QSOs found */
		break;
	}
    }

    /* index points to the first QSO */
    if (index < 0)
	return (-1);			/* not 10 QSOs found */

    minsbefore = qso->hour *60 + qso->min;

    minsnow = get_minutes();

    if ((minsnow - minsbefore) <= 0)
	minsnow += 1440;

    span = minsnow - minsbefore;

    return span;
}
