/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 * 		 2010-2014 Thomas Beierlein <tb@forth-ev.de>
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
 *
 * 		Log routine
 *
 ------------------------------------------------------------------------*/


#include <string.h>

#include "background_process.h"
#include "callinput.h"
#include "clear_display.h"
#include "getexchange.h"
#include "keyer.h"
#include "log_to_disk.h"
#include "printcall.h"
#include "recall_exchange.h"
#include "searchlog.h"		// Includes glib.h
#include "sendbuf.h"
#include "sendqrg.h"
#include "sendspcall.h"
#include "set_tone.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "cleanup.h"


void refresh_comment(void);
void change_mode(void);

void logit(void) {
    extern char mode[];
    extern int trxmode;
    extern char hiscall[];
    extern int cqmode;
    extern int contest;
    extern char ph_message[14][80];
    extern char comment[];
    extern int cqww;
    extern char cqzone[];
    extern char itustr[];
    extern int defer_store;
    extern int recall_mult;
    extern int simulator;
    extern int simulator_mode;
    extern int ctcomp;
    extern int wazmult;
    extern int itumult;
    extern int qsonum;
    extern int exchange_serial;
    extern char tonestr[];
    extern int dxped;
    extern int sprint_mode;

    int callreturn = 0;
    int cury, curx;
    int qrg_out = 0;

    strcpy(mode, "Log     ");
    clear_display();
    defer_store = 0;

    start_background_process();	/* start it up */

    while (1) {
	printcall();

	if ((callreturn == 0) && (defer_store == 2))
	    callreturn = ' ';
	else
	    callreturn = callinput();

	qrg_out = sendqrg();

	if (qrg_out == 0) {		/* no frequency entered? */

	    if ((trxmode == CWMODE || trxmode == DIGIMODE)
		    && (callreturn == '\n') && ctcomp == 1) {
		callreturn = 92; 	/* '\' */
		strcpy(comment, cqzone);
	    }

	    if ((callreturn == 9 || callreturn == 32)) {
		callreturn = getexchange();
	    }

	    if (callreturn == '\n' && strlen(hiscall) >= 3) {
		if ((*comment == '\0') && contest == CONTEST
			&& !ctcomp && !dxped)
		    defer_store = 0;

		if ((cqmode == CQ) && (contest == CONTEST)
			&& (defer_store == 0)) {	/* CQ mode */

		    send_standard_message(2);
		    if (trxmode != CWMODE && trxmode != DIGIMODE) {
			if (exchange_serial == 1)
			    mvprintw(13, 29, "Serial number: %d", qsonum);
			refreshp();
		    }

		    if (simulator != 0) {
			strcpy(tonestr, "700");
			write_tone();
			if (strstr(hiscall, "?") != NULL)
			    simulator_mode = 3;
			else
			    simulator_mode = 2;
		    }

		    if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {

			if (recall_exchange() == -1) {
			    if (itumult == 1)
				strcpy(comment, itustr);	/* fill in the ITUzone */
			    else
				strcpy(comment, cqzone);	/* fill in the CQzone */
			}

			refresh_comment();
		    }

		    if (recall_mult == 1) {
			recall_exchange();
		    }

		    defer_store = 1;
		    callreturn = 0;
		}

		if ((cqmode == S_P) && (contest == CONTEST)
			&& (defer_store == 0)) {	/* S&P mode */

		    if (cqww == 1) {
			if (strlen(comment) == 0 && recall_exchange() == -1)
			    strcpy(comment, cqzone);	/* fill in the zone */

			refresh_comment();

		    } else if (recall_mult == 1) {
			recall_exchange();
		    }

		    if (trxmode == CWMODE || trxmode == DIGIMODE)
			sendspcall();
		    else {
			play_file(ph_message[5]);
			if (exchange_serial == 1)
			    mvprintw(13, 29, "Serial number: %d", qsonum);
			refreshp();
		    }

		    defer_store = 1;
		    callreturn = 0;
		}

		if (defer_store == 1) {
		    defer_store++;
		    callreturn = 0;
		} else if (defer_store > 1) {
		    if ((cqmode == CQ) && (contest == CONTEST)) {
			send_standard_message(CQ_TU_MSG);	/* send cq return */
			if (trxmode == CWMODE || trxmode == DIGIMODE) {
			    if (simulator != 0)
				simulator_mode = 1;
			    if (simulator != 0)
				write_tone();
			}

			defer_store = 0;

		    }

		    if ((cqmode == S_P) && (contest == CONTEST)) {
			send_standard_message(SP_TU_MSG); /* send S&P return */

			defer_store = 0;
		    }

		    log_to_disk(false);
		    if (sprint_mode == 1) {
			change_mode();
		    }
		    HideSearchPanel();

		}
	    }

	    if ((callreturn == 92) && (*hiscall != '\0')) {
		defer_store = 0;

		log_to_disk(false);
		if (sprint_mode == 1) {
		    change_mode();
		}
		HideSearchPanel();
	    }

	    if (callreturn == 11 || callreturn == 44) {	/*  CTRL K  */
		getyx(stdscr, cury, curx);
		mvprintw(5, 0, "");
		keyer();
		mvprintw(cury, curx, "");
	    }

	} else {	/* user entered frequency -> clear input field */
	    hiscall[0] = '\0';
	    HideSearchPanel();
	}
    }
}

/** reprint comment field */
void refresh_comment(void) {
    extern char comment[];

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvprintw(12, 54, "                          ");
    mvprintw(12, 54, comment);
}

void change_mode(void) {
    extern char mode[];
    extern int cqmode;

    /* switch to other mode */
    if (cqmode == CQ) {
	cqmode = S_P;
    } else {
	cqmode = CQ;
    }

    /* and show new mode */
    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

    if (cqmode == CQ) {
	mvprintw(0, 2, "Log     ");
	strcpy(mode, "Log     ");
    } else {
	mvprintw(0, 2, "S&P     ");
	strcpy(mode, "S&P     ");
    }
}
