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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/*------------------------------------------------------------------------

    Log QSO to disk and do all necessary actions to start a new one

------------------------------------------------------------------------*/

#include "globalvars.h"
#include "log_to_disk.h"

int log_to_disk(void)
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
#ifdef HAVE_LIBHAMLIB
    extern freq_t outfreq;
#else
    extern int outfreq;
#endif
    extern int block_part;
    extern int lan_active;
    extern char lan_message[];
    extern char thisnode;
    extern int lan_mutex;
    extern int cqwwm2;

    if (lan_mutex == 1) {	// qso from this node

	addcall();

	makelogline();

	hiscall[0] = '\0';	/* reset the call  string */

	comment[0] = '\0';	/* reset the comment  string */
	comment[30] = '\0';

	store_qso(logline4);
    } else {			// qso from lan

	if ((lan_mutex == 2) && (lan_message[0] != thisnode)
	    && (lan_active == 1)) {
	    strncpy(lan_logline, lan_message + 2, 80);
	    strcat(lan_logline,
		   "                                                                              ");

	    if (cqwwm2 == 1) {
		if (lan_logline[0] != thisnode)
		    lan_logline[79] = '*';
	    }

	    lan_logline[80] = '\0';

	    score2();
	    addcall2();

	    store_qso(lan_logline);
	}

    }

    // send qso to other nodes......

    if ((lan_active == 1) && (strlen(lan_message) == 0)) {

	send_lan_message(LOGENTRY, logline4);
    }

    lan_message[0] = '\0';

    scroll_log();

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);	/* erase comment  field */
    else
	attron(COLOR_PAIR(NORMCOLOR));

    if (lan_mutex != 2)
	mvprintw(12, 54, "                          ");

    attron(COLOR_PAIR(7) | A_STANDOUT);
    if (lan_mutex != 2) {
	mvprintw(7, 0, logline0);
	mvprintw(8, 0, logline1);
	mvprintw(9, 0, logline2);
    }
    mvprintw(10, 0, logline3);
    mvprintw(11, 0, logline4);
    refresh();

    attron(COLOR_PAIR(COLOR_CYAN));

    mvprintw(12, 23, qsonrstr);
    mvprintw(12, 44, his_rst);
    mvprintw(12, 49, my_rst);

    sync();

    if ((rit == 1) && (trx_control == 1))
	outfreq = RESETRIT;

    block_part = 0;		/* unblock use partials */

    return (0);
}
