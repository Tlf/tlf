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
 *		 in case of not enough QSOs -1 is returned
 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>

#include "get_time.h"
#include "globalvars.h"		// Includes glib.h and tlf.h

int last10(void) {

    int index;
    int qsocount = 0;
    int thisband;
    struct qso_t *qso;

    if (NR_QSOS < 10)
	return -1;

    thisband = atoi(band[bandinx]);

    /* look backwards in actual band for QSOs */
    for (index = NR_QSOS - 1; index >= 0; index--) {

	qso = g_ptr_array_index(qso_array, index);
	if (thisband == qso->band) {
	    qsocount++;
	    if (qsocount >= 10)		/* stop after 10 QSOs found */
		break;
	}
    }

    /* index points to the 10th QSO */
    if (index < 0)
	return -1;			/* not 10 QSOs found */

    return (get_time() - qso->timestamp) / 60;  /* elapsed time in minutes */
}
