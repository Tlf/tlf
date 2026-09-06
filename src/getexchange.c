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
#include "bands.h"
#include "cw_utils.h"
#include "change_rst.h"
#include "cleanup.h"
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
#include "plugin.h"

#include "getexchange.h"


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


static void align_serial_number() {
    /* length of serial part in "001" or "001 EU-001" */
    int nr_len = strspn(current_qso.comment, "0123456789");
    if (nr_len == 0 || nr_len > 2) {
	return;     // empty or long enough
    }

    int pad_length = 3 - nr_len;

    /* prepend zeros */
    for (int i = 0; i < pad_length; ++i) {
	insert_char('0', current_qso.comment, 0, contest->exchange_width);
    }
}


int getexchange(void) {

    int x = 0;

    if (lan_active && contest->exchange_serial) {
	strncpy(lastqsonr, qsonrstr, 5);
	send_lan_message(INCQSONUM, qsonrstr);
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

    current_qso.band = bandindex2nr(bandinx); //FIXME drop global bandinx

    int pos = strlen(current_qso.comment);

    /* parse input and modify exchange field accordingly */

    commentfield = 1;

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
	    wmove(stdscr, 12, 54 + pos);
	    x = key_poll();
	}

	x = ascii_toupper(x);
	x = handle_common_key(x);

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
	    case CTRL_S: {	// Ctl+s (^S)--Open QTC panel for sending QTCs
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
		if (pos >= 1) {
		    --pos;
		    delete_char(current_qso.comment, pos);
		}
		break;
	    }

	    case CTRL_U:
		/* wipe out or restore call input and comment field */
		if (current_qso.call[0] != '\0' ||
			current_qso.comment[0] != '\0') {
		    /* wipe out any content */
		    cleanup_hiscall();
		    cleanup_comment();
		    rst_reset();

		    x = TAB;	/* back to call input field */

		}

		break;

	    case CTRL_W: {
		/* wipe out or restore exchange field */
		if (current_qso.comment[0] != '\0') {
		    cleanup_comment();
		} else {
		    restore_comment();
		}

		pos = strlen(current_qso.comment);

		break;
	    }


	    case ESCAPE: {                // <Escape>
		stoptx();			/* stop sending CW */
		if (!stop_tx_only) {
		    if (current_qso.comment[0] != '\0') {	/* if comment not empty */
			/* drop exchange so far */
			cleanup_comment();
		    } else {
			/* back to callinput */
			x = TAB;	// <Tab>
		    }
		}
		break;
	    }

	    // Underscore, confirm last exchange.
	    case '_': {
		if (S_P == cqmode) {
		    send_standard_message_prev_qso(SP_TU_MSG);
		} else {
		    send_standard_message_prev_qso(2);
		}

		break;
	    }

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

	    /* <Home>--move cursor to the beginning of exchange field */
	    case KEY_HOME: {
		pos = 0;
		break;
	    }

	    /* Ctrl-E (^E) or <End>, move to the end of exchange field */
	    case CTRL_E:
	    case KEY_END: {
		pos = strlen(current_qso.comment);
		break;
	    }

	    // <Delete>
	    case KEY_DC: {
		delete_char(current_qso.comment, pos);
		break;
	    }

	    case KEY_LEFT: {	/* Left Arrow--move cursor left */
		if (pos > 0) {
		    --pos;
		}
		break;
	    }

	    case KEY_RIGHT: {	/* Right Arrow--move cursor right */
		if (pos < strlen(current_qso.comment)) {
		    ++pos;
		}
		break;
	    }

	    case KEY_UP:	/* Up/Down--increase/decrease serial number */
	    case KEY_DOWN: {
		serial_up_down(current_qso.comment, (x == KEY_UP) ? 1 : -1);
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

	/* normal character -> insert if space left */
	if (strlen(current_qso.comment) < contest->exchange_width) {
	    if (x >= ' ' && x <= 'Z') {
		pos = insert_char(x, current_qso.comment, pos, contest->exchange_width);
	    }
	}

	/* <Enter>, <Tab>, Ctl-K, '\' */
	if (x == '\n' || x == KEY_ENTER || x == TAB
		|| x == CTRL_K || x == BACKSLASH) {

	    if (contest->exchange_serial && current_qso.comment[0] >= '0'
		    && current_qso.comment[0] <= '9') {	/* align serial nr. */

		align_serial_number();

	    }

	    if (CONTEST_IS(WPX)) {	/* align serial nr. */
		align_serial_number();
	    }

	    if (CONTEST_IS(SPRINT)) {
		align_serial_number();
	    }

	    if (CONTEST_IS(PACC_PA) && (countrynr != my.countrynr)) {
		align_serial_number();
	    }

	    if (CONTEST_IS(ARRL_SS) && (x != TAB) && (strlen(current_qso.section) < 2)) {
		mvaddstr(13, 54, "section?");
		mvaddstr(12, 54, current_qso.comment);
		x = 0;
	    } else if ((serial_section_mult || sectn_mult != MULT_NONE)
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

    sprintf(qso->normalized_comment, "%s %s %s %s", serial, precedent, check,
	    qso->section);
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

    if (plugin_has_check_exchange()) {
	plugin_check_exchange(qso);
	return;
    }

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
    if (sectn_mult != MULT_NONE || dx_arrlsections) {

	checkexchange_sectn_mult(qso, interactive);
	return;
    }

}
