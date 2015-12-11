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
	/* ------------------------------------------------------------
	 *        Read sent QTC QSO's
	 *
	 *--------------------------------------------------------------*/


#include <config.h>		// Includes found ncurses.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h


int qtcdirection = 0;
int next_qtc_qso;
int qsoflags_for_qtc[MAX_QSOS];
int nr_qsoflags_for_qtc;
int nr_qtcsent = 0;

int readqtccalls()
{
    int s = 0;
    char inputbuffer[160];
    FILE *fp;
    char temps[30], callsign[15];
    int tempi;
    int last_qtc = 0;
    int i;

    clear();

    qtc_init();

    nr_qsoflags_for_qtc = nr_qsos;

    if (qtcdirection & SEND) {
	mvprintw(4, 0, "Reading QTC sent logfile...\n");
	refreshp();

	/* mark all qso lines as not used for QTC */
	for (s = 0; s < MAX_QSOS; s++) {
	    qsoflags_for_qtc[s] = 0;
	}

	if ((fp = fopen(QTC_SENT_LOG, "r")) == NULL) {
	    mvprintw(5, 0, "Error opening QTC sent logfile.\n");
	    refreshp();
	    sleep(2);
	    return -1;
	}

	while (fgets(inputbuffer, 100, fp) != NULL) {
	    s++;

	    /* find maximum sent QTC block serial */
	    g_strlcpy(temps, inputbuffer+50, 5);    // get serial of QTC block
	    tempi = atoi(temps);
	    if (tempi > nr_qtcsent) {
		nr_qtcsent = tempi;
	    }

	    /* mark corresponding qso line as used for QTC */
	    g_strlcpy(temps, inputbuffer+12, 5);    // qso nr in qso list
	    tempi = atoi(temps)-1;
	    qsoflags_for_qtc[tempi] = 1;

	    /* remember callsign, build number of sent QTC's */
	    parse_qtcline(inputbuffer, callsign, SEND);
	    qtc_inc(callsign, SEND);

	    total++;			/* add one point per QTC */

	    /* find first unused QSO number for QTCs */
	    if (tempi > last_qtc) {
		last_qtc = tempi;
	    }
	}

	next_qtc_qso = last_qtc;

	/* find first QSO which was not used for QTC yet */
	for(i=0; i<last_qtc; i++) {
	    if (qsoflags_for_qtc[i] == 0) {
		next_qtc_qso = i;
		break;
	    }
	}

	fclose(fp);
    }

    if (qtcdirection & RECV) {
	mvprintw(4, 0, "Reading QTC recv logfile...\n");
	refreshp();

	if ((fp = fopen(QTC_RECV_LOG, "r")) == NULL) {
	    mvprintw(5, 0, "Error opening QTC received logfile.\n");
	    refreshp();
	    sleep(2);
	    return -1;
	}

	while (fgets(inputbuffer, 100, fp) != NULL) {

	    /* remember callsign, build number of received QTC's */
	    parse_qtcline(inputbuffer, callsign, RECV);
	    qtc_inc(callsign, RECV);

	    total++;			/* add one point per QTC */
	}

	fclose(fp);
    }

    if (strlen(qtc_cap_calls) > 0) {
	mvprintw(4, 0, "Reading QTC callsigns file...\n");
	refreshp();

	if ((fp = fopen(qtc_cap_calls, "r")) == NULL) {
	    mvprintw(5, 0, "Error opening QTC callsigns file.\n");
	    refreshp();
	    sleep(2);
	    return -1;
	}

	while (fgets(inputbuffer, 100, fp) != NULL) {

	    /* remember callsign, mark it as QTC capable, based on eg. last years */
	    qtc_inc(g_strstrip(inputbuffer), QTC_CAP);
	}

	fclose(fp);
    }

    mvprintw(4, 0, "Reading QTC meta logfile...\n");
    refreshp();

    if ((fp = fopen(QTC_META_LOG, "r")) == NULL) {
	mvprintw(5, 0, "QTC meta logfile missing, skipping this step.\n");
	refreshp();
    }
    else {
	while (fgets(inputbuffer, 100, fp) != NULL) {
	    /* remember callsign, set marked QTC states */
	    parse_qtc_flagline(inputbuffer);
	}
	fclose(fp);
    }

    return s;
}
