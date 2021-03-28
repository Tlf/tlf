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
 *        Initialize  call array for dupes
 *
 *--------------------------------------------------------------*/

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "addmult.h"
#include "addpfx.h"
#include "bands.h"
#include "cabrillo_utils.h"
#include "get_time.h"
#include "getctydata.h"
#include "getpx.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "ignore_unused.h"
#include "log_utils.h"
#include "paccdx.h"
#include "readqtccalls.h"
#include "plugin.h"
#include "score.h"
#include "searchcallarray.h"
#include "setcontest.h"
#include "startmsg.h"
#include "tlf_curses.h"
#include "zone_nr.h"


void init_scoring(void) {
    /* reset counter and score anew */
    for (int i = 0; i < MAX_QSOS; i++)
	qsos[i][0] = '\0';

    init_worked();

    for (int i = 1; i <= MAX_DATALINES - 1; i++)
	countries[i] = 0;

    for (int i = 0; i < NBANDS; i++)
	qsos_per_band[i] = 0;

    for (int i = 0; i < NBANDS; i++)
	countryscore[i] = 0;

    for (int n = 1; n < MAX_ZONES; n++)
	zones[n] = 0;

    for (int n = 0; n < NBANDS; n++)
	zonescore[n] = 0;

    init_mults();

    InitPfx();

    if (pfxnummultinr > 0) {
	for (int i = 0; i < pfxnummultinr; i++) {
	    for (int n = 0; n < NBANDS; n++) {
		pfxnummulti[i].qsos[n] = 0;
	    }
	}
    }

    if (plugin_has_setup()) {
        plugin_setup();
    }
}

void show_progress(int linenr) {
    if (((linenr + 1) % 100) == 0) {
	printw("*");
	refreshp();
    }
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

bool check_veto() {
    bool veto = false;

    if (!continentlist_only &&
	    exclude_multilist_type == EXCLUDE_CONTINENT) {
	if (is_in_continentlist(continent)) {
	    veto = true;
	}
    }

    if (exclude_multilist_type == EXCLUDE_COUNTRY) {
	if (is_in_countrylist(countrynr)) {
	    veto = true;
	}
    }

    return veto;
}

/* pick up multi string from logline
 *
 * ATTENTION! return value needs to be freed
 */
char *get_multi_from_line(char *logline) {
    char *multbuffer = g_malloc(20);

    if (CONTEST_IS(ARRL_SS)) {

	if (logline[63] == ' ')
	    g_strlcpy(multbuffer, logline + 64, 4);
	else
	    g_strlcpy(multbuffer, logline + 63, 4);

    } else if ((serial_section_mult == 1)
	       || (sectn_mult == 1)
	       || (sectn_mult_once == 1)) {

	g_strlcpy(multbuffer, logline + 68, MAX_SECTION_LENGTH + 1);
	g_strchomp(multbuffer);

    } else if (serial_grid4_mult == 1) {

	g_strlcpy(multbuffer, logline + 59, 5);

    } else if (unique_call_multi != 0) {

	g_strlcpy(multbuffer, logline + 68, 10);
	g_strchomp(multbuffer);

    } else {

	g_strlcpy(multbuffer, logline + 54, 11);	// normal case
	g_strchomp(multbuffer);

    }

    return multbuffer;
}


void count_if_worked(int check, unsigned int bandindex, int *count) {
    if (bandindex >= NBANDS)
	return;

    if ((check & inxes[bandindex]) != 0)
	count[bandindex]++;
}

void count_contest_bands(int check, int *count) {
    count_if_worked(check, BANDINDEX_160, count);
    count_if_worked(check, BANDINDEX_80, count);
    count_if_worked(check, BANDINDEX_40, count);
    count_if_worked(check, BANDINDEX_20, count);
    count_if_worked(check, BANDINDEX_15, count);
    count_if_worked(check, BANDINDEX_10, count);
}



int readcalls(void) {

    char inputbuffer[LOGLINELEN + 1];
    char checkcall[20];
    int z = 0;
    bool add_ok;
    char presentcall[20];	// copy of call..
    char *tmpptr;
    int pfxnumcntidx;
    int pxnr;
    bool veto;
    int qsomode;
    int linenr = 0;

    int bandindex = BANDINDEX_OOB;

    FILE *fp;

    showmsg("Reading logfile... ");
    refreshp();

    init_scoring();

    if ((fp = fopen(logfile, "r")) == NULL) {
	showmsg("Error opening logfile ");
	refreshp();
	sleep(2);
	exit(1);
    }

    while (fgets(inputbuffer, LOGLINELEN + 1, fp) != NULL) {

	// drop trailing newline
	inputbuffer[LOGLINELEN - 1] = '\0';

	// remember logline in qsos[] field
	g_strlcpy(qsos[linenr], inputbuffer, sizeof(qsos[0]));
	linenr++;

	show_progress(linenr);

	if (log_is_comment(inputbuffer))
	    continue;		/* skip note in  log  */

	// prepare helper variables
	pfxnumcntidx = -1;
	pxnr = 0;

	/* get bandindex */
	bandindex = log_get_band(inputbuffer);

	/* get the country number, not known at this point */
	g_strlcpy(presentcall, inputbuffer + 29, 14);
	tmpptr = strchr(presentcall, ' ');
	if (tmpptr)
	    *tmpptr = '\0';

	countrynr = getctydata(presentcall);

	if (continentlist_only) {
	    if (!is_in_continentlist(continent)) {
		qsos_per_band[bandindex]++;
		continue;
	    }
	}

	if (iscontest) {
	    // get points
	    total = total + log_get_points(inputbuffer);

	    if (CONTEST_IS(CQWW) || (itumult == 1) || (wazmult == 1)) {
		// get the zone
		z = zone_nr(inputbuffer + 54);
	    }

	    if (wysiwyg_once == 1 ||
		    wysiwyg_multi == 1 ||
		    unique_call_multi != 0 ||
		    CONTEST_IS(ARRL_SS) ||
		    serial_section_mult == 1 ||
		    serial_grid4_mult == 1 ||
		    sectn_mult == 1 ||
		    sectn_mult_once == 1 ||
		    ((dx_arrlsections == 1)
		     && ((countrynr == w_cty) || (countrynr == ve_cty)))) {
		// get multi info
		char *multbuffer = get_multi_from_line(inputbuffer);
		remember_multi(multbuffer, bandindex, 0);
		g_free(multbuffer);
	    }
	}

	/*  lookup worked stations, add if new */
	int station = lookup_or_add_worked(presentcall);

	/* and fill in according entry */
	g_strlcpy(worked[station].exchange, inputbuffer + 54, 12);
	g_strchomp(worked[station].exchange);	/* strip trailing spaces */

	qsomode = log_get_mode(inputbuffer);
	if (qsomode == -1) {
	    shownr("Invalid line format in line %d.\n", linenr);
	    refreshp();
	    sleep(2);
	    exit(1);
	}

	/* calculate QSO timestamp from logline */
	worked[station].qsotime[qsomode][bandindex] =
	    parse_time(inputbuffer + 7,	DATE_TIME_FORMAT);


	if (pfxmultab == 1) {
	    getpx(presentcall);
	    add_pfx(wpx_prefix, bandindex);
	}


	/* look if calls are excluded */
	add_ok = true;

	if (CONTEST_IS(ARRLDX_USA)
		&& ((countrynr == w_cty) || (countrynr == ve_cty)))
	    add_ok = false;

	if (CONTEST_IS(PACC_PA)) {

	    strcpy(hiscall, presentcall);

	    add_ok = pacc_pa();

	    if (add_ok == false) {
		qsos_per_band[bandindex]++;
	    }

	    hiscall[0] = '\0';
	}

	if (pfxnummultinr > 0) {
	    getpx(presentcall);
	    pxnr = districtnumber(wpx_prefix);

	    getctydata(presentcall);

	    pfxnumcntidx = lookup_country_in_pfxnummult_array(countrynr);

	    add_ok = true;
	}

	veto = check_veto();

	if (add_ok) {

	    worked[station].band |= inxes[bandindex];	/* mark band as worked */

	    qsos_per_band[bandindex]++;

	    if (CONTEST_IS(CQWW) || (itumult == 1) || (wazmult == 1))
		zones[z] |= inxes[bandindex];

	    if (pfxnumcntidx < 0) {
		if (!veto) {
		    countries[countrynr] |= inxes[bandindex];
		}
	    } else {
		pfxnummulti[pfxnumcntidx].qsos[pxnr] |= inxes[bandindex];
	    }
	}
    }

    fclose(fp);

    /* all lines red, now build other statistics */
    if (CONTEST_IS(WPX) || pfxmult == 1) {

	/* build prefixes_worked array from list of worked stations */
	InitPfx();

	for (int n = 0; n < nr_worked; n++) {
	    strcpy(checkcall, worked[n].call);
	    getpx(checkcall);
	    /* FIXME: wpx is counting pfx only once so bandindex is not
	     * really needed here. If you have wpx and pfxmultab set the
	     * count for the last read band wil be wrong as all pfx will be
	     * counted for that band.
	     * Maybe better use BANDINDEX_OOB here:
	     * - Will count pfx for wpx correctly
	     * - but will not change counts for pfxmultab on contest bands */
	    add_pfx(wpx_prefix, BANDINDEX_OOB);
	}
    }

    if (CONTEST_IS(CQWW) || (itumult == 1) || (wazmult == 1)) {
	for (int n = 1; n < MAX_ZONES; n++) {
	    count_contest_bands(zones[n], zonescore);
	}
    }

    if (CONTEST_IS(CQWW)) {
	for (int n = 1; n <= MAX_DATALINES - 1; n++) {
	    count_contest_bands(countries[n], countryscore);
	}
    }

    if (dx_arrlsections == 1) {
	for (int cntr = 1; cntr < MAX_DATALINES; cntr++) {
	    if (cntr != w_cty && cntr != ve_cty) {	// W and VE don't count here...
		count_contest_bands(countries[cntr], countryscore);
	    }
	}
    }

    if (CONTEST_IS(ARRLDX_USA)) {
	for (int cntr = 1; cntr < MAX_DATALINES; cntr++) {
	    if (cntr != w_cty && cntr != ve_cty) {	// W and VE don't count here...
		count_contest_bands(countries[cntr], countryscore);
	    }
	}
    }

    if (CONTEST_IS(PACC_PA)) {
	for (int n = 1; n < MAX_DATALINES; n++) {
	    count_contest_bands(countries[n], countryscore);
	}
    }

    if (country_mult == 1 || pfxnummultinr > 0) {

	for (int n = 1; n <= MAX_DATALINES - 1; n++) {

	    pfxnumcntidx = -1;
	    if (pfxnummultinr > 0) {
		pfxnumcntidx = lookup_country_in_pfxnummult_array(n);
	    }

	    if (pfxnumcntidx >= 0) { /* found in array */
		/* count all possible pfx numbers
		 * eg: K0, K1, K2, ..., K9 */
		for (int pfxnr = 0; pfxnr < 10; pfxnr++) {
		    count_contest_bands(pfxnummulti[pfxnumcntidx].qsos[pfxnr],
					countryscore);
		}
	    } else {
		// simple 'country_mult', but works together with pfxnummulti
		count_contest_bands(countries[n], countryscore);
	    }
	}
    }

    return linenr;			// nr of lines in log
}

int log_read_n_score() {
    int nr_qsolines;

    total = 0;
    nr_qsolines = readcalls();
    if (qtcdirection > 0) {
	readqtccalls();
    }
    return nr_qsolines;
}

//------------------------------------------------------------------------

int synclog(char *synclogfile) {

    char wgetcmd[120] = "wget ftp://";	//user:password@hst/dir/file
    char date_buf[60];

    format_time(date_buf, sizeof(date_buf), "%d%H%M");

    if (strlen(synclogfile) < 80)
	strcat(wgetcmd, synclogfile);
    else {
	showmsg("Warning: Name of syncfile too long\n");
	sleep(5);
	exit(1);
    }
    strcat(wgetcmd, " -O log1 -o wgetlogfile");

    if (system(wgetcmd) == 0)
	showmsg("Syncfile o.k.\n");
    else {
	showmsg("Warning: Did not get syncfile !!\nExiting...\n");
	sleep(5);
	exit(1);
    }

    wgetcmd[0] = '\0';
    sprintf(wgetcmd, "cp %s log2", logfile);
    if (system(wgetcmd) != 0)
	showstring("\nCopying logfile %s failed\n", logfile);

    showmsg("Backing up logfile.\n");
    sleep(1);
    sprintf(wgetcmd, "cp %s %s%s", logfile, date_buf, logfile);
    if (system(wgetcmd) != 0)
	showstring("\nCopying logfile %s to backup failed\n", logfile);

    showmsg("Merging logfiles...\n");
    sleep(1);
    sprintf(wgetcmd, "cat log1 log2 | sort -g -k4,4 | uniq  > %s",
	    logfile);
    if (system(wgetcmd) == 0)
	showmsg("Merging logs successful\n");
    else {
	showmsg("Problem merging logs.\nExiting...\n");
	sleep(5);
	exit(1);
    }
    sleep(1);
    IGNORE(system("rm log1"));;
    IGNORE(system("rm log2"));;

    return (0);
}
