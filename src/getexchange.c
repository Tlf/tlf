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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------
 *        Getexchange handles  the  comment field
 *
 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "addspot.h"
#include "audio.h"
#include "cw_utils.h"
#include "change_rst.h"
#include "globalvars.h"
#include "keyer.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "utils.h"
#include "logit.h"
#include "printcall.h"
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


void exchange_edit(void);

static void serial_up_down(char *exchange, int delta) {
    /* length of serial part in "001" or "001 EU-001" */
    int nr_len = strspn(exchange, "0123456789");
    if (nr_len == 0 || nr_len > 5) {
	return;
    }
    /* serial number, suffix ignored if any */
    int nr = atoi(exchange);
    nr += delta;
    if (nr < 0 || nr > 99999) {
	return;
    }
    /* preserve leading zeros, append old suffix */
    char *buf = g_strdup_printf("%0*d%s", nr_len, nr, exchange + nr_len);
    int len = strlen(buf);
    /* length can change when overflowing 9 -> 10 */
    if (len <= contest->exchange_width) {
	strcpy(exchange, buf);
    }
    g_free(buf);
}


int getexchange(void) {

    int i;
    int x = 0;
    char instring[2];
    char commentbuf[40] = "";

    instring[1] = '\0';

    if (lan_active && contest->exchange_serial) {
	strncpy(lastqsonr, current_qso_values.qsonrstr, 5);
	send_lan_message(INCQSONUM, current_qso_values.qsonrstr);
    }

    if (contest->recall_mult)
	recall_exchange();

    if (CONTEST_IS(ARRLDX_USA) && trxmode != CWMODE)
	recall_exchange();

    if (CONTEST_IS(ARRL_FD))
	recall_exchange();

    if ((CONTEST_IS(CQWW) || wazmult || itumult)
	    && (current_qso.comment[0] == '\0') && (strlen(current_qso.call) != 0)) {
	if (itumult)
	    strcpy(current_qso.comment, ituzone);
	else
	    strcpy(current_qso.comment, cqzone);
    }
    if ((exc_cont) && (current_qso.comment[0] == '\0')
	    && (strlen(current_qso.call) != 0)) {
	strcpy(current_qso.comment, continent);
    }

    if (CONTEST_IS(STEWPERRY)) {
	recall_exchange();
    }

    /* parse input and modify exchange field accordingly */

    commentfield = 1;

    i = strlen(current_qso.comment);
    while (1) {

	refresh_comment();

	checkexchange(&current_qso, true);

	if (call_update && strlen(current_qso.callupdate) >= 3) {
	    strcpy(current_qso.call, current_qso.callupdate);
            current_qso.callupdate[0] = 0;
	    printcall();
	}

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
	    wmove(stdscr, 12, 54 + strlen(current_qso.comment));
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
		if (qtcdirection == 2 || qtcdirection == 3) {	// in case of QTC=SEND or QTC=BOTH
		    qtc_main_panel(SEND);
		}
		x = KEY_LEFT;
		continue;
	    }
	    case CTRL_A: {	// Ctrl-A (^A)
		add_local_spot();
		current_qso.comment[0] = '\0';
		x = TAB;	// <Tab>
		break;
	    }

	    case KEY_BACKSPACE: {	// Erase (^H or <Backspace>)
		if (i >= 1) {
		    current_qso.comment[strlen(current_qso.comment) - 1] = '\0';
		    i -= 1;
		}
		break;
	    }

	    case ESCAPE: {                // <Escape>
		stoptx();			/* stop sending CW */
		if (current_qso.comment[0] != '\0') {	/* if comment not empty */
		    /* drop exchange so far */
		    current_qso.comment[0] = '\0';
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
		if (ctcomp && (strlen(current_qso.call) > 2)) {
		    if (current_qso.comment[0] == '\0') {
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
		if (ctcomp) {
		    /* F3 (RST macro) */
		    send_standard_message(2);

		}
		break;
	    }

	    case KEY_F(1): {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    sendmessage(my.call);		/* F1 */
		} else
		    vk_play_file(ph_message[5]);	// call

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
		if (current_qso.comment[0] != '\0') {
		    exchange_edit();
		    i = strlen(current_qso.comment);
		}
		break;
	    }

	    case KEY_UP:	/* Up/Down--increase/decrease serial number */
	    case KEY_DOWN: {
		serial_up_down(current_qso.comment, (x == KEY_UP) ? 1 : -1);
		i = strlen(current_qso.comment);
		break;
	    }

	    case KEY_PPAGE: {	/* Page-Up--change MY RST */
		if (change_rst) {
		    rst_recv_up();

		    if (!no_rst)
			mvaddstr(12, 49, recvd_rst);

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
			mvaddstr(12, 49, recvd_rst);

		} else {	/* speed down */
		    speeddown();

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2u", GetCWSpeed());
		}
		break;

	    }
	    case ',':		// Keyboard Morse
	    case CTRL_K: {	// Ctrl-K
		move(5, 0);
		keyer();
		x = 0;
		break;
	    }
	    case '\n':
	    case KEY_ENTER: {
		/* log QSO immediately if CT compatible
		 * or not in contest */
		if ((ctcomp) || (!iscontest)) {
		    /* Don't log if exchange field is empty. */
		    if (current_qso.comment[0] == '\0') {
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

	if (i < contest->exchange_width) {  /* normal character -> insert if space left */
	    if (x >= ' ' && x <= 'Z') {
		instring[0] = x;
		addch(x);
		strcat(current_qso.comment, instring);
		i++;
		refreshp();
	    }
	}

	/* <Enter>, <Tab>, Ctl-K, '\' */
	if (x == '\n' || x == KEY_ENTER || x == TAB
		|| x == CTRL_K || x == BACKSLASH) {

	    if ((contest->exchange_serial && current_qso.comment[0] >= '0'
		    && current_qso.comment[0] <= '9')) {	/* align serial nr. */
		if (strlen(current_qso.comment) == 1) {
		    strcpy(commentbuf, current_qso.comment);
		    current_qso.comment[0] = '\0';
		    strcat(current_qso.comment, "00");
		    strcat(current_qso.comment, commentbuf);
		}

		if (strlen(current_qso.comment) == 2) {
		    strcpy(commentbuf, current_qso.comment);
		    current_qso.comment[0] = '\0';
		    strcat(current_qso.comment, "0");
		    strcat(current_qso.comment, commentbuf);
		}

	    }

	    if (CONTEST_IS(WPX)) {	/* align serial nr. */

		if ((strlen(current_qso.comment) == 1) || (current_qso.comment[1] == ' ')) {
		    strcpy(commentbuf, current_qso.comment);
		    current_qso.comment[0] = '\0';
		    strcat(current_qso.comment, "00");
		    strcat(current_qso.comment, commentbuf);
		}

		if ((strlen(current_qso.comment) == 2) || (current_qso.comment[2] == ' ')) {
		    strcpy(commentbuf, current_qso.comment);
		    current_qso.comment[0] = '\0';
		    strcat(current_qso.comment, "0");
		    strcat(current_qso.comment, commentbuf);
		}

	    }

	    if (CONTEST_IS(SPRINT)) {

		if ((current_qso.comment[1] == ' ') && (current_qso.comment[0] != ' ')) {

		    strcpy(commentbuf, "00");
		    commentbuf[2] = current_qso.comment[0];
		    commentbuf[3] = '\0';
		    strcat(commentbuf, current_qso.comment + 1);
		    strcpy(current_qso.comment, commentbuf);
		}
		if ((current_qso.comment[2] == ' ') && (current_qso.comment[1] != ' ')) {

		    strcpy(commentbuf, "0");
		    commentbuf[1] = current_qso.comment[0];
		    commentbuf[2] = current_qso.comment[1];
		    commentbuf[3] = '\0';
		    strcat(commentbuf, current_qso.comment + 2);
		    strcpy(current_qso.comment, commentbuf);
		}

	    }

	    if (CONTEST_IS(PACC_PA) && (countrynr != my.countrynr)) {
		if (strlen(current_qso.comment) == 1) {
		    strcpy(commentbuf, current_qso.comment);
		    current_qso.comment[0] = '\0';
		    strcat(current_qso.comment, "00");
		    strcat(current_qso.comment, commentbuf);
		}

		if (strlen(current_qso.comment) == 2) {
		    strcpy(commentbuf, current_qso.comment);
		    current_qso.comment[0] = '\0';
		    strcat(current_qso.comment, "0");
		    strcat(current_qso.comment, commentbuf);
		}

	    }

	    if (CONTEST_IS(ARRL_SS) && (x != TAB) && (strlen(current_qso.section) < 2)) {
		mvaddstr(13, 54, "section?");
		mvaddstr(12, 54, current_qso.comment);
		x = 0;
	    } else if ((serial_section_mult || sectn_mult)
		       && ((x != TAB) && (strlen(current_qso.section) < 1))) {
		if (!serial_or_section
			|| (serial_or_section && country_found(current_qso.call))) {
		    mvaddstr(13, 54, "section?");
		    mvaddstr(12, 54, current_qso.comment);
		    refreshp();
		}
		break;

	    } else if (CONTEST_IS(STEWPERRY)) {
		if (check_qra(current_qso.comment) == 0) {
		    mvaddstr(13, 54, "locator?");
		    mvaddstr(12, 54, current_qso.comment);
		    break;
		}
		refreshp();
		break;
	    } else if (CONTEST_IS(CQWW) && trxmode == DIGIMODE && ((countrynr == w_cty)
		       || (countrynr == ve_cty))) {
		if (strlen(current_qso.comment) < 5) {
		    mvaddstr(13, 54, "state/prov?");
		    mvaddstr(12, 54, current_qso.comment);
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

    return x;
}


/* ------------------------------------------------------------------------ */

bool call_update = false;

/* ------------------------------------------------------------------------ */


static void checkexchange_cqww(struct qso_t *qso, bool interactive) {
    // <zone> [call_fix] [zone_fix]
    static const char *PATTERN =
	"\\s*(\\d+)?"       // zone
	"\\s*([A-Z0-9/]*?[A-Z]\\d+[A-Z]+[A-Z0-9/]*)?"  // call fix
	"\\s*(\\d+)?"       // zone fix
	"\\s*";

    static GRegex *regex = NULL;
    if (regex == NULL) {
	regex = g_regex_new(PATTERN, 0, 0, NULL);
    }

    int zone = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, qso->comment, 0, &match_info);
    if (g_match_info_matches(match_info)) {
	gchar *index;

	// get zone nr, use fix if available
	index = g_match_info_fetch(match_info, 1);
	gchar *index_fix = g_match_info_fetch(match_info, 3);
	if (index_fix != NULL && strlen(index_fix) >= 1 && strlen(index_fix) <= 4) {
	    g_free(index);
	    index = index_fix;
	} else {
	    g_free(index_fix);
	}

	if (index != NULL && strlen(index) >= 1 && strlen(index) <= 4) {
	    zone = atoi(index);
	}
	g_free(index);

	// get call fix
	index = g_match_info_fetch(match_info, 2);
	if (index != NULL) {
            g_strlcpy(qso->callupdate, index, MAX_CALL_LENGTH + 1);
	}
	g_free(index);
    }
    g_match_info_free(match_info);

    // multiplier: zone
    sprintf(qso->normalized_comment, "%02d", zone);
    g_strlcpy(qso->mult1_value, qso->normalized_comment, MULT_SIZE);

    if (interactive) {
	OnLowerSearchPanel(32, qso->normalized_comment); // show current zone
    }
}

static void checkexchange_arrlss(struct qso_t *qso, bool interactive) {
    char serial[5];
    char precedent[3];
    char check[3];

    static const char *PATTERN =
	"\\s*(\\d+)?"       // serial
	"\\s*([ABMSQU])?"   // precedent
	"\\s*([A-Z0-9]*?[A-Z]\\d+[A-Z]+(?:/\\d)?)?"  // call w/ optional region
	"\\s*(\\d+)?"       // check
	"\\s*([A-Z]{2,3})?" // section
	"\\s*";
    ;
    static GRegex *regex = NULL;
    if (regex == NULL) {
	regex = g_regex_new(PATTERN, 0, 0, NULL);
    }

    qso->section[0] = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, qso->comment, 0, &match_info);
    if (g_match_info_matches(match_info)) {
	gchar *index;

	// get serial nr.
	index = g_match_info_fetch(match_info, 1);
	if (index != NULL && strlen(index) >= 1 && strlen(index) <= 4) {
	    int s = atoi(index);
	    if (s != 0)
		snprintf(serial, sizeof(serial), "%4d", s);
	} else {
	    strcpy(serial, spaces(4));
	}
	g_free(index);

	// get precedent
	index = g_match_info_fetch(match_info, 2);
	if (index != NULL && index[0] != 0) {
	    strcpy(precedent, index);
	} else {
	    strcpy(precedent, spaces(1));
	}
	g_free(index);

	// get call update
	index = g_match_info_fetch(match_info, 3);
	if (index != NULL && strchr("AKNWVC", index[0]) != NULL) {  // US/CA only
            g_strlcpy(qso->callupdate, index, MAX_CALL_LENGTH + 1);
	}
	g_free(index);

	// get check
	index = g_match_info_fetch(match_info, 4);
	if (index != NULL && strlen(index) == 2) {  // only if 2 digits
	    strcpy(check, index);
	} else {
	    strcpy(check, spaces(2));
	}
	g_free(index);

	// get section
	index = g_match_info_fetch(match_info, 5);
	if (index != NULL && index[0] != 0) {
	    if (get_exact_mult_index(index) >= 0) {
		g_strlcpy(qso->section, index, MAX_SECTION_LENGTH + 1);
	    }
	}
	g_free(index);

    }
    g_match_info_free(match_info);

    if (interactive) {
	char buf[40];
	sprintf(buf, " %4s %1s %2s %2s ", serial, precedent,
		check, qso->section);
	OnLowerSearchPanel(8, buf);
    }

    sprintf(qso->normalized_comment, "%s %s %s %s", serial, precedent, check, qso->section);
    g_strlcpy(qso->mult1_value, qso->section, MULT_SIZE);   // multiplier: section
}

static void checkexchange_serial_section(struct qso_t *qso, bool interactive) {
    char serial[5] = "";

    static const char *PATTERN =
	"\\s*(\\d+)?"           // serial
	"\\s*(\\d*[A-Z]+\\d*)?" // section ([digits] letters [digits])
	"\\s*([A-Z0-9/]*?[A-Z]\\d+[A-Z]+[A-Z0-9/]*)?"  // call fix
	"\\s*";
    ;
    static GRegex *regex = NULL;
    if (regex == NULL) {
	regex = g_regex_new(PATTERN, 0, 0, NULL);
    }

    qso->section[0] = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, qso->comment, 0, &match_info);
    if (g_match_info_matches(match_info)) {
	gchar *index;

	// get serial nr.
	index = g_match_info_fetch(match_info, 1);
	if (index != NULL && strlen(index) >= 1 && strlen(index) <= 4) {
	    int s = atoi(index);
	    if (s != 0) {
		snprintf(serial, sizeof(serial), "%4d", s);
	    }
	}
	g_free(index);

	// get section
	index = g_match_info_fetch(match_info, 2);
	if (index != NULL && index[0] != 0) {
	    if (serial_grid4_mult || get_exact_mult_index(index) >= 0) {
		g_strlcpy(qso->section, index, MAX_SECTION_LENGTH + 1);
	    }
	}
	g_free(index);

	// get call update
	index = g_match_info_fetch(match_info, 3);
	if (index != NULL) {
            g_strlcpy(qso->callupdate, index, MAX_CALL_LENGTH + 1);
	}
	g_free(index);
    }
    g_match_info_free(match_info);

    if (serial_grid4_mult) {
        if (!check_qra(qso->section)) {
            qso->section[0] = 0;
        }
        if (strlen(qso->section) > 4) {
            qso->section[4] = 0;     // mult is the first 4 chars only
        }
    }

    if (interactive) {
	char buf[40];
	sprintf(buf, " %*s ", -MAX_SECTION_LENGTH, qso->section);
	OnLowerSearchPanel(32, buf);
    }

    if (serial[0] && qso->section[0]) {
	sprintf(qso->normalized_comment, "%s %s", serial, qso->section);
	g_strlcpy(qso->mult1_value, qso->section, MULT_SIZE);   // multiplier: section
    }
}

static void checkexchange_sectn_mult(struct qso_t *qso, bool interactive) {
    static const char *PATTERN =
	"\\s*(\\d*[A-Z]+\\d*)?" // section ([digits] letters [digits])
	"\\s*([A-Z0-9/]*?[A-Z]\\d+[A-Z]+[A-Z0-9/]*)?"  // call fix
	"\\s*";
    ;
    static GRegex *regex = NULL;
    if (regex == NULL) {
	regex = g_regex_new(PATTERN, 0, 0, NULL);
    }

    qso->section[0] = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, qso->comment, 0, &match_info);

    if (g_match_info_matches(match_info)) {
	gchar *index;

	// get section
	index = g_match_info_fetch(match_info, 1);
	if (index != NULL && index[0] != 0) {
	    if (get_exact_mult_index(index) >= 0) {
		g_strlcpy(qso->section, index, MAX_SECTION_LENGTH + 1);
	    }
	}
	g_free(index);

	// get call update
	index = g_match_info_fetch(match_info, 2);
	if (index != NULL) {
            g_strlcpy(qso->callupdate, index, MAX_CALL_LENGTH + 1);
	}
	g_free(index);
    }
    g_match_info_free(match_info);

    if (interactive) {
	char buf[40];
	sprintf(buf, " %*s ", -MAX_SECTION_LENGTH, qso->section);
	OnLowerSearchPanel(32, buf);
    }

    if (qso->section[0]) {
	g_strlcpy(qso->normalized_comment, qso->section, COMMENT_SIZE);
	g_strlcpy(qso->mult1_value, qso->section, MULT_SIZE);   // multiplier: section
    }
}

/* ------------------------------------------------------------------------ */
/*
    input: comment, interactive
    output (qso): callupdate, normalized_comment, section, mult1_value
    side effect: lower line of search panel updated if interactive
*/

void checkexchange(struct qso_t *qso, bool interactive) {
    // create fields
    if (qso->callupdate == NULL) {
        qso->callupdate = g_malloc0(MAX_CALL_LENGTH + 1);
    }
    if (qso->normalized_comment == NULL) {
        qso->normalized_comment = g_malloc0(COMMENT_SIZE);
    }
    if (qso->section == NULL) {
        qso->section = g_malloc0(MAX_SECTION_LENGTH + 1);
    }
    if (qso->mult1_value == NULL) {
        qso->mult1_value = g_malloc0(MULT_SIZE);
    }

    qso->callupdate[0] = 0;
    qso->normalized_comment[0] = 0;
    qso->mult1_value[0] = 0;

    // ----------------------------cqww------------------------------
    if (CONTEST_IS(CQWW)) {

	checkexchange_cqww(qso, interactive);
	return;
    }

    // ---------------------------arrls------------------------------
    if (CONTEST_IS(ARRL_SS)) {

	checkexchange_arrlss(qso, interactive);
	return;
    }

    // ----------------------serial+section--------------------------
    if (serial_section_mult || serial_grid4_mult) {

	checkexchange_serial_section(qso, interactive);
	return;
    }

    // ----------------------section only----------------------------
    if (sectn_mult || sectn_mult_once || dx_arrlsections) {

	checkexchange_sectn_mult(qso, interactive);
	return;
    }

}


/* ------------------------------------------------------------------------ */
/** Edit exchange field
 */

void exchange_edit(void) {

    int l, b;
    int i = 0, j;
    char comment2[27];

    l = strlen(current_qso.comment);
    b = l - 1;
    while ((i != ESCAPE) && (b <= strlen(current_qso.comment))) {
	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvaddstr(12, 54, spaces(contest->exchange_width));
	mvaddstr(12, 54, current_qso.comment);
	move(12, 54 + b);

	i = key_get();

	// Ctrl-A (^A) or <Home>, move to beginning of comment field.
	if (i == CTRL_A || i == KEY_HOME) {

	    b = 0;

	    // Ctrl-E (^E) or <End>, move to end of comment field, exit edit mode.
	} else if (i == CTRL_E || i == KEY_END) {

	    b = strlen(current_qso.comment);
	    break;

	    // Left arrow, move cursor left one position.
	} else if (i == KEY_LEFT) {

	    if (b > 0)
		b--;

	    // Right arrow, move cursor right one position.
	} else if (i == KEY_RIGHT) {

	    if (b < strlen(current_qso.comment) - 1) {
		b++;
	    } else
		break;		/* stop edit */

	    // <Delete>, erase character under the cursor,
	    // shift all characters to the right of the cursor left one position.
	} else if (i == KEY_DC) {

	    l = strlen(current_qso.comment);

	    for (j = b; j <= l; j++) {
		current_qso.comment[j] = current_qso.comment[j + 1];	/* move to left incl.\0 */
	    }

	    // <Backspace>, erase character to the left of the cursor,
	    // shift all characters to the right of the cursor left one position.
	} else if (i == KEY_BACKSPACE) {

	    if (b > 0) {
		b--;

		l = strlen(current_qso.comment);

		for (j = b; j <= l; j++) {
		    current_qso.comment[j] = current_qso.comment[j + 1];
		}
	    }

	    // <Escape> not received.
	} else if (i != ESCAPE) {

	    // Promote lower case to upper case.
	    if ((i >= 'a') && (i <= 'z'))
		i = i - 32;

	    // Accept printable characters.
	    if ((i >= ' ') && (i <= 'Z')) {

		if (strlen(current_qso.comment) < contest->exchange_width) {
		    /* copy including trailing \0 */
		    strncpy(comment2, current_qso.comment + b, strlen(current_qso.comment) - (b - 1));

		    current_qso.comment[b] = i;
		    current_qso.comment[b + 1] = '\0';
		    strcat(current_qso.comment, comment2);

		    b++;
		}

	    } else if (i != 0)
		i = ESCAPE;
	}
    }

    attron(A_STANDOUT);
    refresh_comment();
}
