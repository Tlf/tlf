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
	 *        Read sent QTC QSO's
	 *
	 *--------------------------------------------------------------*/

#include "readqtccalls.h"
#include "get_time.h"
#include "tlf.h"
#include "globalvars.h"
#include <glib.h>

int readqtccalls()
{
    int s = 0;
    char inputbuffer[160];
    FILE *fp;
    char temps[30];
    int tempi;

    clear();
    mvprintw(4, 0, "Reading QTC sent logfile...\n");
    refreshp();

    /* set all flags to 0 */
    for (s = 0; s < MAX_QSOS; s++) {
	qsoflags_for_qtc[s] = 0;
    }

    if ((fp = fopen(QTC_SENT_LOG, "r")) == NULL) {
	mvprintw(5, 0, "Error opening QTC sent logfile.\n");
	refreshp();
	sleep(2);
	return -1;
    }

    while (fgets(inputbuffer, 90, fp) != NULL) {
	s++;
	strncpy(temps, inputbuffer+1, 4);	// serial
	tempi = atoi(temps);
	if (tempi > nr_qtcsent) {
	    nr_qtcsent = tempi;
	}

	strncpy(temps, inputbuffer+6, 4);	// qso nr in qso list
	tempi = atoi(temps);
	qsoflags_for_qtc[tempi] = 1;
	if (tempi > next_qtc_qso) {
	    next_qtc_qso = tempi+1;
	}
    }
    fclose(fp);
    return s;
}
