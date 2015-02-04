/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Thomas Beierlein <tb@forth-ev.de>
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

    Log QSO to disk and do all necessary actions to start a new one

------------------------------------------------------------------------*/

#include "globalvars.h"
#include "log_to_disk.h"
#include <pthread.h>
#include "tlf.h"
#include "scroll_log.h"
#include "addcall.h"
#include "makelogline.h"
#include "store_qso.h"
#include "qsonr_to_str.h"
#include "writeparas.h"
#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif
#include "gettxinfo.h"
#include "lancode.h"
#include "addspot.h"
#include "score.h"

pthread_mutex_t disk_mutex = PTHREAD_MUTEX_INITIALIZER;

/** \brief logs one record to disk
 * Logs one record to disk which may come from different sources
 * (direct from tlf or from other instance via LAN)
 *
 * \param from_lan true - Log lanmessage, false - normal message
 */
int log_to_disk(int from_lan)
{
    extern int use_rxvt;
    extern char hiscall[];
    extern char comment[];
    extern char my_rst[];
    extern char his_rst[];
    extern char qsonrstr[5];
    extern char lan_logline[];
    extern int rit;
    extern int trx_control;
    extern int cqmode;
#ifdef HAVE_LIBHAMLIB
    extern freq_t outfreq;
#else
    extern int outfreq;
#endif
    extern int block_part;
    extern char lan_message[];
    extern char thisnode;
    extern int lan_mutex;
    extern int cqwwm2;
    extern int no_rst;

    pthread_mutex_lock(&disk_mutex);

    if (!from_lan) {		// qso from this node

	addcall();		/* add call to dupe list */

	makelogline();

	store_qso(logline4);

	// send qso to other nodes......
	send_lan_message(LOGENTRY, logline4);

	if (trx_control && (cqmode == S_P))
	    addspot();		/* add call to bandmap if in S&P and
				   no need to ask for frequency */

	hiscall[0] = '\0';	/* reset the call  string */
	comment[0] = '\0';	/* reset the comment  string */

	his_rst[1] = '9';	/* restore RST to 599 */
	my_rst[1] = '9';

    } else {			// qso from lan

	strncpy(lan_logline, lan_message + 2, 87);
	strcat(lan_logline,
	       "                                                                              ");

	if (cqwwm2 == 1) {
	    if (lan_logline[0] != thisnode)
		lan_logline[79] = '*';
	}

	lan_logline[87] = '\0';

	total = total + score2(lan_logline);

	addcall2();

	store_qso(lan_logline);
    }


    if (from_lan)
	lan_mutex = 2;
    else
	lan_mutex = 1;

    scroll_log();

    lan_mutex = 0;

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);	/* erase comment  field */
    else
	attron(COLOR_PAIR(NORMCOLOR));

    if (!from_lan)
	mvprintw(12, 54, "                          ");

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    if (!from_lan) {
	mvprintw(7, 0, logline0);
	mvprintw(8, 0, logline1);
	mvprintw(9, 0, logline2);
    }
    mvprintw(10, 0, logline3);
    mvprintw(11, 0, logline4);
    refreshp();

    attron(COLOR_PAIR(C_WINDOW));

    mvprintw(12, 23, qsonrstr);

    if (no_rst) {
	mvaddstr(12, 44, "---");
	mvaddstr(12, 49, "---");
    } else {
	mvaddstr(12, 44, his_rst);
	mvaddstr(12, 49, my_rst);
    }

    sync();

    if ((rit == 1) && (trx_control == 1))
	outfreq = RESETRIT;

    block_part = 0;		/* unblock use partials */

    pthread_mutex_unlock(&disk_mutex);

    return (0);
}
