/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
 *               2012-2021      Thomas Beierlein <dl1jbe@darc.de>
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
#include <ctype.h>
#include <time.h>

#include "addcall.h"
#include "addpfx.h"
#include "addmult.h"
#include "cabrillo_utils.h"
#include "getexchange.h"
#include "getctydata.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "ignore_unused.h"
#include "log_utils.h"
#include "makelogline.h"
#include "readqtccalls.h"
#include "score.h"
#include "searchcallarray.h"
#include "startmsg.h"
#include "store_qso.h"
#include "ui_utils.h"


void init_scoring(void) {
    /* reset counter and score anew */
    total = 0;

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

    for (int i = 0; i < pfxnummultinr; i++) {
	for (int n = 0; n < PFXNUMBERS; n++) {
	    pfxnummulti[i].qsos[n] = 0;
	}
    }
}

static void show_progress(int linenr) {
    if (linenr == 1) {
	printw("  ");  // leading separator after log file name
	refreshp();
    }
    if (((linenr + 1) % 100) == 0) {
	printw("*");
	refreshp();
    }
}

static void qso_free(gpointer data) {
    free_qso((struct qso_t *) data);
}

int readcalls(const char *logfile, bool interactive) {

    char inputbuffer[LOGLINELEN + 1];
    struct qso_t *qso;
    int linenr = 0;

    FILE *fp;

    if (interactive) {
	showstring("Reading logfile:", (char *)logfile);
    }

    init_scoring();

    if ((fp = fopen(logfile, "r")) == NULL) {
	showmsg("Error opening logfile ");
	sleep(2);
	exit(1);
    }

    bool log_changed = false;
    // array of qso's
    // FIXME use this instead of qsos[] (make it global and don't free it here)
    // FIXME also include comments/notes into qso_t
    GPtrArray *qso_array = g_ptr_array_new_with_free_func(qso_free);

    while (fgets(inputbuffer, sizeof(inputbuffer), fp) != NULL) {

	// drop trailing newline
	inputbuffer[LOGLINELEN - 1] = '\0';

	// remember logline in qsos[] field
	g_strlcpy(qsos[linenr], inputbuffer, sizeof(qsos[0]));
	linenr++;
	// FIXME check for overflow....

	if (interactive) {
	    show_progress(linenr);
	}

	if (log_is_comment(inputbuffer))
	    continue;		/* skip note in log */

	qso = parse_qso(inputbuffer);

	/* get the country number, not known at this point */
	countrynr = getctydata(qso->call);
	checkexchange(qso->comment, false);
	if (strlen(normalized_comment) > 0) {   //FIXME global
	    strcpy(qso->comment, normalized_comment);
	}
	dupe = is_dupe(qso->call, qso->bandindex, qso->mode);

	addcall(qso);
	score_qso(qso);

	char *logline = makelogline(qso);

	if (strcmp(logline, qsos[linenr - 1]) != 0) {
	    // different: update log line and mark change
	    g_strlcpy(qsos[linenr - 1], logline, sizeof(qsos[0]));
	    g_free(qso->logline);
	    qso->logline = g_strdup(logline);
	    log_changed = true;
	}

	g_free(logline);

	g_ptr_array_add(qso_array, qso);
    }

    fclose(fp);

    if (log_changed && interactive) {
	showmsg("Log changed due to rescoring. Do you want to save it? Y/(N)");
	if (toupper(key_get()) == 'Y') {
	    // save a backup
	    char prefix[40];
	    format_time(prefix, sizeof(prefix), "%Y%m%d_%H%M%S");
	    char *backup = g_strdup_printf("%s_%s", prefix, logfile);
	    rename(logfile, backup);
	    // rewrite log
	    nr_qsos = 0;    // FIXME store_qso increments nr_qsos
	    for (int i = 0 ; i < linenr; i++) {
		store_qso(qsos[i]);
	    }
	    showstring("Log has been backed up as", backup);
	    g_free(backup);
	    sleep(1);
	}
    }

    g_ptr_array_free(qso_array, TRUE);  // FIXME keep the array

    return linenr;			// nr of lines in log
}

int log_read_n_score() {
    int nr_qsolines = readcalls(logfile, false);

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
