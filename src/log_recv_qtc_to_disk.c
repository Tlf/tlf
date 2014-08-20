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

    Log QSO to disk the received QTC line, do all necessary actions to clear qtc store

------------------------------------------------------------------------*/

#include "globalvars.h"
#include "get_time.h"
#include "log_recv_qtc_to_disk.h"
#include "qtcutil.h"

extern int trx_control;
extern float freq;
extern int logfrequency;

int log_recv_qtc_to_disk(int qsonr)
{
    char qtclogline[80], temp[20];
    int qpos = 0, i, tempi;

    static char time_buf[80];
    char khz[5] = " 000";

    for(i=0; i<10; i++) {

	if (strlen(qtcreclist.qtclines[i].time) == 4 &&
	    strlen(qtcreclist.qtclines[i].callsign) > 0 &&
	    strlen(qtcreclist.qtclines[i].serial) > 0) { // all fields are filled
	    for(qpos=0; qpos<80; qpos++) {
		qtclogline[qpos] = 32;
	    }

	    qpos = 0;
	    // QTC:  3799 PH 2003-03-23 0711 YB1AQS        001/10     DL8WPX        0330 DL6RAI        1021
	    // QTC: 21086 RY 2001-11-10 0759 HA3LI           1/10     YB1AQS        0003 KB3TS          003

	    sprintf(temp, "%3s", band[bandinx]);
	    if (trxmode == CWMODE) {
		strcat(temp, "CW ");
	    }
	    else if (trxmode == SSBMODE) {
		strcat(temp, "SSB");
	    }
	    else {
		strcat(temp, "DIG");
	    }
	    strncpy(qtclogline, temp, strlen(temp));

	    qpos = strlen(temp);
	    sprintf(temp, "%04d", qsonr);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos += strlen(temp);

	    get_time();
	    strftime(time_buf, 60, " %d-%b-%y %H:%M ", time_ptr);

	    strncpy(qtclogline+qpos, time_buf, strlen(time_buf));
	    qpos+=strlen(time_buf);

	    if (logfrequency == 1 &&
		trx_control == 1) {
		sprintf(khz, " %3d", ((int)freq)%1000);	// show freq.
		strncpy(qtclogline+qpos, khz, strlen(khz));
		qpos += strlen(khz);
	    }

	    if (lan_active == 1) {
		qtclogline[qpos++] = thisnode;	// set node ID...
	    } else {
		qtclogline[qpos++] = ' ';
	    }
	    qtclogline[qpos++] = ' ';

	    sprintf(temp, "%-14s", qtcreclist.callsign);

	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d", qtcreclist.serial);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d", qtcreclist.count);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %s", qtcreclist.qtclines[i].time);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %-14s", qtcreclist.qtclines[i].callsign);

	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    tempi = atoi(qtcreclist.qtclines[i].serial);
	    if(tempi < 1000) {
		sprintf(temp, "  %03d", tempi);
	    }
	    else {
		sprintf(temp, " %04d", tempi);
	    }
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    qtclogline[qpos] = '\0';

	    store_recv_qtc(qtclogline);

	    // send qtc to other nodes......
	    if (lan_active == 1) {
	      send_lan_message(QTCRENTRY, qtclogline);
	    }
	}
    }
    for(i=0; i<10; i++) {
	qtcreclist.qtclines[i].time[0] = '\0';
	qtcreclist.qtclines[i].callsign[0] = '\0';
	qtcreclist.qtclines[i].serial[0] = '\0';
	qtcreclist.qtclines[i].status = 0;
	qtcreclist.qtclines[i].confirmed = 0;
    }

    qtcreclist.count = 0;
    qtcreclist.serial = 0;
    qtcreclist.confirmed = 0;
    qtcreclist.sentcfmall = 0;
    qtcreclist.callsign[0] = '\0';

    return (0);
}

int store_recv_qtc(char *loglineptr)
{
	FILE *fp;
	int i;
	char callsign[15];

	if  ( (fp = fopen(QTC_RECV_LOG, "a"))  == NULL){
		fprintf(stdout,  "log_recv_qtc_to_disk.c: Error opening file.\n");
		endwin();
		exit(1);
	}
	for (i=strlen(loglineptr)-1; loglineptr[i] == ' '; i--);
	loglineptr[i+1] = '\n';
	loglineptr[i+2] = '\0';
	fputs  (loglineptr, fp);
	total++;

	fclose(fp);
	parse_qtcline(loglineptr, callsign, RECV);
	qtc_inc(callsign, RECV);

	return(0);
}