/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
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
 *      add call/band to dupe list
 *
 *--------------------------------------------------------------*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "addcall.h"
#include "addmult.h"
#include "addpfx.h"
#include "bands.h"
#include "dxcc.h"
#include "getctydata.h"
#include "getpx.h"
#include "get_time.h"
#include "globalvars.h"
#include "log_utils.h"
#include "paccdx.h"
#include "score.h"
#include "searchcallarray.h"
#include "setcontest.h"
#include "tlf.h"
#include "zone_nr.h"

int excl_add_veto;
/* This variable helps to handle in other modules, that station is multiplier
 * or not */
/* In addcall2(), this variable helps to handle the excluded multipliers,
 * which came from lan_logline the Tlf scoring logic is totally completely
 * different in local and LAN source the addcall() function doesn't increment
 * the band_score[] array, that maintains the score() function. Here, the
 * addcall2() is need to separate the points and multipliers.
 */


/* collect all relevant data for the actual QSO into a qso_t structure */
struct qso_t *collect_qso_data(void) {
    struct qso_t *qso = g_malloc0(sizeof(struct qso_t));
    qso->call = g_strdup(hiscall);
    qso->mode = trxmode;
    qso->bandindex = bandinx;
    qso->timestamp = get_time();
    qso->comment = g_strdup(comment);
    return qso;
}

// lookup the current country 'n' from the outer loop
// pfxnummulti[I].countrynr contains the country codes,
// I:=[0..pfxnummultinr]
// according to the order of prefixes in rules, eg:
// PFX_NUM_MULTIS=W,VE,VK,ZL,ZS,JA,PY,UA9
// pfxnummulti[0].countrynr will be nr of USA
// pfxnummulti[1].countrynr will be nr of Canada
int lookup_country_in_pfxnummult_array(int n) {
    int found = -1;
    for (int i = 0; i < pfxnummultinr; i++) {
	if (pfxnummulti[i].countrynr == n) {
	    found = i;
	    break;
	}
    }
    return found;
}



int addcall(struct qso_t *qso) {

    int cty, zone = 0;
    int add_ok;
    int pfxnumcntidx = -1;
    int pxnr = 0;

    excl_add_veto = 0;

    int station = lookup_or_add_worked(qso->call);
    update_worked(station, qso);


    // can we get the ctydata from countrynr?
    cty = getctydata(qso->call);


    if (strlen(qso->comment) >= 1) {		/* remember last exchange */
	if (CONTEST_IS(CQWW) || wazmult == 1 || itumult == 1) {
	    /*
	    			if (strlen(zone_fix) > 1) {
	    				zone = zone_nr(zone_fix);
	    			} else
	    				zone = zone_nr(zone_export);
	    */
	    zone = zone_nr(qso->comment);
	}
    }

    add_ok = 1;			/* look if certain calls are excluded */

    if (CONTEST_IS(ARRLDX_USA)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	add_ok = 0;

    if (country_mult == 1 && iscontest)
	add_ok = 1;

    if ((dx_arrlsections == 1)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	add_ok = 0;

    if (CONTEST_IS(PACC_PA))
	add_ok = pacc_pa();

    // if pfx number as multiplier
    if (pfxnummultinr > 0) {
	getpx(qso->call);
	pxnr = districtnumber(wpx_prefix);

	pfxnumcntidx = lookup_country_in_pfxnummult_array(cty);
    }

    if (continentlist_only) {
	if (!is_in_continentlist(continent)) {
	    add_ok = 0;
	    new_cty = 0;
	    addcallarea = 0;
	    excl_add_veto = 1;
	}
    }

    if (!continentlist_only
	    && exclude_multilist_type == EXCLUDE_CONTINENT) {
	if (is_in_continentlist(continent)) {
	    add_ok = 0;
	    new_cty = 0;
	    addcallarea = 0;
	    excl_add_veto = 1;
	}
    }

    if (exclude_multilist_type == EXCLUDE_COUNTRY) {
	if (is_in_countrylist(cty)) {
	    add_ok = 0;
	    new_cty = 0;
	    addcallarea = 0;
	    excl_add_veto = 1;
	}
    }

    if (add_ok == 1) {
	worked[station].band |= inxes[qso->bandindex];	/* worked on this band */

	switch (qso->bandindex) {

	    case BANDINDEX_160:
	    case BANDINDEX_80:
	    case BANDINDEX_40:
	    case BANDINDEX_20:
	    case BANDINDEX_15:
	    case BANDINDEX_10:

		if (pfxnumcntidx < 0) {
		    if (cty != 0 && (countries[cty] & inxes[qso->bandindex]) == 0) {
			countries[cty] |= inxes[qso->bandindex];
			countryscore[qso->bandindex]++;
			new_cty = cty;
		    }
		    if (zone != 0 && (zones[zone] & inxes[qso->bandindex]) == 0) {
			zones[zone] |= inxes[qso->bandindex];
			zonescore[qso->bandindex]++;
			new_zone = zone;
		    }
		} else {
		    if ((pfxnummulti[pfxnumcntidx].qsos[pxnr] & inxes[qso->bandindex])
			    == 0) {
			pfxnummulti[pfxnumcntidx].qsos[pxnr] |= inxes[qso->bandindex];
			addcallarea = 1;
			countryscore[qso->bandindex]++;
			zonescore[qso->bandindex]++;
		    }
		}
		break;


	    case BANDINDEX_12:
	    case BANDINDEX_17:
	    case BANDINDEX_30:

		if (cty != 0 && (countries[cty] & inxes[qso->bandindex]) == 0) {
		    countries[cty] |= inxes[qso->bandindex];
		    new_cty = cty;
		}
		if (zone != 0 && (zones[zone] & inxes[qso->bandindex]) == 0) {
		    zones[zone] |= inxes[qso->bandindex];
		    new_zone = zone;
		}
		break;

	}
    }

    addmult();			/* for wysiwyg */

    return cty;
}

/* ----------------------for network qso's-----------------------------------*/

int addcall2(void) {

    int cty, zone = 0;
    int add_ok;
    char lancopy[6];

    char hiscall[20];
    char comment[40];
    int bandinx;
    int pfxnumcntidx = -1;
    int pxnr = 0;
    excl_add_veto = 0;
    char date_and_time[16];
    time_t qsotimets;

    g_strlcpy(hiscall, lan_logline + 29, 20);
    *strchrnul(hiscall, ' ') = '\0';	/* terminate on first blank */

    g_strlcpy(comment, lan_logline + 54, 31);
    *strchrnul(comment, ' ') = '\0';	/* terminate on first blank */

    /* FIXME: worked array needs mutex protection */
    int station = lookup_or_add_worked(hiscall);

    cty = worked[station].country;

    bandinx = log_get_band(lan_logline);

    /* calculate QSO timestamp from lan_logline */
    strncpy(date_and_time, lan_logline + 7, 15);
    qsotimets = parse_time(date_and_time, DATE_TIME_FORMAT);

    worked[station].qsotime[trxmode][bandinx] = qsotimets;

    if (strlen(comment) >= 1) {
// 		strcpy(station->exchange, comment);

	if (CONTEST_IS(CQWW) || wazmult == 1 || itumult == 1)
	    zone = zone_nr(comment);
    }

    add_ok = 1;			/* look if certain calls are excluded */

    /* 	     if ((arrldx_usa ==1) && ((cty == w_cty) || (cty == ve_cty)))
     	     	add_ok = 0;
    */
    if ((country_mult == 1) && iscontest)
	add_ok = 1;

    if (CONTEST_IS(PACC_PA))
	/* FIXME: Does not work for LAN qso's as pacc_pa uses global variables
	 * set from foreground task */
	add_ok = pacc_pa();

    // if pfx number as multiplier
    if (pfxnummultinr > 0) {
	getpx(hiscall);	    /* FIXME: uses global 'wpx_prefix' for background
			       job */
	pxnr = districtnumber(wpx_prefix);

	pfxnumcntidx = lookup_country_in_pfxnummult_array(cty);
	add_ok = 1;
    }


    if (continentlist_only) {
	if (!is_in_continentlist(dxcc_by_index(cty)->continent)) {
	    excl_add_veto = 1;
	}
    }

    if (!continentlist_only
	    && exclude_multilist_type == EXCLUDE_CONTINENT) {
	if (is_in_continentlist(dxcc_by_index(cty)->continent)) {
	    excl_add_veto = 1;
	}
    }

    if (exclude_multilist_type == EXCLUDE_COUNTRY) {
	if (is_in_countrylist(cty)) {
	    excl_add_veto = 1;
	}
    }

    if (add_ok == 1) {

	bandinx = log_get_band(lan_logline);
	qsos_per_band[bandinx]++;

	worked[station].band |= inxes[bandinx];	/* worked on this band */

	if (excl_add_veto == 0) {

	    switch (bandinx) {

		case BANDINDEX_160:
		case BANDINDEX_80:
		case BANDINDEX_40:
		case BANDINDEX_20:
		case BANDINDEX_15:
		case BANDINDEX_10:

		    if (pfxnumcntidx < 0) {
			if (cty != 0 && (countries[cty] & inxes[bandinx]) == 0) {
			    countries[cty] |= inxes[bandinx];
			    countryscore[bandinx]++;
//                              new_cty = cty;
			}
			if (zone != 0 && (zones[zone] & inxes[bandinx]) == 0) {
			    zones[zone] |= inxes[bandinx];
			    zonescore[bandinx]++;
//                              new_zone = zone;
			}
		    } else {
			if ((pfxnummulti[pfxnumcntidx].qsos[pxnr] & BAND10) == 0) {
			    pfxnummulti[pfxnumcntidx].qsos[pxnr] |= inxes[bandinx];
			    addcallarea = 1;
			    zonescore[bandinx]++;
			    countryscore[bandinx]++;
			}
		    }
		    break;

		case BANDINDEX_30:
		case BANDINDEX_17:
		case BANDINDEX_12:

		    if (cty != 0 && (countries[cty] & inxes[bandinx]) == 0) {
			countries[cty] |= inxes[bandinx];
		    }
		    if (zone != 0 && (zones[zone] & inxes[bandinx]) == 0) {
			zones[zone] |= inxes[bandinx];
		    }
		    break;

	    }
	}
    }
    if (CONTEST_IS(WPX) || pfxmult == 1 || pfxmultab == 1) {

	if (lan_logline[68] != ' ') {

	    strcpy(lancopy, "     ");

	    /* max 5 char for prefix written in makelogline */
	    strncpy(lancopy, lan_logline + 68, 5);

	    for (int i = 0; i <= 5; i++) {	// terminate at first space

		if (lancopy[i] == ' ') {
		    lancopy[i] = '\0';
		    break;
		}
	    }

	    bandinx = log_get_band(lan_logline);

	    add_pfx(lancopy, bandinx);
	}
    }

    addmult2();			/* for wysiwyg from LAN */

    return cty;
}

