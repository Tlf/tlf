/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2019-22 Thomas Beierlein <dl1jbe@darc.de>
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
/* ------------------------------------------------------------------------
*    util functions to work with lines from log
*
---------------------------------------------------------------------------*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <ctype.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bands.h"
#include "get_time.h"
#include "setcontest.h"
#include "tlf.h"


/* for the following code we assume that we have well formatted log lines,
 * which has to be checked separately if needed */

/** check if logline is only a comment */
bool log_is_comment(const char *buffer) {

    if (buffer[0] != ';')
	return 0;
    else
	return 1;
}


/** read bandindex from logline */
int log_get_band(const char *logline) {

    int nr = 0;
    char band[4];

    g_strlcpy(band, logline, 4);

    nr = atoi(band);

    return bandnr2index(nr);
}


/** read mode from logline
 * -1 if no recognized mode */
int log_get_mode(const char *logline) {
    if (strncasecmp("CW ", logline + 3, 3) == 0) {
	return CWMODE;
    }
    if (strncasecmp("SSB", logline + 3, 3) == 0) {
	return SSBMODE;
    }
    if (strncasecmp("DIG", logline + 3, 3) == 0) {
	return DIGIMODE;
    }
    return -1;
}

/** read points from logline */
int log_get_points(const char *logline) {
    char tmpbuf[3];

    g_strlcpy(tmpbuf, logline + 76, 3);
    return atoi(tmpbuf);
}


struct qso_t *parse_qso(char *buffer) {
    char *tmp;
    char *sp;
    struct qso_t *ptr;
    struct tm date_n_time;

    ptr = g_malloc0(sizeof(struct qso_t));

    /* remember whole line */
    ptr->logline = g_strdup(buffer);
    ptr->qsots = 0;

    if (log_is_comment(buffer)) {
	ptr->is_comment = true;
	return ptr;
    }

    ptr->is_comment = false;

    /* split buffer into parts for linedata_t record and parse
     * them accordingly */
    tmp = strtok_r(buffer, " \t", &sp);

    /* band */
    ptr->band = atoi(tmp);
    ptr->bandindex = bandnr2index(ptr->band);


    /* mode */
    if (strcasestr(tmp, "CW"))
	ptr->mode = CWMODE;
    else if (strcasestr(tmp, "SSB"))
	ptr->mode = SSBMODE;
    else
	ptr->mode = DIGIMODE;

    /* date & time */
    memset(&date_n_time, 0, sizeof(struct tm));

    strptime(strtok_r(NULL, " \t", &sp), DATE_FORMAT, &date_n_time);
    strptime(strtok_r(NULL, " \t", &sp), TIME_FORMAT, &date_n_time);
    ptr->timestamp = timegm(&date_n_time);

    ptr->year = date_n_time.tm_year + 1900;	/* convert to
						   1968..2067 */
    ptr->month = date_n_time.tm_mon + 1;	/* tm_mon = 0..11 */
    ptr->day   = date_n_time.tm_mday;

    ptr->hour  = date_n_time.tm_hour;
    ptr->min   = date_n_time.tm_min;

    /* qso number */
    ptr->qso_nr = atoi(strtok_r(NULL, " \t", &sp));

    /* his call */
    ptr->call = g_strdup(strtok_r(NULL, " \t", &sp));

    /* RST send and received */
    ptr->rst_s = atoi(strtok_r(NULL, " \t", &sp));
    ptr->rst_r = atoi(strtok_r(NULL, " \t", &sp));

    /* comment (exchange) */
    ptr->comment = g_strndup(buffer + 54, contest->exchange_width);

    /* tx */
    ptr->tx = (buffer[79] == '*') ? 1 : 0;

    /* frequency (kHz) */
    ptr->freq = atof(buffer + 80) * 1000.0;
    if (freq2band(ptr->freq) == BANDINDEX_OOB) {
	ptr->freq = 0.;
    }

    return ptr;
}


/** free qso record pointed to by ptr */
void free_qso(struct qso_t *ptr) {

    if (ptr != NULL) {
	g_free(ptr->comment);
	g_free(ptr->logline);
	g_free(ptr->call);
	g_free(ptr->mult1_value);
	g_free(ptr->callupdate);
	g_free(ptr->normalized_comment);
	g_free(ptr->section);
	g_free(ptr);
    }
}


static void qso_free(gpointer data) {
    free_qso((struct qso_t *) data);
}


void free_qso_array() {
   if (qso_array != NULL) {
	g_ptr_array_free(qso_array, TRUE);
	qso_array = NULL;
   }
}

void init_qso_array() {
    free_qso_array();
    qso_array = g_ptr_array_new_with_free_func(qso_free);
}

