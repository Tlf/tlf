/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 		 2011 Thomas Beierlein <tb@forth-ev.de>
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
/* ------------------------------------------------------------
 *      search array of worked stations
 *
 *--------------------------------------------------------------*/

#include <stdbool.h>
#include <string.h>

#include "bands.h"
#include "get_time.h"
#include "globalvars.h"
#include "tlf.h"


/**	\brief lookup 'hiscall' in array of worked stations
 *
 * 	See if 'hiscall' was already worked by looking it up in worked[]
 * 	\param hiscall 	callsign to lookup
 *      \return index in callarray where hiscall was found (-1 if not found)
 */
int searchcallarray(char *hiscall) {

    int found = -1;
    int i;

    for (i = 0; i < nr_worked; i++) {

	if (strcmp(worked[i].call, hiscall) == 0) {
	    found = i;
	    break;
	}

    }

    return (found);
}

//
// check if station was worked in the current minitest period
// it takes into account actual mode/band info
//
bool worked_in_current_minitest_period(int found) {

    if (found < 0) {
	return false;
    }
    if (!minitest) {
	return true;    // minitest is off, so the answer is yes
    }

    long currtime = get_time();
    long period_start = (currtime / minitest) * minitest;
    return worked[found].qsotime[trxmode][bandinx] >= period_start;
}


bool is_dupe(char *call, int bandindex, int mode) {

    int index;

    index = searchcallarray(call);
    if (index == -1)	/* new station */
	return false;

    if (!qso_once	/* check band only if qso_once not set */
	    && ((worked[index].band & inxes[bandindex]) == 0))
	return false;

    if (mixedmode	/* check mode only if MIXED is allowed */
	    && (worked[index].qsotime[mode][bandindex] == 0))
	return false;

    if (!worked_in_current_minitest_period(index))
	return false;

    return true;

}


