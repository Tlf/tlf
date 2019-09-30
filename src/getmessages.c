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
 *        Get messages  from  -paras file
 *        and  gets  the last  5 qso records for  display
 *        also gets the nr of the last qso from  the logfile
 *--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "checklogfile.h"
#include "dxcc.h"
#include "getctydata.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "qsonr_to_str.h"
#include "ignore_unused.h"
#include "tlf_curses.h"


/* get countrynumber, QTH, CQ zone and continent for myself */
void getstationinfo() {
    extern char call[];
    extern int mycountrynr;
    extern char mycqzone[];
    extern char mycontinent[];
    extern double QTH_Lat;
    extern double QTH_Long;

    dxcc_data *mydx;

    mycountrynr = getctydata(call);	/* whoami? */
    mydx = dxcc_by_index(mycountrynr);

    sprintf(mycqzone, "%02d", mydx -> cq);
    strcpy(mycontinent, mydx->continent);
    QTH_Lat = mydx->lat; 	/* whereami? */
    QTH_Long = mydx->lon;
}


void getmessages(void) {

    extern char call[];
    extern char mycqzone[];
    extern char mycontinent[];
    extern char logfile[];
    extern int qsonum;
    extern char qsonrstr[];
    extern char backgrnd_str[];

    FILE *fp;

    int i, ii;
    char logline[5][82];

    printw("\n     Call = ");
    printw(call);

    getstationinfo();

    printw("     My Zone = ");
    printw(mycqzone);

    printw("     My Continent = ");
    printw(mycontinent);

    printw("\n\n");
    refreshp();

    if ((fp = fopen(logfile, "r")) == NULL) {
	printw("\nError opening logfile.\nExiting...\n");
	refreshp();
	sleep(5);
	endwin();
	exit(1);
    }

    for (i = 5; i >= 1; i--) {

	ii = 5 - i;

	if (fseek(fp, -1L * i * LOGLINELEN, SEEK_END) == 0) {
	    IGNORE(fgets(logline[ii], 85, fp));;
	} else {
	    strncpy(logline[ii], backgrnd_str, 81);
	}

	logline[ii][80] = '\0';
	logline[ii][78] = 32;
	logline[ii][79] = 32;
    }

    fclose(fp);


    strncpy(qsonrstr, logline[4] + 23, 4);
    qsonrstr[4] = '\0';

    qsonum = atoi(qsonrstr) + 1;

    if (qsonum == 1) {
	strncpy(qsonrstr, logline[3] + 23, 4);
	qsonrstr[4] = '\0';
	qsonum = atoi(qsonrstr) + 1;
	qsonr_to_str();
    }

    if (strlen(logline[0]) >= 75)
	strncpy(logline0, logline[0], 80);
    else
	strcpy(logline0, backgrnd_str);

    if (strlen(logline[1]) >= 75)
	strncpy(logline1, logline[1], 80);
    else
	strcpy(logline1, backgrnd_str);

    if (strlen(logline[2]) >= 75)
	strncpy(logline2, logline[2], 80);
    else
	strcpy(logline2, backgrnd_str);

    if (strlen(logline[3]) >= 75)
	strncpy(logline3, logline[3], 80);
    else
	strcpy(logline3, backgrnd_str);

    if (strlen(logline[4]) >= 75)
	strncpy(logline4, logline[4], 80);
    else
	strcpy(logline4, backgrnd_str);
}
