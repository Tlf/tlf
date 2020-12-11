/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@eudx.org>
 *               2011-2012                Thomas Beierlein <tb@forth-ev.de>
 *               2013-2014                Ervin Hegedus - HA2OS <airween@gmail.com>
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
 *        Getexchange handles  the  comment field
 *
 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "addspot.h"
#include "cw_utils.h"
#include "change_rst.h"
#include "globalvars.h"
#include "keyer.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "locator2longlat.h"
#include "logit.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "qtcwin.h"
#include "recall_exchange.h"
#include "rtty.h"
#include "score.h"
#include "searchlog.h"		// Includes glib.h
#include "sendbuf.h"
#include "setcontest.h"
#include "speedupndown.h"
#include "stoptx.h"
#include "time_update.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "addmult.h"

#include "getexchange.h"

#define LEN(array) (sizeof(array) / sizeof(array[0]))


int play_file(char *audiofile);


int getlastpattern(char *checkstring);
void exchange_edit(void);

int getexchange(void) {

    extern char comment[];
    extern char cqzone[];
    extern char ituzone[];
    extern char ph_message[14][80];
    extern char hiscall[];
    extern char qsonrstr[];
    extern int pacc_pa_flg;
    extern int exchange_serial;
    extern int countrynr;
    extern int trxmode;
    extern int recall_mult;
    extern int arrlss;
    extern int lan_active;
    extern char lastqsonr[];
    extern char qsonrstr[];
    extern char section[];
    extern int serial_section_mult;
    extern int serial_grid4_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;
    extern int ctcomp;
    extern int wazmult;
    extern int itumult;
    extern int exc_cont;
    extern char continent[];
    extern int commentfield;
    extern int no_rst;
    extern int serial_or_section;
    extern int ve_cty;
    extern int w_cty;

    int i;
    int x = 0;
    char instring[2];
    char commentbuf[40] = "";
    char *gridmult = "";

    instring[1] = '\0';

    if ((lan_active == 1) && (exchange_serial == 1)) {
	strncpy(lastqsonr, qsonrstr, 5);
	send_lan_message(INCQSONUM, qsonrstr);
    }

    if (recall_mult == 1)
	recall_exchange();

    if (IS_CONTEST(ARRLDX_USA) && trxmode != CWMODE)
	recall_exchange();

    if (IS_CONTEST(ARRL_FD))
	recall_exchange();

    if ((IS_CONTEST(CQWW) || (wazmult == 1) || (itumult == 1))
	    && (*comment == '\0') && (strlen(hiscall) != 0)) {
	if (itumult == 1)
	    strcpy(comment, ituzone);
	else
	    strcpy(comment, cqzone);
    }
    if ((exc_cont == 1) && (*comment == '\0')
	    && (strlen(hiscall) != 0)) {
	strcpy(comment, continent);
    }

    if (IS_CONTEST(STEWPERRY)) {
	recall_exchange();
    }

    /* parse input and modify exchange field accordingly */

    commentfield = 1;

    i = strlen(comment);

    while (1) {

	refresh_comment();

	/* wait for next char pressed, but update time, cluster and TRX qrg */
	/* main loop waiting for input */
	x = -1;
	while (x < 1) {

	    usleep(10000);

	    time_update();

	    if (trxmode == DIGIMODE) {
		show_rtty();
	    }

	    /* make sure that the wrefresh() inside getch() shows the cursor
	     * in the input field */
	    wmove(stdscr, 12, 54 + strlen(comment));
	    x = key_poll();
	}

	switch (x) {

	    case CTRL_Q: {	// Ctl-q (^Q)--Open QTC panel for receiving or sending QTCs
		if (qtcdirection == 1 || qtcdirection == 3) {	// in case of QTC=RECV or QTC=BOTH
		    qtc_main_panel(RECV);
		}
		if (qtcdirection == 2) {			// in case of QTC=SEND
		    qtc_main_panel(SEND);
		}
		x = KEY_LEFT;
		continue;
	    }
	    case 19: {	// Ctl+s (^S)--Open QTC panel for sending QTCs
		if (qtcdirection == 2 || qtcdirection == 3) {	// in case of QTC=SEND ot QTC=BOTH
		    qtc_main_panel(SEND);
		}
		x = KEY_LEFT;
		continue;
	    }
	    case CTRL_A: {	// Ctrl-A (^A)
		addspot();
		*comment = '\0';
		x = TAB;	// <Tab>
		break;
	    }

	    case KEY_BACKSPACE: {	// Erase (^H or <Backspace>)
		if (i >= 1) {
		    comment[strlen(comment) - 1] = '\0';
		    i -= 1;
		}
		break;
	    }

	    case ESCAPE: {                // <Escape>
		stoptx();			/* stop sending CW */
		if (comment[0] != '\0') {	/* if comment not empty */
		    /* drop exchange so far */
		    comment[0] = '\0';
		    i = 0;
		} else {
		    /* back to callinput */
		    x = TAB;	// <Tab>
		}
		break;
	    }

	    /* I cannot find any reference for this key combination in my
	     * CT ver 9 documentation.  As it is, most X window managers
	     * will trap this combination for the window menu so would
	     * only be useful on the console.
	     *
	     * - N0NB
	     */
	    /* case 160: {	// For CT compatibility Meta-<Space> (M- ) */
	    /*     if (ctcomp != 0) { */
	    /*         send_standard_message(1);		// F2 */

	    /*     } */
	    /*     break; */
	    /* } */

	    /* '+', send TU and log in CT mode */
	    case '+': {
		if ((ctcomp != 0) && (strlen(hiscall) > 2)) {
		    if (comment[0] == '\0') {
			x = -1;
		    } else {
			/* F4 (TU macro) */
			send_standard_message(3);

			/* log without additional message */
			x = BACKSLASH;
		    }
		}
		break;
	    }

	    /* <Insert>, send exchange in CT mode */
	    case KEY_IC: {
		if (ctcomp != 0) {
		    /* F3 (RST macro) */
		    send_standard_message(2);

		}
		break;
	    }

	    case KEY_F(1): {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    sendmessage(my.call);		/* F1 */
		} else
		    play_file(ph_message[5]);	// call

		break;
	    }

	    case KEY_F(2) ... KEY_F(11): {
		/* F2...F11 - F1 = 1...10 */
		send_standard_message(x - KEY_F(1));

		break;
	    }

	    case 176 ... 185: {	/* Alt-0 to Alt-9 */
		send_standard_message(x - 162);	/* Messages 15-24 */

		break;
	    }

	    /* <Home>--edit exchange field, position cursor to left end of field.
	     * Fall through to KEY_LEFT stanza if ungetch() is successful.
	     */
	    case KEY_HOME: {
		if (ungetch(x) != OK)
		    break;
	    }

	    case KEY_LEFT: {	/* Left Arrow--edit exchange field */
		if (*comment != '\0')
		    exchange_edit();
		break;
	    }

	    case KEY_PPAGE: {	/* Page-Up--change MY RST */
		if (change_rst) {
		    rst_recv_up();

		    if (!no_rst)
			mvprintw(12, 49, recvd_rst);

		} else {	/* speed up */
		    speedup();

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2u", GetCWSpeed());
		}
		break;

	    }
	    case KEY_NPAGE: {	/* Page-Down--change MY RST */
		if (change_rst) {

		    rst_recv_down();

		    if (!no_rst)
			mvprintw(12, 49, recvd_rst);

		} else {	/* speed down */
		    speeddown();

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2u", GetCWSpeed());
		}
		break;

	    }
	    case ',':		// Keyboard Morse
	    case CTRL_K: {	// Ctrl-K
		mvprintw(5, 0, "");
		keyer();
		x = 0;
		break;
	    }
	    case '\n':
	    case KEY_ENTER: {
		/* log QSO immediately if CT compatible
		 * or not in contest */
		if ((ctcomp == 1) || (!iscontest)) {
		    /* Don't log if exchange field is empty. */
		    if (comment[0] == '\0') {
			x = -1;
		    } else {
			/* Log without sending a message. */
			x = BACKSLASH;
		    }
		}
		break;
	    }
	}	// End switch

	if (x >= 'a' && x <= 'z')
	    x = x - 32;		// Promote to upper case

	if (i < 25) {		/* normal character -> insert if space left */
	    if (x >= ' ' && x <= 'Z') {
		instring[0] = x;
		addch(x);
		strcat(comment, instring);
		i++;
		refreshp();
	    }
	}

	if ((serial_section_mult == 1) ||
		(dx_arrlsections == 1) ||
		(sectn_mult == 1) ||
		(sectn_mult_once == 1) ||
		(arrlss == 1) ||
		IS_CONTEST(CQWW) ||
		IS_CONTEST(STEWPERRY)) {

	    x = checkexchange(x);
	}

	/* <Enter>, <Tab>, Ctl-K, '\' */
	if (x == '\n' || x == KEY_ENTER || x == TAB
		|| x == CTRL_K || x == BACKSLASH) {

	    if ((exchange_serial == 1 && comment[0] >= '0'
		    && comment[0] <= '9')) {	/* align serial nr. */
		if (strlen(comment) == 1) {
		    strcpy(commentbuf, comment);
		    comment[0] = '\0';
		    strcat(comment, "00");
		    strcat(comment, commentbuf);
		}

		if (strlen(comment) == 2) {
		    strcpy(commentbuf, comment);
		    comment[0] = '\0';
		    strcat(comment, "0");
		    strcat(comment, commentbuf);
		}

	    }

	    if (IS_CONTEST(WPX)) {	/* align serial nr. */

		if ((strlen(comment) == 1) || (comment[1] == ' ')) {
		    strcpy(commentbuf, comment);
		    comment[0] = '\0';
		    strcat(comment, "00");
		    strcat(comment, commentbuf);
		}

		if ((strlen(comment) == 2) || (comment[2] == ' ')) {
		    strcpy(commentbuf, comment);
		    comment[0] = '\0';
		    strcat(comment, "0");
		    strcat(comment, commentbuf);
		}

	    }

	    if (IS_CONTEST(SPRINT)) {

		if ((comment[1] == ' ') && (comment[0] != ' ')) {

		    strcpy(commentbuf, "00");
		    commentbuf[2] = comment[0];
		    commentbuf[3] = '\0';
		    strcat(commentbuf, comment + 1);
		    strcpy(comment, commentbuf);
		}
		if ((comment[2] == ' ') && (comment[1] != ' ')) {

		    strcpy(commentbuf, "0");
		    commentbuf[1] = comment[0];
		    commentbuf[2] = comment[1];
		    commentbuf[3] = '\0';
		    strcat(commentbuf, comment + 2);
		    strcpy(comment, commentbuf);
		}

	    }

	    if ((pacc_pa_flg == 1) && (countrynr != my.countrynr)) {
		if (strlen(comment) == 1) {
		    strcpy(commentbuf, comment);
		    comment[0] = '\0';
		    strcat(comment, "00");
		    strcat(comment, commentbuf);
		}

		if (strlen(comment) == 2) {
		    strcpy(commentbuf, comment);
		    comment[0] = '\0';
		    strcat(comment, "0");
		    strcat(comment, commentbuf);
		}

	    }

	    if ((arrlss == 1) && (x != TAB) && (strlen(section) < 2)) {
		mvprintw(13, 54, "section?");
		mvprintw(12, 54, comment);
		x = 0;
	    } else if (((serial_section_mult == 1) || (sectn_mult == 1))
		       && ((x != TAB) && (strlen(section) < 1))) {
		if (serial_or_section == 0 || (serial_or_section == 1
					       && country_found(hiscall))) {
		    mvprintw(13, 54, "section?", section);
		    mvprintw(12, 54, comment);
		    refreshp();
		}
		break;

	    } else if (serial_grid4_mult == 1) {
		//      mvprintw(13,54, "section?");
		mvprintw(12, 54, comment);
		refreshp();
		gridmult = getgrid(comment);
		strcpy(section, gridmult);
		section[4] = '\0';

		break;
//                              x = 0; //##debug

	    } else if (IS_CONTEST(STEWPERRY)) {
		if (check_qra(comment) == 0) {
		    mvprintw(13, 54, "locator?");
		    mvprintw(12, 54, comment);
		    break;
		}
		refreshp();
		break;
	    } else if (IS_CONTEST(CQWW) && trxmode == DIGIMODE && ((countrynr == w_cty)
		       || (countrynr == ve_cty))) {
		if (strlen(comment) < 5) {
		    mvprintw(13, 54, "state/prov?");
		    mvprintw(12, 54, comment);
		    if (x == '\n' || x == KEY_ENTER || x == BACKSLASH) {
			x = 0;
		    } else {
			refreshp();
			break;
		    }
		    x = 0;
		} else {
		    refreshp();
		    break;
		}
	    } else
		break;
	}

    }
    refresh_comment();

    commentfield = 0;

    return (x);
}


/* ------------------------------------------------------------------------ */

char cmpattern[32] = "                               ";	// global
char ssexchange[30] = "";
char section[8] = "";
char callupdate[7];
int call_update = 0;
char zone_export[3] = "  ";
char zone_fix[3] = "";

/* ------------------------------------------------------------------------ */

int checkexchange(int x) {

    extern char comment[];
    extern char ssexchange[];
    extern int arrlss;
    extern char section[];
    extern char callupdate[];
    extern char hiscall[];
    extern int call_update;
    extern char zone_export[];
    extern char zone_fix[];
    extern int serial_section_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;

    char precedent[] = " ";
    char serial[5] = "    ";
    char check[3] = "  ";
    char checksection[30];
    char zone[4] = "";

    /* field of allowed pattern sequences
     *
     * The characters have the following meaning:
     * u - undefined (left or right delimiter)
     * b - blank character
     * a - ascii character
     * f - a figure / digit
     *
     * e.g. faf means a character between two digits
     */
    char serpats[8][8] = {
	"bfb",
	"afb",
	"bfa",
	"bffab",
	"affab",
	"bffbffb",
	"fff",
	"ffff"
    };
    char precpats[8][4] = {
	"faf",
	"fab",
	"bab",
	"baf",
	"fau",
	"bau",
	"uaf",
	"uab"
    };
    char checkpats[6][5] = {
	"bffb",
	"bffu",
	"affu",
	"affb",
	"affa",
	"bffa"
    };
    char secpats[13][7] = {
	"fab",
	"faab",
	"faaab",
	"faaaab",
	"bab",
	"baab",
	"baaab",
	"baaaab",
	"bau",
	"baau",
	"baaau",
	"baaaau",
	"baafb"
    };
    char callpats[5][9] = {
	"bafaab",
	"baafab",
	"baafaab",
	"bafaaab",
	"baafaaab"
    };
    char zonepats[6][6] = {
	"ufb",
	"uffb",
	"bfb",
	"bffb",
	"bffu",
	"bfu"
    };
    char sectionpats[12][7] = {
	"uab",
	"uaab",
	"uaaab",
	"uaaaab",
	"uau",
	"uaau",
	"uaaau",
	"uaiaaau",
	"bab",
	"baab",
	"baaab",
	"baaaab"
    };

    int i, s, hr, ii, pr, jj;

    /* get the pattern sequence from comment string */
    strcpy(cmpattern, "u                    ");

    if (strlen(comment) > 0) {

	for (i = 0; i < strlen(comment); i++) {

	    switch ((int) comment[i]) {

		case 'A'...'Z': {
		    cmpattern[i + 1] = 'a';
		    cmpattern[i + 2] = 'u';
		    break;
		}

		case '0'...'9': {
		    cmpattern[i + 1] = 'f';
		    cmpattern[i + 2] = 'u';
		    break;
		}

		case ' ': {
		    cmpattern[i + 1] = 'b';
		    // TODO: check if add 'u' is missing
		    break;
		}

		default:
		    cmpattern[i + 1] = 'u';
	    }
	}
    }

    // -----------------------------------cqww-----------------------
    if (IS_CONTEST(CQWW)) {

	s = atoi(comment);
	snprintf(zone, sizeof(zone), "%02d", s);

	for (ii = 0; ii < LEN(zonepats); ii++) {

	    hr = getlastpattern(zonepats[ii]);

//! \todo  logik und Verwendung zone_fix vs zone_export unklar
//! Was passiert, falls zonenummer in comment zu groess ist?
	    if ((hr > 1) && (atoi(comment + hr - 1) != 0)) {
		sprintf(zone, "%02d", atoi(comment + hr - 1));
		strncpy(zone_fix, zone, 2);
		zone_fix[2] = '\0';
	    } else {
		strncpy(zone_export, zone, 2);
		zone_export[2] = '\0';
	    }
	}

	if (strlen(hiscall) >= 2)
	    OnLowerSearchPanel(32, zone_export);


	for (ii = 0; ii < LEN(callpats); ii++) {

	    hr = getlastpattern(callpats[ii]);	// call update ?

	    if (hr > 0) {

		switch (ii) {

		    case 0 ... 1:
			strncpy(callupdate, comment + hr, 4);
			callupdate[4] = '\0';
			break;
		    case 2 ... 3:
			strncpy(callupdate, comment + hr, 5);
			callupdate[5] = '\0';
			break;
		    case 4:
			strncpy(callupdate, comment + hr, 6);
			callupdate[6] = '\0';
		}

		if (strlen(callupdate) > 3) {

		    if (call_update == 1)
			strcpy(hiscall, callupdate);

		    mvprintw(12, 29, "       ");
		    mvprintw(12, 29, "%s", hiscall);
		    mvprintw(12, 54, "%s", comment);
		}
	    }
	}

	return (x);
    }

    // ---------------------------arrls------------------------------
    if (arrlss == 1) {

	// get serial nr.

	s = atoi(comment);

	if (s != 0)
	    snprintf(serial, sizeof(serial), "%4d", s);

	for (ii = 0; ii < LEN(serpats); ii++) {

	    hr = getlastpattern(serpats[ii]);

	    if (hr > 0)
		snprintf(serial, sizeof(serial), "%4d",
			 atoi(comment + hr - 1));

	    if (ii == 5 && hr > 0) {
		snprintf(serial, sizeof(serial), "%4d", atoi(comment + hr - 1));
		snprintf(check, sizeof(check), "%2d", atoi(comment + hr + 2));
	    }

	}

	// get precedent

	if (((comment[0] == 'A')
		|| (comment[0] == 'B')
		|| (comment[0] == 'M')
		|| (comment[0] == 'Q')
		|| (comment[0] == 'S')
		|| (comment[0] == 'U'))
		&& ((comment[1] == ' ') || (cmpattern[2] == 'f'))) {

	    precedent[0] = comment[0];
	}


	/* look for a single letter */
	for (ii = 0; ii < LEN(precpats); ii++) {

	    hr = getlastpattern(precpats[ii]);

	    if (hr > 0) {
		pr = comment[hr];
		if ((pr == 'Q') || (pr == 'A') || (pr == 'B')
			|| (pr == 'U') || (pr == 'M') || (pr == 'S')) {
		    precedent[0] = pr;
		    precedent[1] = '\0';
		}
	    }
	}

	// get call update

	for (ii = 0; ii < LEN(callpats); ii++) {

	    hr = getlastpattern(callpats[ii]);

	    if (hr > 0) {
		if (((comment[hr] == 'A') && (comment[hr + 1] > 59))
			|| (comment[hr] == 'K') || (comment[hr] == 'N')
			|| (comment[hr] == 'W')
			|| (comment[hr] == 'V') || (comment[hr] == 'C')) {

		    switch (ii) {

			case 0 ... 1:
			    strncpy(callupdate, comment + hr, 4);
			    callupdate[4] = '\0';
			    break;
			case 2 ... 3:
			    strncpy(callupdate, comment + hr, 5);
			    callupdate[5] = '\0';
			    break;
			case 4:
			    strncpy(callupdate, comment + hr, 6);
			    callupdate[6] = '\0';

		    }
		    if (strlen(callupdate) > 3) {

			if (call_update == 1)
			    strcpy(hiscall, callupdate);

			mvprintw(12, 29, "       ");
			mvprintw(12, 29, "%s", hiscall);
			mvprintw(12, 54, "%s", comment);
		    }

		}
	    }

	}

	// get check

	for (ii = 0; ii < LEN(checkpats); ii++) {

	    hr = getlastpattern(checkpats[ii]);
	    if (hr > 0) {
		check[0] = comment[hr];
		check[1] = comment[hr + 1];
		check[2] = '\0';
	    }
	}

	// get section
	*section = '\0';

	for (ii = 0; ii < LEN(secpats); ii++) {

	    hr = getlastpattern(secpats[ii]);

	    if (hr > 0) {

		g_strlcpy(checksection, comment + hr, 4);
		g_strchomp(checksection);

		for (jj = 0; jj < get_mult_count(); jj++) {

		    char *multi = g_strdup(get_mult(jj));
		    g_strchomp(multi);

		    if ((strlen(multi) >= 1) &&
			    (strcmp(checksection, multi) == 0)) {

			strcpy(section, multi);
			break;
		    }
		    g_free(multi);
		}
	    }
	}

	{
	    char buf[40];
	    sprintf(buf, " %4s %1s %2s %2s ", serial, precedent,
		    check, section);
	    OnLowerSearchPanel(8, buf);
	}

	/* \todo use sprintf */
	ssexchange[0] = '\0';

	strcat(ssexchange, serial);
	strcat(ssexchange, " ");
	strcat(ssexchange, precedent);
	strcat(ssexchange, " ");
	strcat(ssexchange, check);
	strcat(ssexchange, " ");
	strcat(ssexchange, section);

	mvprintw(12, 54, comment);
	refreshp();

	return (x);		// end arrlss
    }

    // ----------------------serial+section--------------------------
    if ((serial_section_mult == 1) || (sectn_mult == 1)
	    || (sectn_mult_once == 1)
	    || (dx_arrlsections == 1)) {

	if (serial_section_mult == 1) {

	    // get serial nr.

	    s = atoi(comment);

	    if (s != 0)
		snprintf(serial, sizeof(serial), "%4d", atoi(comment));

	    for (ii = 0; ii < LEN(serpats); ii++) {

		hr = getlastpattern(serpats[ii]);

		if (hr > 0)
		    snprintf(serial, sizeof(serial), "%4d",
			     atoi(comment + hr - 1));

		if (ii == 5 && hr > 0) {
		    snprintf(serial, sizeof(serial), "%4d",
			     atoi(comment + hr - 1));
		    snprintf(check, sizeof(check), "%2d",
			     atoi(comment + hr + 2));
		}

	    }

	    // get section

	    for (ii = 0; ii < LEN(secpats); ii++) {

		hr = getlastpattern(secpats[ii]);

		if (hr > 0) {

		    memset(checksection, 0, 29);
		    strncpy(checksection, comment + (hr), MAX_SECTION_LENGTH);
		    if (checksection[strlen(checksection) - 1] == ' ') {
			checksection[strlen(checksection) - 1] = '\0';
		    }

		    for (jj = 0; jj < get_mult_count(); jj++) {
			if (get_matching_length(checksection, jj) ==
				strlen(checksection)) {
			    strcpy(section, get_mult(jj));
			    break;
			}
		    }
		}
	    }
	}			// end serial_section_mult
	if ((sectn_mult == 1) || (sectn_mult_once)) {

	    for (ii = 0; ii < LEN(sectionpats); ii++) {

		hr = getlastpattern(sectionpats[ii]);

		g_strlcpy(checksection, comment, MAX_SECTION_LENGTH + 1);

		int best_len = 0;
		int idx = -1;
		for (jj = 0; jj < get_mult_count(); jj++) {
		    int len = get_matching_length(checksection, jj);
		    if (len > best_len) {
			best_len = len;
			idx = jj;
		    }
		}

		if (idx >= 0) {
		    strcpy(section, get_mult(idx));
		}
	    }

	}			//  end sectn_mult
	if (dx_arrlsections == 1) {

	    for (ii = 0; ii < LEN(sectionpats); ii++) {

		hr = getlastpattern(sectionpats[ii]);

		strncpy(checksection, comment, 3);
		checksection[3] = '\0';

		for (jj = 0; jj < get_mult_count(); jj++) {
		    if (get_matching_length(checksection, jj) == strlen(checksection)) {
			strcpy(section, get_mult(jj));

			// if (strlen(section) == strlen(mults_possible[jj])) break;
		    }
		}

	    }

	}			// end dx_arrlsections

	callupdate[0] = '\0';

    }


    // get call update

    for (ii = 0; ii < LEN(callpats); ii++) {

	hr = getlastpattern(callpats[ii]);

	if (hr > 0) {
	    if (((comment[hr] == 'A') && (comment[hr + 1] > 59))
		    || (comment[hr] == 'K') || (comment[hr] == 'N')
		    || (comment[hr] == 'W')
		    || (comment[hr] == 'V') || (comment[hr] == 'C')) {

		switch (ii) {

		    case 0 ... 1:
			strncpy(callupdate, comment + hr, 4);
			callupdate[4] = '\0';
			break;
		    case 2 ... 3:
			strncpy(callupdate, comment + hr, 5);
			callupdate[5] = '\0';
			break;
		    case 4:
			strncpy(callupdate, comment + hr, 6);
			callupdate[6] = '\0';

		}

		if (strlen(callupdate) > 3) {

		    if (call_update == 1)
			strcpy(hiscall, callupdate);

		    mvprintw(12, 29, "       ");
		    mvprintw(12, 29, "%s", hiscall);
		    mvprintw(12, 54, "%s", comment);
		}

	    }
	}

    }
    OnLowerSearchPanel(32, "   ");
    OnLowerSearchPanel(32, section);	/* show section on lower frame of
					   Worked window */
    ssexchange[0] = '\0';

    /*	if (serial_section_mult == 1) {
    		strcat (ssexchange,serial);
    		strcat (ssexchange, " ");
    	}
    */
    strcat(ssexchange, section);

    // ---------------------------end mults --------------------------
    mvprintw(12, 54, comment);
    refreshp();

    return (x);
}


/* ------------------------------------------------------------------------ */

/** search checkstring in cmpattern
 *
 * find first occurence of checkstring in cmpattern
 * \parm checkstring - the pattern to be found
 * \return offset of checkstring in cmpattern (or 0 if not found)
 */
int getlastpattern(char *checkstring) {

    extern char comment[];
    char newpat[80];
    int i, x = 0;

    if ((strlen(cmpattern) - strlen(checkstring)) > 0) {
	for (i = 0; i < (strlen(cmpattern) - strlen(checkstring)) - 1; i++) {

	    newpat[0] = '\0';
	    strncat(newpat, cmpattern + i, strlen(comment));

	    if (strncmp(newpat, checkstring, strlen(checkstring)) == 0) {
		x = i;
	    }
	}
	if (x > strlen(comment))
	    x = 0;
    }
    return (x);

}

/* ------------------------------------------------------------------------
 * return a pointer to the start of grid locator
 */

char *getgrid(char *comment) {

    char *gridmult = "";
    int multposition = 0;
    int i = 0;

    /* search for first letter, that should be the start of the Grid locator*/
    for (i = 0; i < strlen(comment); i++) {
	if (comment[i] > 64 && comment[i] < 91) {
	    multposition = i;
	    break;
	}
    }
    gridmult = comment + multposition;

    return (gridmult);
}

/* ------------------------------------------------------------------------ */
/** Edit exchange field
 */

void exchange_edit(void) {
    extern char comment[];

    int l, b;
    int i = 0, j;
    char comment2[27];

    l = strlen(comment);
    b = l - 1;

    while ((i != ESCAPE) && (b <= strlen(comment))) {
	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvprintw(12, 54, spaces(80 - 54));
	mvprintw(12, 54, comment);
	mvprintw(12, 54 + b, "");

	i = key_get();

	// Ctrl-A (^A) or <Home>, move to beginning of comment field.
	if (i == CTRL_A || i == KEY_HOME) {

	    b = 0;

	    // Ctrl-E (^E) or <End>, move to end of comment field, exit edit mode.
	} else if (i == CTRL_E || i == KEY_END) {

	    b = strlen(comment);
	    break;

	    // Left arrow, move cursor left one position.
	} else if (i == KEY_LEFT) {

	    if (b > 0)
		b--;

	    // Right arrow, move cursor right one position.
	} else if (i == KEY_RIGHT) {

	    if (b < strlen(comment) - 1) {
		b++;
	    } else
		break;		/* stop edit */

	    // <Delete>, erase character under the cursor,
	    // shift all characters to the right of the cursor left one position.
	} else if (i == KEY_DC) {

	    l = strlen(comment);

	    for (j = b; j <= l; j++) {
		comment[j] = comment[j + 1];	/* move to left incl.\0 */
	    }

	    // <Backspace>, erase character to the left of the cursor,
	    // shift all characters to the right of the cursor left one position.
	} else if (i == KEY_BACKSPACE) {

	    if (b > 0) {
		b--;

		l = strlen(comment);

		for (j = b; j <= l; j++) {
		    comment[j] = comment[j + 1];
		}
	    }

	    // <Escape> not received.
	} else if (i != ESCAPE) {

	    // Promote lower case to upper case.
	    if ((i >= 'a') && (i <= 'z'))
		i = i - 32;

	    // Accept printable characters.
	    if ((i >= ' ') && (i <= 'Z')) {

		if (strlen(comment) <= 24) {
		    /* copy including trailing \0 */
		    strncpy(comment2, comment + b, strlen(comment) - (b - 1));

		    comment[b] = i;
		    comment[b + 1] = '\0';
		    strcat(comment, comment2);

		    b++;
		}

	    } else if (i != 0)
		i = ESCAPE;
	}
    }

    attron(A_STANDOUT);
    refresh_comment();
}
