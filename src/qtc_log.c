/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013 Ervin Hegedüs - HA2OS <airween@gmail.com>
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
/*------------------------------------------------------------------------

    Log received or sent QTC lines to disk, clear the qtc list

------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>

#include "lancode.h"
#include "qtc_log.h"
#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "tlf_curses.h"

extern int trx_control;
extern freq_t freq;

int log_recv_qtc_to_disk(int qsonr) {

    int i;
    struct read_qtc_t qtc_line;

    for (i = 0; i < 10; i++) {

	if (strlen(qtcreclist.qtclines[i].time) == 4 &&
		strlen(qtcreclist.qtclines[i].callsign) > 0 &&
		strlen(qtcreclist.qtclines[i].serial) > 0) { // all fields are filled

	    qtc_line.direction = RECV;
	    strcpy(qtc_line.band, band[bandinx]);
	    if (trxmode == CWMODE) {
		strcpy(qtc_line.mode, "CW ");
	    } else if (trxmode == SSBMODE) {
		strcpy(qtc_line.mode, "PH ");
	    } else {
		strcpy(qtc_line.mode, "DIG");
	    }

	    strncpy(qtc_line.date, qtcreclist.qtclines[i].receivedtime, 9);
	    qtc_line.date[9] = '\0';
	    strncpy(qtc_line.time, qtcreclist.qtclines[i].receivedtime + 10, 5);
	    qtc_line.time[5] = '\0';

	    qtc_line.qsonr = qsonr;

	    strcpy(qtc_line.call, qtcreclist.callsign);

	    qtc_line.qtchead_serial = qtcreclist.serial;
	    qtc_line.qtchead_count = qtcreclist.count;
	    strcpy(qtc_line.qtc_time, qtcreclist.qtclines[i].time);
	    qtc_line.qtc_time[4] = '\0';
	    strcpy(qtc_line.qtc_call, qtcreclist.qtclines[i].callsign);
	    qtc_line.qtc_call[strlen(qtcreclist.qtclines[i].callsign)] = '\0';
	    qtc_line.qtc_serial = atoi(qtcreclist.qtclines[i].serial);

	    if (trx_control == 1) {
		qtc_line.freq = freq;
	    } else {
		qtc_line.freq = 0;
	    }

	    qtc_line.callpos = 0;
	    make_qtc_logline(qtc_line, QTC_RECV_LOG);

	}
    }

    /* clear all line infos */
    for (i = 0; i < 10; i++) {
	qtcreclist.qtclines[i].time[0] = '\0';
	qtcreclist.qtclines[i].callsign[0] = '\0';
	qtcreclist.qtclines[i].serial[0] = '\0';
	qtcreclist.qtclines[i].status = 0;
	qtcreclist.qtclines[i].confirmed = 0;
	qtcreclist.qtclines[i].receivedtime[0] = '\0';
    }
    for (i = 0; i < QTC_RY_LINE_NR; i++) {
	qtc_ry_lines[i].content[0] = '\0';
	qtc_ry_lines[i].attr = 0;
    }
    qtc_ry_currline = 0;
    qtc_ry_copied = 0;

    /* clear record list */
    qtcreclist.count = 0;
    qtcreclist.serial = 0;
    qtcreclist.confirmed = 0;
    qtcreclist.sentcfmall = 0;
    qtcreclist.callsign[0] = '\0';

    return (0);
}


int log_sent_qtc_to_disk(int qsonr) {

    int i;
    char *tempstrp;
    struct read_qtc_t qtc_line;

    for (i = 0; i < 10; i++) {
	if (qtclist.qtclines[i].saved == 0 && qtclist.qtclines[i].flag == 1
		&& qtclist.qtclines[i].sent == 1) { // not saved and marked for sent

	    qtc_line.direction = SEND;
	    strcpy(qtc_line.band, band[bandinx]);
	    if (trxmode == CWMODE) {
		strcpy(qtc_line.mode, "CW ");
	    } else if (trxmode == SSBMODE) {
		strcpy(qtc_line.mode, "PH ");
	    } else {
		strcpy(qtc_line.mode, "DIG");
	    }

	    strncpy(qtc_line.date, qtclist.qtclines[i].senttime, 9);
	    qtc_line.date[9] = '\0';
	    strncpy(qtc_line.time, qtclist.qtclines[i].senttime + 10, 5);
	    qtc_line.time[5] = '\0';

	    qtc_line.qsonr = qsonr;

	    strcpy(qtc_line.call, qtclist.callsign);

	    qtc_line.qtchead_serial = qtclist.serial;
	    qtc_line.qtchead_count = qtclist.count;

	    strcpy(qtc_line.qtcstr, qtclist.qtclines[i].qtc);
	    tempstrp = strtok(qtc_line.qtcstr, " ");
	    if (tempstrp != NULL) {
		strcpy(qtc_line.qtc_time, tempstrp);
	    } else {
		strcpy(qtc_line.qtc_time, "----");
	    }

	    tempstrp = strtok(NULL, " ");
	    g_strchomp(tempstrp);
	    if (tempstrp != NULL) {
		strcpy(qtc_line.qtc_call, tempstrp);
	    } else {
		strcpy(qtc_line.qtc_call, "-------------");
	    }

	    tempstrp = strtok(NULL, " ");
	    g_strchomp(tempstrp);
	    if (tempstrp != NULL) {
		qtc_line.qtc_serial = atoi(tempstrp);
	    } else {
		qtc_line.qtc_serial = 0;
	    }

	    qtc_line.callpos = qtclist.qtclines[i].qsoline + 1;

	    if (trx_control == 1) {
		qtc_line.freq = freq;
	    } else {
		qtc_line.freq = 0;
	    }

	    make_qtc_logline(qtc_line, QTC_SENT_LOG);

	}
    }

    for (i = 0; i < 10; i++) {
	qtclist.qtclines[i].qtc[0] = '\0';
	qtclist.qtclines[i].flag = 0;
	qtclist.qtclines[i].saved = 0;
	qtclist.qtclines[i].sent = 0;
	qtclist.qtclines[i].senttime[0] = '\0';
    }

    qtclist.count = 0;
    qtclist.marked = 0;
    qtclist.totalsent = 0;

    return (0);
}


/* common code to store sent or received QTC's */
void store_qtc(char *loglineptr, int direction, char *filename) {

    FILE *fp;
    char callsign[15];
    char temps[15];
    int tempi;

    if ((fp = fopen(filename, "a"))  == NULL) {
	fprintf(stdout,  "Error opening file: %s\n", filename);
	endwin();
	exit(1);
    }
    fputs(loglineptr, fp);
    fclose(fp);

    total++;
    if (direction == SEND) {
	/* find maximum sent QTC block serial */
	g_strlcpy(temps, loglineptr + 50, 5);  // get serial of qtc block
	tempi = atoi(temps);
	if (tempi > nr_qtcsent) {
	    nr_qtcsent = tempi;
	}

	/* mark corresponding qso line as used for QTC */
	g_strlcpy(temps, loglineptr + 12, 5);  // qso nr in qso list
	tempi = atoi(temps) - 1;
	qsoflags_for_qtc[tempi] = 1;

	/* find first unused QSO number for QTCs */
	if (tempi == next_qtc_qso && tempi < MAX_QSOS) {
	    while (qsoflags_for_qtc[tempi++] == 1) {
		if (tempi == MAX_QSOS)
		    break;
		next_qtc_qso = tempi;
	    }
	}
    }
    /* remember callsign, build number of sent or received QTC's */
    parse_qtcline(loglineptr, callsign, direction);
    qtc_inc(callsign, direction);
}

void make_qtc_logline(struct read_qtc_t qtc_line, char *fname) {

    char nodemark = ' ';
    char qtclogline[120];
    char padding[2] = " ";

    if (lan_active) {
	nodemark = thisnode;
    }

    memset(qtclogline, '\0', 120);

    if (qtc_line.qtc_serial >= 1000) {
	padding[0] = '\0';
    }

    if (qtc_line.direction == RECV) {
	sprintf(qtclogline,
		"%s%s %04d %s %s %c %-14s %04d %04d %s %-15s%s%03d    %7.1f\n", qtc_line.band,
		qtc_line.mode, qtc_line.qsonr,
		qtc_line.date, qtc_line.time, nodemark, qtc_line.call, qtc_line.qtchead_serial,
		qtc_line.qtchead_count,
		qtc_line.qtc_time, qtc_line.qtc_call, padding, qtc_line.qtc_serial,
		qtc_line.freq / 1000.0);
	store_qtc(qtclogline, qtc_line.direction, fname);
	if (lan_active) {
	    send_lan_message(QTCRENTRY, qtclogline);
	}
    }
    if (qtc_line.direction == SEND) {
	sprintf(qtclogline,
		"%s%s %04d %04d %s %s %c %-14s %04d %04d %s %-14s%s%03d    %7.1f\n",
		qtc_line.band, qtc_line.mode, qtc_line.qsonr,
		qtc_line.callpos, qtc_line.date, qtc_line.time, nodemark, qtc_line.call,
		qtc_line.qtchead_serial, qtc_line.qtchead_count,
		qtc_line.qtc_time, qtc_line.qtc_call, padding, qtc_line.qtc_serial,
		qtc_line.freq / 1000.0);
	store_qtc(qtclogline, qtc_line.direction, fname);
	if (lan_active) {
	    send_lan_message(QTCSENTRY, qtclogline);
	}
    }
}


