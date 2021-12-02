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
#include "callinput.h"
#include "cw_utils.h"
#include "change_rst.h"
#include "globalvars.h"
#include "keyer.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "locator2longlat.h"
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


char callupdate[MAX_CALL_LENGTH + 1];

void exchange_edit(void);

int getexchange(void) {

    int i;
    int x = 0;
    char instring[2];
    char commentbuf[40] = "";
    char *gridmult = "";

    instring[1] = '\0';

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
	    && (*comment == '\0') && (strlen(hiscall) != 0)) {
	if (itumult)
	    strcpy(comment, ituzone);
	else
	    strcpy(comment, cqzone);
    }
    if ((exc_cont) && (*comment == '\0')
	    && (strlen(hiscall) != 0)) {
	strcpy(comment, continent);
    }

    if (CONTEST_IS(STEWPERRY)) {
	recall_exchange();
    }

    /* parse input and modify exchange field accordingly */

    commentfield = 1;

    i = strlen(comment);
    while (1) {

	refresh_comment();

	checkexchange(comment, true);

	if (call_update && strlen(callupdate) >= 3) {
	    strcpy(hiscall, callupdate);
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
		if (ctcomp && (strlen(hiscall) > 2)) {
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

	if (i < contest->exchange_width) {  /* normal character -> insert if space left */
	    if (x >= ' ' && x <= 'Z') {
		instring[0] = x;
		addch(x);
		strcat(comment, instring);
		i++;
		refreshp();
	    }
	}

	/* <Enter>, <Tab>, Ctl-K, '\' */
	if (x == '\n' || x == KEY_ENTER || x == TAB
		|| x == CTRL_K || x == BACKSLASH) {

	    if ((contest->exchange_serial && comment[0] >= '0'
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

	    if (CONTEST_IS(WPX)) {	/* align serial nr. */

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

	    if (CONTEST_IS(SPRINT)) {

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

	    if (CONTEST_IS(PACC_PA) && (countrynr != my.countrynr)) {
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

	    if (CONTEST_IS(ARRL_SS) && (x != TAB) && (strlen(section) < 2)) {
		mvaddstr(13, 54, "section?");
		mvaddstr(12, 54, comment);
		x = 0;
	    } else if ((serial_section_mult || sectn_mult)
		       && ((x != TAB) && (strlen(section) < 1))) {
		if (!serial_or_section
			|| (serial_or_section && country_found(hiscall))) {
		    mvaddstr(13, 54, "section?X");
		    mvaddstr(12, 54, comment);
		    refreshp();
		}
		break;

	    } else if (serial_grid4_mult) {
		//      mvaddstr(13,54, "section?");
		mvaddstr(12, 54, comment);
		refreshp();
		gridmult = getgrid(comment);
		strcpy(section, gridmult);
		section[4] = '\0';

		break;

	    } else if (CONTEST_IS(STEWPERRY)) {
		if (check_qra(comment) == 0) {
		    mvaddstr(13, 54, "locator?");
		    mvaddstr(12, 54, comment);
		    break;
		}
		refreshp();
		break;
	    } else if (CONTEST_IS(CQWW) && trxmode == DIGIMODE && ((countrynr == w_cty)
		       || (countrynr == ve_cty))) {
		if (strlen(comment) < 5) {
		    mvaddstr(13, 54, "state/prov?");
		    mvaddstr(12, 54, comment);
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

char section[MAX_SECTION_LENGTH + 1] = "";
bool call_update = false;

/* ------------------------------------------------------------------------ */


static void checkexchange_cqww(char *comment, bool interactive) {
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
    g_regex_match(regex, comment, 0, &match_info);
    if (g_match_info_matches(match_info)) {
	gchar *index;

	// get zone nr, use fix if available
	index = g_match_info_fetch(match_info, 1);
	gchar *index_fix = g_match_info_fetch(match_info, 3);
	if (index_fix != NULL && strlen(index_fix) >= 1 && strlen(index_fix) <= 4) {
	    g_free(index);
	    index = index_fix;
	}
	if (index != NULL && strlen(index) >= 1 && strlen(index) <= 4) {
	    zone = atoi(index);
	}
	g_free(index);

	// get call fix
	index = g_match_info_fetch(match_info, 2);
	if (index != NULL) {
	    g_strlcpy(callupdate, index, sizeof(callupdate));
	}
	g_free(index);
    }
    g_match_info_free(match_info);

    // multiplier: zone
    sprintf(normalized_comment, "%02d", zone);
    g_strlcpy(mult1_value, normalized_comment, sizeof(normalized_comment));

    if (interactive) {
	OnLowerSearchPanel(32, normalized_comment); // show current zone
    }
}

static void checkexchange_arrlss(char *comment, bool interactive) {
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

    section[0] = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, comment, 0, &match_info);
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
	    g_strlcpy(callupdate, index, sizeof(callupdate));
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
		g_strlcpy(section, index, sizeof(section));
	    }
	}
	g_free(index);

    }
    g_match_info_free(match_info);

    if (interactive) {
	char buf[40];
	sprintf(buf, " %4s %1s %2s %2s ", serial, precedent,
		check, section);
	OnLowerSearchPanel(8, buf);
    }

    sprintf(normalized_comment, "%s %s %s %s", serial, precedent, check, section);
    g_strlcpy(mult1_value, section, sizeof(section));   // multiplier: section
}

static void checkexchange_serial_section(char *comment, bool interactive) {
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

    section[0] = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, comment, 0, &match_info);
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
	    if (get_exact_mult_index(index) >= 0) {
		g_strlcpy(section, index, sizeof(section));
	    }
	}
	g_free(index);

	// get call update
	index = g_match_info_fetch(match_info, 3);
	if (index != NULL) {
	    g_strlcpy(callupdate, index, sizeof(callupdate));
	}
	g_free(index);
    }
    g_match_info_free(match_info);

    if (interactive) {
	char buf[40];
	sprintf(buf, " %*s ", -MAX_SECTION_LENGTH, section);
	OnLowerSearchPanel(32, buf);
    }

    if (serial[0] && section[0]) {
	sprintf(normalized_comment, "%s %s", serial, section);
	g_strlcpy(mult1_value, section, sizeof(mult1_value));   // multiplier: section
    }
}

static void checkexchange_sectn_mult(char *comment, bool interactive) {
    static const char *PATTERN =
	"\\s*(\\d*[A-Z]+\\d*)?" // section ([digits] letters [digits])
	"\\s*([A-Z0-9/]*?[A-Z]\\d+[A-Z]+[A-Z0-9/]*)?"  // call fix
	"\\s*";
    ;
    static GRegex *regex = NULL;
    if (regex == NULL) {
	regex = g_regex_new(PATTERN, 0, 0, NULL);
    }

    section[0] = 0;

    GMatchInfo *match_info;
    g_regex_match(regex, comment, 0, &match_info);

    if (g_match_info_matches(match_info)) {
	gchar *index;

	// get section
	index = g_match_info_fetch(match_info, 1);
	if (index != NULL && index[0] != 0) {
	    if (get_exact_mult_index(index) >= 0) {
		g_strlcpy(section, index, sizeof(section));
	    }
	}
	g_free(index);

	// get call update
	index = g_match_info_fetch(match_info, 2);
	if (index != NULL) {
	    g_strlcpy(callupdate, index, sizeof(callupdate));
	}
	g_free(index);
    }
    g_match_info_free(match_info);

    if (interactive) {
	char buf[40];
	sprintf(buf, " %*s ", -MAX_SECTION_LENGTH, section);
	OnLowerSearchPanel(32, buf);
    }

    if (section[0]) {
	g_strlcpy(normalized_comment, section, sizeof(normalized_comment));
	g_strlcpy(mult1_value, section, sizeof(mult1_value));   // multiplier: section
    }
}

/* ------------------------------------------------------------------------ */
/*
    input: comment, interactive
    output (global vars): section, callupdate, mult1_value, normalized_comment
    side effect: lower line of search panel updated if interactive
*/

void checkexchange(char *comment, bool interactive) {

    callupdate[0] = 0;
    normalized_comment[0] = 0;
    mult1_value[0] = 0;

    // ----------------------------cqww------------------------------
    if (CONTEST_IS(CQWW)) {

	checkexchange_cqww(comment, interactive);
	return;
    }

    // ---------------------------arrls------------------------------
    if (CONTEST_IS(ARRL_SS)) {

	checkexchange_arrlss(comment, interactive);
	return;
    }

    // ----------------------serial+section--------------------------
    if (serial_section_mult) {

	checkexchange_serial_section(comment, interactive);
	return;
    }

    // ----------------------section only----------------------------
    if (sectn_mult || sectn_mult_once || dx_arrlsections) {

	checkexchange_sectn_mult(comment, interactive);
	return;
    }

}


/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
 * return a pointer to the start of grid locator
 */

char *getgrid(char *comment) {

    int multposition = 0;
    int i = 0;

    /* search for first letter, that should be the start of the Grid locator*/
    for (i = 0; i < strlen(comment); i++) {
	if (isalpha(comment[i])) {
	    multposition = i;
	    break;
	}
    }

    return comment + multposition;
}

/* ------------------------------------------------------------------------ */
/** Edit exchange field
 */

void exchange_edit(void) {

    int l, b;
    int i = 0, j;
    char comment2[27];

    l = strlen(comment);
    b = l - 1;
    while ((i != ESCAPE) && (b <= strlen(comment))) {
	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvaddstr(12, 54, spaces(contest->exchange_width));
	mvaddstr(12, 54, comment);
	move(12, 54 + b);

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

		if (strlen(comment) < contest->exchange_width) {
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
