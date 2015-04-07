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

    Log QSO to disk the sent QTC line, do all necessary actions to clear qtc store

------------------------------------------------------------------------*/

#include "globalvars.h"
#include "get_time.h"
#include "log_sent_qtc_to_disk.h"
#include "qtcutil.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tlf.h"
#include "lancode.h"
#include "qtcvars.h"

extern int trx_control;
extern float freq;
extern int logfrequency;

int log_sent_qtc_to_disk(int qsonr)
{
    char qtclogline[100], temp[20];
    int qpos = 0, i;
    int has_empty = 0;
    int last_qtc = 0;

    static char time_buf[80];
    char khz[9] = "";

    for(i=0; i<10; i++) {
	if (qtclist.qtclines[i].saved == 0 && qtclist.qtclines[i].flag == 1 && qtclist.qtclines[i].sent == 1) { // not saved and marked for sent
	    for(qpos=0; qpos<100; qpos++) {
		qtclogline[qpos] = 32;
	    }

	    qpos = 0;
	    // QTC:  3799 PH 2003-03-23 0711 YB1AQS        001/10     DL8WPX        0330 DL6RAI        1021
	    // QTC: 21086 RY 2001-11-10 0759 HA3LI           1/10     YB1AQS        0003 KB3TS          003

	    sprintf(temp, "%3s", band[bandinx]);
	    if (trxmode == CWMODE) {
		strcat(temp, "CW  ");
	    }
	    else if (trxmode == SSBMODE) {
		strcat(temp, "SSB ");
	    }
	    else {
		strcat(temp, "DIG ");
	    }
	    strncpy(qtclogline, temp, strlen(temp));

	    qpos = strlen(temp);
	    sprintf(temp, "%04d", qsonr);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos += strlen(temp);

	    sprintf(temp, " %04d", qtclist.qtclines[i].qsoline+1);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos += strlen(temp);

	    sprintf(time_buf, " %s ", qtclist.qtclines[i].senttime);
	    strncpy(qtclogline+qpos, time_buf, strlen(time_buf));
	    qpos+=strlen(time_buf);

	    if (lan_active == 1) {
		qtclogline[qpos++] = thisnode;	// set node ID...
	    } else {
		qtclogline[qpos++] = ' ';
	    }
	    qtclogline[qpos++] = ' ';

	    sprintf(temp, "%-14s", qtclist.callsign);

	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d", qtclist.serial);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d ", qtclist.count);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    strcpy(qtclogline+qpos, qtclist.qtclines[i].qtc);
	    qpos+=strlen(qtclist.qtclines[i].qtc);
	    strncpy(qtclogline+qpos, "    ", 4);
	    qpos += 4;

	    if (trx_control == 1) {
		snprintf(khz, 8, "%7.1f", freq);
	    }
	    else {
		snprintf(khz, 8, "      *");
	    }

	    strncpy(qtclogline+qpos, khz, strlen(khz));
	    qpos += strlen(khz);
	    qtclogline[qpos] = '\0';

	    store_sent_qtc(qtclogline);

	    // send qtc to other nodes......
	    if (lan_active == 1) {
	      send_lan_message(QTCSENTRY, qtclogline);
	    }

	    // mark qso as sent as qtc
	    qsoflags_for_qtc[qtclist.qtclines[i].qsoline] = 1;
	    if (qtclist.qtclines[i].qsoline > last_qtc) {
		last_qtc = qtclist.qtclines[i].qsoline;
	    }

	    // check if prev qso callsign is the current qtc window,
	    // and excluded from list; if true, set the next_qtc_qso to that
	    // else see below, the next qtc window pointer will set to
	    // next qso after the current window
	    if (qtclist.qtclines[i].qsoline > 0 && qsoflags_for_qtc[qtclist.qtclines[i].qsoline-1] == 0) {
		has_empty = 1;
		next_qtc_qso = qtclist.qtclines[i].qsoline-1;
	    }

	    // set next_qtc_qso pointer to next qso line,
	    // if the list is continous
	    if (has_empty == 0) {
		next_qtc_qso = qtclist.qtclines[i].qsoline+1;
	    }

	}
    }
    for(i=0; i<last_qtc; i++) {
	if (qsoflags_for_qtc[i] == 0) {
	    next_qtc_qso = i;
	    break;
	}
    }

    for(i=0; i<10; i++) {
	qtclist.qtclines[i].qtc[0] = '\0';
	qtclist.qtclines[i].flag = 0;
	qtclist.qtclines[i].saved = 0;
	qtclist.qtclines[i].sent = 0;
	qtclist.qtclines[i].senttime[0] = '\0';

    }

    qtclist.count = 0;
    qtclist.marked = 0;
    qtclist.totalsent = 0;
    nr_qtcsent++;

    return (0);
}

void store_sent_qtc(char *loglineptr)
{
	FILE *fp;
	int i;
	char callsign[15];

	if  ( (fp = fopen(QTC_SENT_LOG, "a"))  == NULL){
		fprintf(stdout,  "log_sent_qtc_to_disk.c: Error opening file.\n");
		endwin();
		exit(1);
	}
	for (i=strlen(loglineptr)-1; loglineptr[i] == ' '; i--);
	loglineptr[i+1] = '\n';
	loglineptr[i+2] = '\0';
	fputs  (loglineptr, fp);
	total++;

	fclose(fp);
	parse_qtcline(loglineptr, callsign, SEND);
	qtc_inc(callsign, SEND);

}
