/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
 *               2012-2022      Thomas Beierlein <dl1jbe@darc.de>
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
#include <errno.h>

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
#include "plugin.h"
#include "score.h"
#include "searchcallarray.h"
#include "startmsg.h"
#include "store_qso.h"
#include "ui_utils.h"


/* Backup original logfile and write a new one from internal database */
void do_backup(const char *logfile, bool interactive) {
    // save a backup
    char prefix[40];
    format_time(prefix, sizeof(prefix), "%Y%m%d_%H%M%S");
    char *backup = g_strdup_printf("%s_%s", prefix, logfile);
    rename(logfile, backup);
    // rewrite log
    for (int i = 0 ; i < NR_QSOS; i++) {
	store_qso(logfile, QSOS(i));
    }
    if (interactive) {
	showstring("Log has been backed up as", backup);
	sleep(1);
    }
    g_free(backup);
}

void init_scoring(void) {
    /* reset counter and score anew */
    total = 0;

    init_qso_array();
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

    if (plugin_has_setup()) {
	plugin_setup();
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

int readcalls(const char *logfile, bool interactive) {
    char *inputbuffer = NULL;
    size_t inputbuffer_len = 0;
    struct qso_t *qso;
    int linenr = 0;
    ssize_t read;

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

    while ((read = getline(&inputbuffer, &inputbuffer_len, fp)) != -1) {
	if (inputbuffer_len > 0) {
	    if (errno == ENOMEM) {
		fprintf(stderr, "Error in: %s:%d", __FILE__, __LINE__);
		perror("RuntimeError: ");
		exit(EXIT_FAILURE);
	    }
	    // drop trailing newline
	    inputbuffer[inputbuffer_len - 1] = '\0';
	    linenr++;

	    if (interactive) {
		show_progress(linenr);
	    }

	    qso = parse_qso(inputbuffer);

	    if (qso->is_comment) {
		g_ptr_array_add(qso_array, qso);
		continue;		/* skip further processing for note entry */
	    }

	    /* get the country number, not known at this point */
	    countrynr = getctydata(qso->call);
	    checkexchange(qso, false);
	    if (qso->normalized_comment != NULL && strlen(qso->normalized_comment) > 0) {
		strcpy(qso->comment, qso->normalized_comment);
	    }

	    dupe = is_dupe(qso->call, qso->bandindex, qso->mode);
	    addcall(qso);
	    score_qso(qso);
	    char *logline = makelogline(qso);

	    // Ignore new line character at end of line in qso->logline
	    if (strcmp(logline, strtok(qso->logline, "\n")) != 0) {
		g_free(qso->logline);
		qso->logline = g_strdup(logline);
		log_changed = true;
	    }

	    g_free(logline);

	    if (inputbuffer != NULL)
		free(inputbuffer);

	    // drop transient fields
	    FREE_DYNAMIC_STRING(qso->callupdate);
	    FREE_DYNAMIC_STRING(qso->normalized_comment);
	    FREE_DYNAMIC_STRING(qso->section);

	    g_ptr_array_add(qso_array, qso);
	}
    }

    fclose(fp);

    if (log_changed) {
	bool ok = false;
	if (interactive) {
	    showmsg("Log changed due to rescoring. Do you want to save it? Y/(N)");
	    ok = toupper(key_get()) == 'Y';
	} else {
	    ok = true;
	}

	if (ok) {
	    do_backup(logfile, interactive);
	}
    }

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
