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


#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "err_utils.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "log_utils.h"
#include "ignore_unused.h"
#include "qsonr_to_str.h"
#include "tlf_curses.h"

/** find out highest used serial number and prepare next one in qsonum
 * and qsonumstr
 *
 * \Todo needs to be cleaned up, to make more clear how it works, especially
 * if LAN is active*/
void get_next_serial(void) {
    int mm;

    mm = qsonum - 1;

    if (!log_is_comment(logline4)) {
	strncpy(qsonrstr, logline4 + 23, 4);
	mm = atoi(qsonrstr);
    }
    if (!log_is_comment(logline3)) {
	if (atoi(logline3 + 23) > mm) {
	    strncpy(qsonrstr, logline3 + 23, 4);
	    mm = atoi(qsonrstr);
	}
    }
    if (!log_is_comment(logline2)) {
	if (atoi(logline2 + 23) > mm) {
	    strncpy(qsonrstr, logline2 + 23, 4);
	    mm = atoi(qsonrstr);
	}
    }
    if (!log_is_comment(logline1)) {
	if (atoi(logline1 + 23) > mm) {
	    strncpy(qsonrstr, logline1 + 23, 4);
	    mm = atoi(qsonrstr);
	}
    }

    if ((lan_active == 1) && (exchange_serial == 1)) {

	if (lan_mutex == 2) {	/* last stored message is from lan */

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

    if (!log_is_comment(logline4))
	qsonum++;
    else
	qsonum = mm + 1;

    qsonr_to_str();
}

/** read the last 5  log lines and set the next qso number */
void scroll_log(void) {

    char inputbuffer[800];
    FILE *fp;

    if ((fp = fopen(logfile, "r")) == NULL) {
	TLF_LOG_ERR("Error opening logfile.");
    }

    for (int i = 5; i >= 1; i--) {

	if (fseek(fp, -1L * i * LOGLINELEN, SEEK_END) == 0)
	    IGNORE(fgets(inputbuffer, 90, fp));
	else
	    strcpy(inputbuffer,
		   "                                                                                ");

	if (strlen(inputbuffer) <= 10)	/* log repair */
	    IGNORE(fgets(inputbuffer, 90, fp));;

	g_strlcpy(logline_edit[5 - i], inputbuffer, 81);
    }

    fclose(fp);

    get_next_serial();
}
