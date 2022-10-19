/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2021           Thomas Beierlein <dl1jbe@darc.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/* ------------------------------------------------------------------------
*    scroll  the loglines of the terminal 1 up
*
---------------------------------------------------------------------------*/

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "err_utils.h"
#include "ui_utils.h"
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
	memcpy(current_qso_values.qsonrstr, logline4 + 23, 4);
	current_qso_values.qsonrstr[4] = '\0';
	mm = atoi(current_qso_values.qsonrstr);
    }
    if (!log_is_comment(logline3)) {
	if (atoi(logline3 + 23) > mm) {
	    memcpy(current_qso_values.qsonrstr, logline3 + 23, 4);
	    current_qso_values.qsonrstr[4] = '\0';
	    mm = atoi(current_qso_values.qsonrstr);
	}
    }
    if (!log_is_comment(logline2)) {
	if (atoi(logline2 + 23) > mm) {
	    memcpy(current_qso_values.qsonrstr, logline2 + 23, 4);
	    current_qso_values.qsonrstr[4] = '\0';
	    mm = atoi(current_qso_values.qsonrstr);
	}
    }
    if (!log_is_comment(logline1)) {
	if (atoi(logline1 + 23) > mm) {
	    memcpy(current_qso_values.qsonrstr, logline1 + 23, 4);
	    current_qso_values.qsonrstr[4] = '\0';
	    mm = atoi(current_qso_values.qsonrstr);
	}
    }

    if (lan_active == 1 && (contest->exchange_serial)) {

	if (lan_mutex == 2) {	/* last stored message is from lan */

	    if (atoi(current_qso_values.qsonrstr) <= highqsonr) {
		qsonum = highqsonr;
	    }
	} else {
	    qsonum = atoi(current_qso_values.qsonrstr);
	    if (qsonum < highqsonr)
		qsonum = highqsonr;
	    if (highqsonr < qsonum)
		highqsonr = qsonum;
	}
    } else
	qsonum = atoi(current_qso_values.qsonrstr);

    if (!log_is_comment(logline4))
	qsonum++;
    else
	qsonum = mm + 1;

    qsonr_to_str(current_qso_values.qsonrstr, qsonum);
}

#define LINELEN 80

/** read the last 5 log lines from qso_array and set the next qso number */
void scroll_log(void) {

    for (int i = 5; i > 0; i--) {
	if (NR_QSOS < i) {
	    g_strlcpy(logline_edit[5 - i], spaces(80), LINELEN + 1);
	} else {
	    g_strlcpy(logline_edit[5- i], QSOS(NR_QSOS - i), LINELEN + 1);
	}
    }

    get_next_serial();
}
