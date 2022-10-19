/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013-2022      Thomas Beierlein <tb@forth-ev.de>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/*------------------------------------------------------------------------

    Log QSO to disk and do all necessary actions to start a new one

------------------------------------------------------------------------*/


#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "addcall.h"
#include "addspot.h"
#include "gettxinfo.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "lancode.h"
#include "log_utils.h"
#include "makelogline.h"
#include "scroll_log.h"
#include "score.h"
#include "store_qso.h"
#include "setcontest.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "cleanup.h"

pthread_mutex_t disk_mutex = PTHREAD_MUTEX_INITIALIZER;

char lan_logline[81];


/* restart band timer if in wpx and qso on new band */
void restart_band_timer(void) {
    static int lastbandinx = 0;

    if (CONTEST_IS(WPX)) {
	if (lastbandinx != bandinx) {
	    lastbandinx = bandinx;
	    minute_timer = 600;		/* 10 minute timer */
	}
    }
}

/** \brief logs one record to disk
 * Logs one record to disk which may come from different sources
 * (direct from tlf or from other instance via LAN)
 *
 * \param from_lan true - Log lanmessage, false - normal message
 */
void log_to_disk(int from_lan) {

    pthread_mutex_lock(&disk_mutex);

    if (!from_lan) {		// qso from this node

	/* remember call and report for resend after qso (see callinput.c)  */
	strcpy(lastcall, current_qso.call);
	strcpy(last_rst, sent_rst);

	// use normalized comment if available
	if (strlen(current_qso.normalized_comment) > 0) {
	    strcpy(current_qso.comment, current_qso.normalized_comment);
	}

	restart_band_timer();

	struct qso_t *qso = collect_qso_data(); //TODO: move this after store_qso() call below
	addcall(qso);		/* add call to dupe list */

	score_qso(qso);
	char *logline = makelogline(qso);
	qso->logline = logline; /* remember formatted line in qso entry */

	store_qso(logfile, logline);
        //TODO: create a copy of current_qso
	g_ptr_array_add(qso_array, qso);

	// send qso to other nodes......
	send_lan_message(LOGENTRY, logline);

	if (trx_control && (cqmode == S_P))
	    add_local_spot();	/* add call to bandmap if in S&P and
				   no need to ask for frequency */

	cleanup_qso();		/* reset qso related parameters */

    } else {			/* qso from lan */

	/* LOGENTRY contains 82 characters (node,command and logline */
	g_strlcpy(lan_logline, lan_message + 2, 81);
	char *fill = g_strnfill(80 - strlen(lan_logline), ' ');
	g_strlcat(lan_logline, fill, 81);    /* fill with spaces if needed */

	if (cqwwm2) {	    /* mark as coming from other station */
	    if (lan_message[0] != thisnode)
		lan_logline[79] = '*';
	}

	total = total + score2(lan_logline);

	struct qso_t *qso = parse_qso(lan_logline);

	addcall2();

	store_qso(logfile, lan_logline);
	g_ptr_array_add(qso_array, qso);
    }


    if (from_lan)
	lan_mutex = 2;
    else
	lan_mutex = 1;

    scroll_log();

    lan_mutex = 0;

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));	/* erase comment  field */

    if (!from_lan)
	mvaddstr(12, 54, spaces(contest->exchange_width));

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    if (!from_lan) {
	mvaddstr(7, 0, logline0);
	mvaddstr(8, 0, logline1);
	mvaddstr(9, 0, logline2);
    }
    mvaddstr(10, 0, logline3);
    mvaddstr(11, 0, logline4);
    refreshp();

    attron(COLOR_PAIR(C_WINDOW));

    mvaddstr(12, 23, current_qso_values.qsonrstr);

    if (no_rst) {
	mvaddstr(12, 44, "   ");
	mvaddstr(12, 49, "   ");
    } else {
	mvaddstr(12, 44, sent_rst);
	mvaddstr(12, 49, recvd_rst);
    }

    sync();

    if (rit) {
	set_outfreq(RESETRIT);
    }

    block_part = 0;		/* unblock use partials */

    pthread_mutex_unlock(&disk_mutex);
}
