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
	/* ------------------------------------------------------------
	 *        Initialize  call array for dupes
	 *
	 *--------------------------------------------------------------*/

#include "readcalls.h"
#include "addpfx.h"
#include "addmult.h"
#include "get_time.h"
#include <curses.h>
#include "tlf.h"
#include "globalvars.h"
#include <glib.h>

int readcalls(void)
{
    char inputbuffer[160];
    char tmpbuf[20];
    char bndbuf[20];
    char zonebuf[3];
    char checkcall[20];
    int i = 0, k = 0, l = 0, n = 0, r = 0, s = 0;
    int m = 0;
    int t = 0, tt = 0;
    int z = 0;
    int add_ok;
    char multbuffer[40];
    char presentcall[20];	// copy of call..
    char *tmpptr;
    int points;

    FILE *fp;

    clear();
    mvprintw(4, 0, "Reading logfile...\n");
    refreshp();

    /* reset counter and score anew */
    for (s = 0; s < MAX_QSOS; s++)
	qsos[s][0] = '\0';

    for (i = 0; i < MAX_CALLS; i++) {
	callarray[i][0] = '\0';
	call_band[i] = 0;
	call_country[i] = -1;
    }

    for (i = 1; i <= MAX_DATALINES - 1; i++)
	countries[i] = 0;

    for (i = 0; i < NBANDS; i++)
	band_score[i] = 0;

    for (i = 0; i <= 5; i++)
	countryscore[i] = 0;

    for (n = 1; n <= 40; n++)
	zones[n] = 0;

    for (n = 0; n < 6; n++)
	zonescore[n] = 0;

    for (n = 0; n < NBANDS; n++)	//F6CFE
	multscore[n] = 0;

    init_mults();

    if ((fp = fopen(logfile, "r")) == NULL) {
	mvprintw(5, 0, "Error opening logfile.\n");
	refreshp();
	sleep(2);
    }
    i = 0;
    k = 0;
    l = 0;
    total = 0;
    s = 0;
    t = 0;

    while (fgets(inputbuffer, 90, fp) != NULL) {

	r++;

	if (r >= 100) {
	    r = 0;
	    printw("*");
	    refreshp();
	}

	strcat(inputbuffer, "                                                  ");	/* repair the logfile */
	inputbuffer[LOGLINELEN-1] = '\0';

	for (t = 0; t <= strlen(inputbuffer); t++) {
	    if (inputbuffer[t] == '\n')
		inputbuffer[t] = ' ';
	}

	strncpy(qsos[s], inputbuffer, LOGLINELEN);
	s++;

	if (inputbuffer[0] == ';')
	    continue;		/*  note in  log  */

	strncpy(presentcall, inputbuffer + 29, 13);
	presentcall[13] = '\0';

	strncpy(bndbuf, inputbuffer + 1, 2);
	bndbuf[2] = '\0';

	if (bndbuf[0] == '1' && bndbuf[1] == '0')
	    bandinx = BANDINDEX_10;
	if (bndbuf[0] == '1' && bndbuf[1] == '5')
	    bandinx = BANDINDEX_15;
	if (bndbuf[0] == '2')
	    bandinx = BANDINDEX_20;
	if (bndbuf[0] == '4')
	    bandinx = BANDINDEX_40;
	if (bndbuf[0] == '8')
	    bandinx = BANDINDEX_80;
	if (bndbuf[0] == '6')
	    bandinx = BANDINDEX_160;

	/* get the country number, not known at this point */
	tmpptr = strchr(presentcall, ' ');
	if (tmpptr)
	    *tmpptr = '\0';
	strcpy(tmpbuf, presentcall);
	countrynr = getctydata(tmpbuf);

	if (contest == 1) {
	    strncpy(tmpbuf, inputbuffer + 76, 2);	/* get the points */
	    tmpbuf[2] = '\0';
	    points = atoi(tmpbuf);
	    total = total + points;

	    if (cqww == 1) {
		strncpy(zonebuf, inputbuffer + 54, 2);	/* get the zone */
		zonebuf[2] = '\0';
		z = zone_nr(zonebuf);
	    }

	    if (wysiwyg_once == 1 ||
		wysiwyg_multi == 1 ||
		arrlss == 1 ||
		serial_section_mult == 1 ||
		serial_grid4_mult == 1 ||
		sectn_mult == 1 ||
		((dx_arrlsections == 1)
		 && ((countrynr == w_cty) || (countrynr == ve_cty)))) {

		multbuffer[0] = '\0';

		if (arrlss == 1) {
		    other_flg = 0;

		    if (inputbuffer[63] == ' ')
			strncpy(multbuffer, inputbuffer + 64, 3);
		    else
			strncpy(multbuffer, inputbuffer + 63, 3);

		    multbuffer[3] = '\0';

		} else if (serial_section_mult == 1) {

		    tt = 0;

		    memset(multbuffer, 0, 39);

		    for (t = 54; t < 64; t++) {
			if (inputbuffer[t] >= 'A' && inputbuffer[t] <= 'Z') {
			    multbuffer[tt] = inputbuffer[t];
			    tt++;
			}
		    }

		} else if (serial_grid4_mult == 1) {

		    memset(multbuffer, 0, 39);

		    for (t = 0; t < 4; t++) {

			multbuffer[t] = inputbuffer[t + 59];
		    }

		} else {

		    strncpy(multbuffer, inputbuffer + 54, 10);	// normal case

		    multbuffer[10] = '\0';

		    g_strchomp(multbuffer);

		}

		remember_multi( multbuffer, bandinx, 0 );

	    }			// end wysiwig

	    if (other_flg == 1) {	/* mult = max 3 characters */

		strncpy(multbuffer, inputbuffer + 54, 3);
		multbuffer[3] = '\0';

		if (multbuffer[3] == ' ')
		    multbuffer[3] = '\0';
		if (multbuffer[2] == ' ')
		    multbuffer[2] = '\0';
		if (multbuffer[1] == ' ')
		    multbuffer[1] = '\0';

		remember_multi( multbuffer, bandinx, 0 );
	    }

	}
	/*  once  per call !  */
	for (k = 0; k < i; k++) {	// changed k=< i
	    m = strcmp(callarray[k], presentcall);

	    if (m == 0) {
		l = k;
		break;
	    } else
		l = i;

	}

	strncpy(callarray[l], inputbuffer + 29, 19);
	callarray[l][19] = 0;
	strtok(callarray[l], " \r");	/* delimit first word */

	call_country[l] = countrynr;
	g_strlcpy(call_exchange[l], inputbuffer + 54, 12);
	g_strchomp(call_exchange[l]);	/* strip trailing spaces */

	add_ok = 1;		/* look if calls are excluded */

	if ((arrldx_usa == 1)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	    add_ok = 0;

	if (pacc_pa_flg == 1) {

	    strcpy(hiscall, presentcall);

	    add_ok = pacc_pa();

	    if (add_ok == 0) {
		band_score[bandinx]++;
	    }

	    hiscall[0] = '\0';
	}

	if (add_ok == 1) {

	    call_band[l] |= inxes[bandinx];	/* mark band as worked */
	    band_score[bandinx]++;	/*  qso counter  per band */
	    if (cqww == 1)
		zones[z] |= inxes[bandinx];
	    countries[countrynr] |= inxes[bandinx];

	}			/* end add_ok */

	if (l == i)
	    i++;
    }

    fclose(fp);

    /* remember nuber of callarray entries */
    callarray_nr = i;

    if (wpx == 1) {

	/* build prefixes_worked array from list of worked stations */
	nr_of_px = 0;

	for (n = 0; n < i; n++) {
	    strcpy(checkcall, callarray[n]);
	    getpx(checkcall);
	    add_pfx(pxstr);
	}
    }

    if (cqww == 1) {
	for (n = 1; n <= 40; n++) {
	    if ((zones[n] & BAND160) != 0)
		zonescore[0]++;
	    if ((zones[n] & BAND80) != 0)
		zonescore[1]++;
	    if ((zones[n] & BAND40) != 0)
		zonescore[2]++;
	    if ((zones[n] & BAND20) != 0)
		zonescore[3]++;
	    if ((zones[n] & BAND15) != 0)
		zonescore[4]++;
	    if ((zones[n] & BAND10) != 0)
		zonescore[5]++;
	}

	for (n = 1; n <= MAX_DATALINES - 1; n++) {
	    if ((countries[n] & BAND160) != 0)
		countryscore[0]++;
	    if ((countries[n] & BAND80) != 0)
		countryscore[1]++;
	    if ((countries[n] & BAND40) != 0)
		countryscore[2]++;
	    if ((countries[n] & BAND20) != 0)
		countryscore[3]++;
	    if ((countries[n] & BAND15) != 0)
		countryscore[4]++;
	    if ((countries[n] & BAND10) != 0)
		countryscore[5]++;
	}
    }
    /* end cqww */
    if (dx_arrlsections == 1) {

	int cntr;

	for (cntr = 1; cntr < MAX_DATALINES; cntr++) {

	    if (cntr != w_cty && cntr != ve_cty)	// W and VE don't count here...
	    {
		if ((countries[cntr] & BAND160) != 0)
		    countryscore[0]++;
		if ((countries[cntr] & BAND80) != 0)
		    countryscore[1]++;
		if ((countries[cntr] & BAND40) != 0)
		    countryscore[2]++;
		if ((countries[cntr] & BAND20) != 0)
		    countryscore[3]++;
		if ((countries[cntr] & BAND15) != 0)
		    countryscore[4]++;
		if ((countries[cntr] & BAND10) != 0)
		    countryscore[5]++;
	    }
	}
    }				// end dx_arrlsections

    if ((arrldx_usa == 1) && (countrynr != w_cty) && (countrynr != ve_cty)) {

	int cntr;
	for (cntr = 1; cntr < MAX_DATALINES; cntr++) {
	    if ((countries[cntr] & BAND160) != 0)
		countryscore[0]++;
	    if ((countries[cntr] & BAND80) != 0)
		countryscore[1]++;
	    if ((countries[cntr] & BAND40) != 0)
		countryscore[2]++;
	    if ((countries[cntr] & BAND20) != 0)
		countryscore[3]++;
	    if ((countries[cntr] & BAND15) != 0)
		countryscore[4]++;
	    if ((countries[cntr] & BAND10) != 0)
		countryscore[5]++;
	}

    }

    /* end arrldx_usa */
    if (pacc_pa_flg == 1) {

	for (n = 1; n < MAX_DATALINES; n++) {
	    if ((countries[n] & BAND160) != 0)
		countryscore[0]++;
	    if ((countries[n] & BAND80) != 0)
		countryscore[1]++;
	    if ((countries[n] & BAND40) != 0)
		countryscore[2]++;
	    if ((countries[n] & BAND20) != 0)
		countryscore[3]++;
	    if ((countries[n] & BAND15) != 0)
		countryscore[4]++;
	    if ((countries[n] & BAND10) != 0)
		countryscore[5]++;
	}
    }

    if (country_mult == 1) {

	for (n = 1; n <= MAX_DATALINES - 1; n++) {
	    if ((countries[n] & BAND160) != 0)
		countryscore[0]++;
	    if ((countries[n] & BAND80) != 0)
		countryscore[1]++;
	    if ((countries[n] & BAND40) != 0)
		countryscore[2]++;
	    if ((countries[n] & BAND20) != 0)
		countryscore[3]++;
	    if ((countries[n] & BAND15) != 0)
		countryscore[4]++;
	    if ((countries[n] & BAND10) != 0)
		countryscore[5]++;
	}
    }

    if (qsonum == 1) {
	nr_of_px = 0;
	total = 0;
	for (i = 0; i < NBANDS; i++)
	    band_score[i] = 0;

	for (i = 0; i <= 5; i++)
	    countryscore[i] = 0;

	for (i = 0; i < NBANDS; i++)
	    multscore[i] = 0;

    }


    /* \todo check what the following code is for tb 19sep2011 */
    if (((serial_section_mult == 1)
//              || (serial_grid4_mult == 1)
	 || (sectn_mult == 1)) && multarray_nr == 1) {	// correction ......
	mults[1][0] = '\0';
	mult_bands[1] = 0;
    } else if (serial_section_mult == 1 && multarray_nr > 1)
	multarray_nr++;

    return (s);			// nr of lines in log
}

	//------------------------------------------------------------------------

int synclog(char *synclogfile)
{
    extern char logfile[];
    extern struct tm *time_ptr;

    int rc;
    char wgetcmd[120] = "wget ftp://";	//user:password@hst/dir/file
    char date_buf[60];

    get_time();
    strftime(date_buf, 9, "%d%H%M", time_ptr);

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
    sprintf(wgetcmd, "cat log1 log2 | sort -g -k4,4 | uniq -w79 > %s",
	    logfile);
    if (system(wgetcmd) == 0)
	showmsg("Merging logs successfull\n");
    else {
	showmsg("Problem merging logs.\nExiting...\n");
	sleep(5);
	exit(1);
    }
    sleep(1);
    rc = system("rm log1");
    rc = system("rm log2");

    return (0);
}
