/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013-2014      Ervin Hegedüs - HA2OS <airween@gmail.com>
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
 *        Generate QTC list to send
 *
 *--------------------------------------------------------------*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "qtcvars.h"		// Includes globalvars.h


void genqtcline(char *qtc, char *qsoline);

/** generate list of QTCs to send
 *
 * \param callsign - the call of the station we send the QTCs to
 * \param nrofqtc  - maximum number of lines to send
 * \return  number of actual lines in the qtclist
 */
int genqtclist(char *callsign, int nrofqtc) {

    int qtclistlen;
    int s = 0, i = 0;

    qtclistlen = QTC_LINES;
    if (nrofqtc >= 0 && nrofqtc < QTC_LINES) {
	qtclistlen = nrofqtc;
    }

    /* initialize qtclist */
    qtclist.serial = nr_qtcsent + 1;
    qtclist.marked = 0;
    qtclist.totalsent = 0;
    qtclist.count = 0;
    g_strlcpy(qtclist.callsign, callsign, sizeof(qtclist.callsign));
    for (s = 0; s < qtclistlen; s++) {
	qtclist.qtclines[s].qtc[0] = '\0';
	qtclist.qtclines[s].flag = 0;
	qtclist.qtclines[s].saved = 0;
	qtclist.qtclines[s].sent = 0;
	qtclist.qtclines[s].senttime[0] = '\0';
    }

    s = next_qtc_qso;

    while (qtclist.count < qtclistlen && s < nr_qsos) {
	if (strlen(callsign) == 0 ||
		strncmp(qsos[s] + 29, callsign, strlen(callsign)) != 0) {
	    /* exclude current callsign */

	    if (qsoflags_for_qtc[s] == 0) {
		/* qso line not yet used for QTC */

		genqtcline(qtclist.qtclines[i].qtc, qsos[s]);

		if (trxmode == DIGIMODE) {
		    qtclist.qtclines[i].flag = 1;
		    qtclist.marked++;
		} else {
		    if (i == 0) {
			qtclist.qtclines[i].flag = 1;
			qtclist.marked++;
		    } else {
			qtclist.qtclines[i].flag = 0;
		    }
		}
		/* remember number of the corresponding QSO line */
		qtclist.qtclines[i].qsoline = s;

		qtclist.count++;
		i++;	/* next qtcline */
	    }
	}
	s++;		/* try next qso */
    }

    return qtclist.count;
}

void genqtcline(char *qtc, char *qsoline) {
    int i, qpos, nr;
    char tstring[5];

    /* pick out qso time hhmm */
    strncpy(qtc, qsoline + 17, 2);
    strncpy(qtc + 2, qsoline + 20, 2);
    qtc[4] = ' ';

    /* copy callsign */
    qpos = 5;
    for (i = 29; qsoline[i] != ' '; i++) {
	qtc[qpos] = qsoline[i];
	qpos++;
    }
    while (qpos < 20) {
	qtc[qpos] = ' ';
	qpos++;
    }

    /* add finally 3 or 4 digit exchange */
    g_strlcpy(tstring, qsoline + 54, sizeof(tstring));
    nr = atoi(tstring);
    // 3 digit
    if ((nr >= 0) && (nr < 1000)) {
	sprintf(tstring, "%03d ", nr);
    }
    // 4 digit
    else if ((nr >= 0) && (nr < 10000)) {
	sprintf(tstring, "%d", nr);
    } else {
	// ignore all other exchange values
	strcpy(tstring, "    ");
    }

    strcpy(qtc + qpos, tstring);
    qpos += 4;
    qtc[qpos] = '\0';
}
