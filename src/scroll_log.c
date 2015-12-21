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
/* ------------------------------------------------------------------------
*    scroll  the loglines of the keyer terminal 1 up
*
---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "globalvars.h"		// Includes glib.h and tlf.h
#include "qsonr_to_str.h"
#include "tlf_curses.h"


void scroll_log(void)
{
    char *rp;
    char inputbuffer[800];
    static int ii, kk;
    int mm;

    FILE *fp;

    if ((fp = fopen(logfile, "r")) == NULL) {

	mvprintw(24, 0, "Error opening logfile.\n");
	refreshp();
	sleep(2);
	exit(1);
    }
    for (ii = 5; ii >= 1; ii--) {

	inputbuffer[0] = '\0';

	if (fseek(fp, -1 * ii * LOGLINELEN, SEEK_END) == 0)
	    rp = fgets(inputbuffer, 90, fp);
	else
	    strcpy(inputbuffer,
		   "                                                                                ");

	kk = 5 - ii;

	if (strlen(inputbuffer) <= 10)	/* log repair */
	    rp = fgets(inputbuffer, 90, fp);

//              if (strlen(inputbuffer) != LOGLINELEN)
//                      strcat (inputbuffer, backgrnd_str);

	inputbuffer[80] = '\0';

	switch (kk) {
	case 0:{
		strncpy(logline0, inputbuffer, 80);
		logline0[80] = '\0';
		break;
	    }
	case 1:{
		strncpy(logline1, inputbuffer, 80);
		logline1[80] = '\0';
		break;
	    }

	case 2:{
		strncpy(logline2, inputbuffer, 80);
		logline2[80] = '\0';
		break;

	    }
	case 3:{
		strncpy(logline3, inputbuffer, 80);
		logline3[80] = '\0';
		break;

	    }
	case 4:{
		strncpy(logline4, inputbuffer, 80);
		logline4[80] = '\0';
		break;
	    }
	}

    }

    fclose(fp);

    mm = qsonum - 1;

    if (logline4[0] != ';') {
	strncpy(qsonrstr, logline4 + 23, 4);
	mm = atoi(qsonrstr);
    }
    if (logline3[0] != ';') {
	if (atoi(logline3 + 23) > mm) {
	    mm = atoi(logline3 + 23);
	    strncpy(qsonrstr, logline3 + 23, 4);
	}
    }
    if (logline2[0] != ';') {
	if (atoi(logline2 + 23) > mm) {
	    mm = atoi(logline2 + 23);
	    strncpy(qsonrstr, logline2 + 23, 4);
	}
    }
    if (logline1[0] != ';') {
	if (atoi(logline1 + 23) > mm) {
	    mm = atoi(logline1 + 23);
	    strncpy(qsonrstr, logline1 + 23, 4);
	}
    }

    if ((lan_active == 1) && (exchange_serial == 1)) {

	if (lan_mutex == 2) {	/* last mesagge from lan */

	    if (atoi(qsonrstr) <= highqsonr) {
		qsonum = highqsonr;
	    }
	} else {
	    qsonum = atoi(qsonrstr);
	    if (qsonum < highqsonr)
		qsonum = highqsonr;
	    if (highqsonr < qsonum)
		highqsonr = qsonum;
	}
    } else
	qsonum = atoi(qsonrstr);

    if (logline4[0] != ';')
	qsonum++;
    else
	qsonum = mm + 1;

//              if((qsonum > highqsonr) && (lan_mutex == 2)) highqsonr++;

    qsonr_to_str();

}
