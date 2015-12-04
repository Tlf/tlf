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

#include <curses.h>

#include "lancode.h"
#include "qtc_log.h"
#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h


extern int trx_control;
extern float freq;

int add_to_qtcline(char *line, char *toadd, int pos) {
    int len = strlen(toadd);

    strncpy(line + pos, toadd, len);
    return pos + len;
}


int log_recv_qtc_to_disk(int qsonr)
{
    char qtclogline[100], temp[80];
    int qpos = 0, i, tempi;

    for(i=0; i<10; i++) {

	if (strlen(qtcreclist.qtclines[i].time) == 4 &&
	    strlen(qtcreclist.qtclines[i].callsign) > 0 &&
	    strlen(qtcreclist.qtclines[i].serial) > 0) { // all fields are filled
	    memset(qtclogline, sizeof(qtclogline)/sizeof(qtclogline[0]), ' ');
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
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, "%04d", qsonr);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %s ", qtcreclist.qtclines[i].receivedtime);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    if (lan_active == 1) {
		qtclogline[qpos++] = thisnode;	// set node ID...
	    } else {
		qtclogline[qpos++] = ' ';
	    }
	    qtclogline[qpos++] = ' ';

	    sprintf(temp, "%-14s", qtcreclist.callsign);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %04d", qtcreclist.serial);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %04d", qtcreclist.count);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %s", qtcreclist.qtclines[i].time);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %-14s", qtcreclist.qtclines[i].callsign);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    tempi = atoi(qtcreclist.qtclines[i].serial);
	    if(tempi < 1000) {
		sprintf(temp, "  %03d    ", tempi);
	    }
	    else {
		sprintf(temp, " %04d    ", tempi);
	    }
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    if (trx_control == 1) {
		snprintf(temp, 8, "%7.1f", freq);
	    }
	    else {
		snprintf(temp, 8, "      *");
	    }
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    qtclogline[qpos] = '\n';
	    qtclogline[qpos + 1] = '\0';

	    store_recv_qtc(qtclogline);

	    // send qtc to other nodes......
	    if (lan_active == 1) {
	      send_lan_message(QTCRENTRY, qtclogline);
	    }
	}
    }

    /* clear all line infos */
    for(i=0; i<10; i++) {
	qtcreclist.qtclines[i].time[0] = '\0';
	qtcreclist.qtclines[i].callsign[0] = '\0';
	qtcreclist.qtclines[i].serial[0] = '\0';
	qtcreclist.qtclines[i].status = 0;
	qtcreclist.qtclines[i].confirmed = 0;
	qtcreclist.qtclines[i].receivedtime[0] = '\0';
    }
    for(i=0; i<QTC_RY_LINE_NR; i++) {
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


int log_sent_qtc_to_disk(int qsonr)
{
    char qtclogline[100], temp[80];
    int qpos = 0, i;

    for(i=0; i<10; i++) {
	if (qtclist.qtclines[i].saved == 0 && qtclist.qtclines[i].flag == 1 && qtclist.qtclines[i].sent == 1) { // not saved and marked for sent

	    memset(qtclogline, sizeof(qtclogline)/sizeof(qtclogline[0]), ' ');
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
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, "%04d", qsonr);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %04d", qtclist.qtclines[i].qsoline+1);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %s ", qtclist.qtclines[i].senttime);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    if (lan_active == 1) {
		qtclogline[qpos++] = thisnode;	// set node ID...
	    } else {
		qtclogline[qpos++] = ' ';
	    }
	    qtclogline[qpos++] = ' ';

	    sprintf(temp, "%-14s", qtclist.callsign);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %04d", qtclist.serial);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    sprintf(temp, " %04d ", qtclist.count);
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    strcpy(qtclogline+qpos, qtclist.qtclines[i].qtc);
	    qpos+=strlen(qtclist.qtclines[i].qtc);

	    qpos = add_to_qtcline(qtclogline, "    ", qpos);

	    if (trx_control == 1) {
		snprintf(temp, 8, "%7.1f", freq);
	    }
	    else {
		snprintf(temp, 8, "      *");
	    }
	    qpos = add_to_qtcline(qtclogline, temp, qpos);

	    qtclogline[qpos] = '\n';
	    qtclogline[qpos + 1] = '\0';

	    store_sent_qtc(qtclogline);

	    // send qtc to other nodes......
	    if (lan_active == 1) {
	      send_lan_message(QTCSENTRY, qtclogline);
	    }
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

    return (0);
}


/* common code to store sent or received QTC's */
void store_qtc(char *loglineptr, int direction)
{
	FILE *fp;
	char callsign[15];
	char filename[80];
	char temps[15];
	int tempi;

	if (direction == SEND)
	    strcpy(filename, QTC_SENT_LOG);
	else if (direction == RECV)
	    strcpy(filename, QTC_RECV_LOG);
	else
	    return;

	if  ( (fp = fopen(filename, "a"))  == NULL){
		fprintf(stdout,  "Error opening file: %s\n", filename);
		endwin();
		exit(1);
	}
	fputs  (loglineptr, fp);
	fclose(fp);

	total++;

	if (direction == SEND) {
		/* find maximum sent QTC block serial */
		g_strlcpy(temps, loglineptr+50, 5);    // get serial of qtc block
		tempi = atoi(temps);
		if (tempi > nr_qtcsent) {
		      nr_qtcsent = tempi;
		}

		/* mark corresponding qso line as used for QTC */
		g_strlcpy(temps, loglineptr+12, 5);    // qso nr in qso list
		tempi = atoi(temps)-1;
		qsoflags_for_qtc[tempi] = 1;

		/* find first unused QSO number for QTCs */
		if (tempi-1 == next_qtc_qso) {
		    next_qtc_qso = tempi;
		}
	}
	/* remember callsign, build number of sent or received QTC's */
	parse_qtcline(loglineptr, callsign, direction);
	qtc_inc(callsign, direction);
}

void store_sent_qtc(char *loglineptr)
{
	store_qtc(loglineptr, SEND);
}

void store_recv_qtc(char *loglineptr)
{
	store_qtc(loglineptr, RECV);
}


