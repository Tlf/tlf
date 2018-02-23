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

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <time.h>

#include "addcall.h"
#include "addmult.h"
#include "addpfx.h"
#include "getctydata.h"
#include "getpx.h"
#include "paccdx.h"
#include "searchcallarray.h"
#include "tlf.h"
#include "zone_nr.h"
#include "get_time.h"
#include "dxcc.h"

int excl_add_veto;
/* This variable helps to handle in other modules, that station is multiplier or not */
/* In addcall2(), this variable helps to handle the excluded multipliers, which came from lan_logline
 * the Tlf scoring logic is totally completely different in local and LAN source
 * the addcall() function doesn't increment the band_score[] array, that maintains the score()
 * function. Here, the addcall2() is need to separate the points and multipliers.
 * This variable is used in readcall() too.
 */

int addcall(void) {
    extern char hiscall[];
    extern int nr_worked;
    extern struct worked_t worked[];
    extern char comment[];
    extern int cqww;
    extern int bandinx;
    extern int countries[MAX_DATALINES];
    extern int zones[];
    extern int countryscore[];
    extern int addcty;
    extern int zonescore[];
    extern int addzone;
    extern int countrynr;
    extern int arrldx_usa;
    extern int pacc_pa_flg;
    extern int universal;
    extern int country_mult;
    extern int w_cty;
    extern int ve_cty;
    extern int dx_arrlsections;
    extern int wazmult;
    extern int itumult;
    extern char pxstr[];
    extern t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];
    extern int pfxnummultinr;
    extern int addcallarea;
    extern int continentlist_only;
    extern char continent_multiplier_list[7][3];
    extern char continent[];
    extern int exclude_multilist_type;
    extern char countrylist[][6];
    extern int trxmode;
    extern struct tm *time_ptr;

    static int found = 0;
    static int i, j, z = 0;
    static int add_ok;
    int pfxnumcntidx = -1;
    int pxnr = 0;
    excl_add_veto = 0;

    get_time();

    found = searchcallarray(hiscall);

    if (found == -1) {

	i = nr_worked;
	g_strlcpy(worked[i].call, hiscall, 20);
	nr_worked++;
    } else
	i = found;

    worked[i].qsotime[trxmode][bandinx] = (long)mktime(time_ptr);
    j = getctydata(hiscall);
    worked[i].country = j;
    if (strlen(comment) >= 1) {		/* remember last exchange */
	strcpy(worked[i].exchange, comment);

	if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
	    /*
	    			if (strlen(zone_fix) > 1) {
	    				z = zone_nr(zone_fix);
	    			} else
	    				z = zone_nr(zone_export);
	    */
	    z = zone_nr(comment);

	}
    }

    add_ok = 1;			/* look if certain calls are excluded */

    if ((arrldx_usa == 1)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	add_ok = 0;

    if ((country_mult == 1) && (universal == 1))
	add_ok = 1;

    if ((dx_arrlsections == 1)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	add_ok = 0;

    if (pacc_pa_flg == 1)
	add_ok = pacc_pa();

    // if pfx number as multiplier
    if (pfxnummultinr > 0) {
	getpx(hiscall);
	pxnr = pxstr[strlen(pxstr) - 1] - 48;

	int pfxi = 0;
	while (pfxi < pfxnummultinr) {
	    if (pfxnummulti[pfxi].countrynr == j) {
		pfxnumcntidx = pfxi;
		break;
	    }
	    pfxi++;
	}
    }

    if (continentlist_only == 1 || (continentlist_only == 0
				    && exclude_multilist_type == 1)) {
	int ci = 0;
	int cont_in_list = 0;

	while (strlen(continent_multiplier_list[ci]) != 0) {
	    if (strcmp(continent, continent_multiplier_list[ci]) == 0) {
		cont_in_list = 1;
	    }
	    ci++;
	}

	if ((cont_in_list == 0 && continentlist_only == 1) || (cont_in_list == 1
		&& continentlist_only == 0 && exclude_multilist_type == 1)) {
	    add_ok = 0;
	    addcty = 0;
	    addcallarea = 0;
	    excl_add_veto = 1;
	}
    }

    if (exclude_multilist_type == 2) {
	int ci = 0;
	while (strlen(countrylist[ci]) != 0) {
	    if (getctynr(countrylist[ci]) == j) {
		add_ok = 0;
		addcty = 0;
		addcallarea = 0;
		excl_add_veto = 1;
	    }
	    ci++;
	}
    }

    if (add_ok == 1) {

	worked[i].band |= inxes[bandinx];	/* worked on this band */

	switch (bandinx) {

	    case BANDINDEX_160:
	    case BANDINDEX_80:
	    case BANDINDEX_40:
	    case BANDINDEX_20:
	    case BANDINDEX_15:
	    case BANDINDEX_10:

		if (pfxnumcntidx < 0) {
		    if (j != 0 && (countries[j] & inxes[bandinx]) == 0) {
			countries[j] |= inxes[bandinx];
			countryscore[bandinx]++;
			addcty = j;
		    }
		    if (z != 0 && (zones[z] & inxes[bandinx]) == 0) {
			zones[z] |= inxes[bandinx];
			zonescore[bandinx]++;
			addzone = z;
		    }
		} else {
		    if ((pfxnummulti[pfxnumcntidx].qsos[pxnr] & inxes[bandinx])
			    == 0) {
			pfxnummulti[pfxnumcntidx].qsos[pxnr] |= inxes[bandinx];
			addcallarea = 1;
			countryscore[bandinx]++;
			zonescore[bandinx]++;
		    }
		}
		break;


	    case BANDINDEX_12:
	    case BANDINDEX_17:
	    case BANDINDEX_30:

		if (j != 0 && (countries[j] & inxes[bandinx]) == 0) {
		    countries[j] |= inxes[bandinx];
		    addcty = j;
		}
		if (z != 0 && (zones[z] & inxes[bandinx]) == 0) {
		    zones[z] |= inxes[bandinx];
		    addzone = z;
		}
		break;

	}
    }

    addmult();			/* for wysiwyg */

    return (j);
}

/* -------------------------for network qso's-----------------------------------------*/

int addcall2(void) {

    extern int nr_worked;
    extern struct worked_t worked[];
    extern int cqww;
    extern int countries[MAX_DATALINES];
    extern int zones[];
    extern int countryscore[];
    extern int zonescore[];
    extern int pacc_pa_flg;
    extern int universal;
    extern int country_mult;
    extern char lan_logline[];
    extern int band_score[];
    extern int wpx;
    extern int wazmult;
    extern int itumult;
    extern char pxstr[];
    extern t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];
    extern int pfxnummultinr;
    extern int addcallarea;
    extern int countrynr;
    extern int continentlist_only;
    extern char continent_multiplier_list[7][3];
    extern char continent[];

    extern int pfxmultab;
    extern int exclude_multilist_type;
    extern char countrylist[][6];
    extern int trxmode;

    int found = 0;
    int i, j, p, z = 0;
    int add_ok;
    char lancopy[6];

    char hiscall[20];
    char comment[40];
    int bandinx;
    int pfxnumcntidx = -1;
    int pxnr = 0;
    excl_add_veto = 0;
    char date_and_time[16];
    struct tm qsotime;
    time_t qsotimets;

    g_strlcpy(hiscall, lan_logline + 29, 20);
    *strchrnul(hiscall, ' ') = '\0';	/* terminate on first blank */

    g_strlcpy(comment, lan_logline + 54, 31);
    *strchrnul(comment, ' ') = '\0';	/* terminate on first blank */

    /* FIXME: worked array needs mutex protection */
    found = searchcallarray(hiscall);

    if (found == -1) {

	i = nr_worked;
	g_strlcpy(worked[i].call, hiscall, 20);
	nr_worked++;
    } else
	i = found;

    j = getctynr(hiscall);

    bandinx = get_band(lan_logline);

    /* calculate QSO timestamp from lan_logline */
    memset(&qsotime, 0, sizeof(struct tm));
    strncpy(date_and_time, lan_logline + 7, 15);
    strptime(date_and_time, "%d-%b-%y %H:%M", &qsotime);
    qsotimets = mktime(&qsotime);

    worked[i].qsotime[trxmode][bandinx] = qsotimets;
    worked[i].country = j;
    if (strlen(comment) >= 1) {
//              strcpy(worked[i].exchange,comment);

	if ((cqww == 1) || (wazmult == 1) || (itumult == 1))
	    z = zone_nr(comment);
    }

    add_ok = 1;			/* look if certain calls are excluded */

    /* 	     if ((arrldx_usa ==1) && ((j == w_cty) || (j == ve_cty)))
     	     	add_ok = 0;
    */
    if ((country_mult == 1) && (universal == 1))
	add_ok = 1;

    if (pacc_pa_flg == 1)
	/* FIXME: Does not work for LAN qso's as pacc_pa uses global variables
	 * set from foreground task */
	add_ok = pacc_pa();

    // if pfx number as multiplier
    if (pfxnummultinr > 0) {
	getpx(hiscall);		/* FIXME: uses global 'pxstr' for background
				   job */
	pxnr = pxstr[strlen(pxstr) - 1] - 48;

	int pfxi = 0;
	while (pfxi < pfxnummultinr) {
	    if (pfxnummulti[pfxi].countrynr == j) {
		pfxnumcntidx = pfxi;
		break;
	    }
	    pfxi++;
	}
	add_ok = 1;
    }

    if (continentlist_only == 1 || (continentlist_only == 0
				    && exclude_multilist_type == 1)) {
	int ci = 0;
	int cont_in_list = 0;

	while (strlen(continent_multiplier_list[ci]) != 0) {
	    if (strcmp(dxcc_by_index(j)->continent, continent_multiplier_list[ci])
		    == 0) {
		cont_in_list = 1;
	    }
	    ci++;
	}

	if ((cont_in_list == 0 && continentlist_only == 1) || (cont_in_list == 1
		&& continentlist_only == 0 && exclude_multilist_type == 1)) {
	    excl_add_veto = 1;
	}
    }

    if (exclude_multilist_type == 2) {
	int ci = 0;
	while (strlen(countrylist[ci]) != 0) {
	    if (getctynr(countrylist[ci]) == j) {
		excl_add_veto = 1;
	    }
	    ci++;
	}
    }

    if (add_ok == 1) {

	bandinx = get_band(lan_logline);
	band_score[bandinx]++;

	worked[i].band |= inxes[bandinx];	/* worked on this band */

	if (excl_add_veto == 0) {

	    switch (bandinx) {

		case BANDINDEX_160:
		case BANDINDEX_80:
		case BANDINDEX_40:
		case BANDINDEX_20:
		case BANDINDEX_15:
		case BANDINDEX_10:

		    if (pfxnumcntidx < 0) {
			if (j != 0 && (countries[j] & inxes[bandinx]) == 0) {
			    countries[j] |= inxes[bandinx];
			    countryscore[bandinx]++;
//                              addcty = j;
			}
			if (z != 0 && (zones[z] & inxes[bandinx]) == 0) {
			    zones[z] |= inxes[bandinx];
			    zonescore[bandinx]++;
//                              addzone = z;
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

		    if (j != 0 && (countries[j] & inxes[bandinx]) == 0) {
			countries[j] |= inxes[bandinx];
		    }
		    if (z != 0 && (zones[z] & inxes[bandinx]) == 0) {
			zones[z] |= inxes[bandinx];
		    }
		    break;

	    }
	}
    }
    if (wpx == 1 || pfxmultab == 1) {

	if (lan_logline[68] != ' ') {

	    strcpy(lancopy, "     ");

	    /* max 5 char for prefix written in makelogline */
	    strncpy(lancopy, lan_logline + 68, 5);

	    for (p = 0; p <= 5; p++) {	// terminate at first space

		if (lancopy[p] == ' ') {
		    lancopy[p] = '\0';
		    break;
		}
	    }

	    bandinx = get_band(lan_logline);

	    add_pfx(lancopy, bandinx);
	}
    }

    addmult2();			/* for wysiwyg from LAN */

    return (j);
}

int get_band(char *logline) {

    int j = 0;

    switch (atoi(logline)) {

	case 160:
	    j = BANDINDEX_160;
	    break;

	case 80:
	    j = BANDINDEX_80;
	    break;

	case 40:
	    j = BANDINDEX_40;
	    break;

	case 20:
	    j = BANDINDEX_20;
	    break;

	case 15:
	    j = BANDINDEX_15;
	    break;

	case 10:
	    j = BANDINDEX_10;
	    break;

	case 12:
	    j = BANDINDEX_12;
	    break;

	case 17:
	    j = BANDINDEX_17;
	    break;

	case 30:
	    j = BANDINDEX_30;
	    break;

    }

    return (j);
}
