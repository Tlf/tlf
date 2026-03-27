/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2005 Rein Couperus <pa0r@eudxf.org>
 *               2009-2016 Thomas Beierlein <tb@forth-ev.de>
 *               2013      Ervin Hegedüs - HA2OS <airween@gmail.com>
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
 *        Callinput handles the call input field
 *
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "addspot.h"
#include "audio.h"
#include "autocq.h"
#include "bandmap.h"
#include "calledit.h"
#include "callinput.h"
#include "change_rst.h"
#include "changefreq.h"
#include "changepars.h"
#include "clear_display.h"
#include "cleanup.h"
#include "cqww_simulator.h"
#include "cw_utils.h"
#include "edit_last.h"
#include "err_utils.h"
#include "deleteqso.h"
#include "getctydata.h"
#include "gettxinfo.h"
#include "grabspot.h"
#include "ignore_unused.h"
#include "keyer.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "muf.h"
#include "netkeyer.h"
#include "nicebox.h"		// Includes curses.h
#include "note.h"
#include "printcall.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "qtcwin.h"
#include "rtty.h"
#include "searchcallarray.h"
#include "searchlog.h"		// Includes glib.h
#include "sendbuf.h"
#include "sendspcall.h"
#include "setcontest.h"
#include "show_help.h"
#include "showinfo.h"
#include "showpxmap.h"
#include "speedupndown.h"
#include "splitscreen.h"
#include "stoptx.h"
#include "time_update.h"
#include "trx_memory.h"
#include "ui_utils.h"
#include "utils.h"
#include "showzones.h"
#include "bands.h"
#include "fldigixmlrpc.h"

typedef enum { STORE_OR_POP, POP, SWAP } memory_op_t;

void send_bandswitch(freq_t freq);
int autosend(void);
void handle_bandswitch(int direction);
void handle_memory_operation(memory_op_t op);



/** callsign input loop
 *
 * \return code of last typed character */
int callinput(void) {

    static freq_t freqstore;		/* qrg during last callsign input
					   character, 0 if grabbed,
					   used to decide if a call in input
					   field was entered by hand or
					   grabbed from spot list */

    enum grabstate_t { NONE, IN_PROGRESS, REACHED };

    struct grab_t {
	enum grabstate_t state;
	freq_t spotfreq;
	char call[CALL_SIZE];
    };

    static struct grab_t grab =  { .state = NONE };


    int cury, curx;
    int j, t, x = 0;
    char instring[2] = { '\0', '\0' };
    static int lastwindow;

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    printcall();	/* print call input field */
    searchlog();

    while (strlen(current_qso.call) <= MAX_CALL_LENGTH) {

	show_zones(bandinx);
	update_info_line();
	searchlog();
	printcall();    // note: calls refreshp()

	/* wait for next char pressed, but update time, cluster and TRX qrg */
	/* main loop waiting for input */
	x = -1;
	while (x < 1) {

	    usleep(10000);

	    time_update();

	    if (trxmode == DIGIMODE) {
		show_rtty();
	    }

	    if (digikeyer == FLDIGI && fldigi_set_callfield
		    && current_qso.call[0] != '\0') {
		freqstore = freq;
		fldigi_set_callfield = false;
		// call has been just set, restart outer loop to update display
		break;
	    }

	    /* if BMAUTOADD is active and user has input a call sign
	     * (indicated by non-zero freqstore) check if he turns away
	     * from frequency and if so add call to spot list */
	    if (bmautoadd && freqstore != 0) {
		if (strlen(current_qso.call) >= 3) {
		    if (fabs(freq - freqstore) > 500) {
			add_to_spots(current_qso.call, freqstore);
			current_qso.call[0] = '\0';
			freqstore = 0;
			break;
		    }
		}
	    }

	    /* if BMAUTOGRAB is active in S&P mode and input field is empty and a spot has
	     * not already been grabbed here check if a spot is on freq
	     * and pick it up if one found */
	    if (bmautograb && cqmode == S_P && *current_qso.call == '\0'
		    && grab.state == NONE) {
		get_spot_on_qrg(grab.call, freq);
		if (strlen(grab.call) >= 3) {
		    g_strlcpy(current_qso.call, grab.call, CALL_SIZE);
		    grab.state = REACHED;
		    grab.spotfreq = freq;
		    freqstore = 0;
		    break;
		}
	    }

	    /* wait till freq of grabbed spot is reported back from rig.
	     * Then go to 'reached' state' */
	    if (grab.state == IN_PROGRESS && fabs(freq - grab.spotfreq) <= 100)
		grab.state = REACHED;

	    /* Check if we tune away from old freq before a grabbed spot is
	     * reached. If so stop grab process */

	    /* if we have grabbed a call from spot list and tune away
	     * then forget about it */
	    if (fabs(freq - grab.spotfreq) > 500 && grab.state == REACHED) {
		grab.state = NONE;
		current_qso.call[0] = '\0';
		break;
	    }


	    /* make sure that the wrefresh() inside getch() shows the cursor
	     * in the input field */
	    wmove(stdscr, 12, 29 + strlen(current_qso.call));
	    x = key_poll();

	}

	/* special handling of some keycodes if call field is empty */
	if (*current_qso.call == '\0') {
	    // <Enter>, sends CQ message (F1), starts autoCQ, or sends S&P message.
	    if (x == '\n' || x == KEY_ENTER) {
		if (cqmode == CQ) {
		    if (!noautocq)
			x = auto_cq();
		} else {
		    sendspcall();
		    continue;
		}
	    }

	    // Up Arrow or Alt-e, edit last QSO
	    if (x == KEY_UP || x == ALT_E) {
		edit_last();
		continue;
	    }

	    // Equals, confirms last callsign already logged if call field is empty.
	    if (x == '=' && lastcall[0] != 0) {
		char *str = g_strdup_printf("%s TU ", lastcall);
		sendmessage(str);
		g_free(str);
		continue;
	    }
	}

	// Shift-F1: switch back (swap) to CQ frequency if it's in memory
	//           and start a CQ
	if (x == SHIFT_F(1)) {
	    if (cqmode == S_P && memory_get_cqmode() == CQ) {
		handle_memory_operation(SWAP);
	    }

	    x = KEY_F(1);   // continue as F1
	}

	x = handle_common_key(x);

	switch (x) {
	    // Ctrl-V: toggle grab direction
	    case CTRL_V:
		grab_up = !grab_up;
		break;

	    // Plus (+)
	    // - in non-CT mode switch to other mode (CQ <-> S&P)
	    // - in CT mode send TU and log QSO.
	    case '+': {
		if (!ctcomp) {

		    /* switch to other mode */
		    if (cqmode == CQ) {
			cqmode = S_P;
		    } else
			cqmode = CQ;

		    /* and show new mode */
		    show_header_line();

		} else {

		    if (strlen(current_qso.call) > 2) {
			if ((CONTEST_IS(CQWW) || wazmult)
				&& current_qso.comment[0] == '\0')
			    strcpy(current_qso.comment, cqzone);

			if (itumult && current_qso.comment[0] == '\0')
			    strcpy(current_qso.comment, ituzone);

			if (current_qso.comment[0] == '\0') {
			    x = -1;
			} else {
			    /* F4 (TU macro) */
			    send_standard_message(3);

			    x = '\\';   // key for logging QSO without message
			}
		    }
		}
		break;
	    }

	    // Ctrl-Q (^Q), open QTC window for receiving or sending QTCs.
	    case CTRL_Q: {
		if (qtcdirection == 1 || qtcdirection == 3) {	// in case of QTC=RECV or QTC=BOTH
		    qtc_main_panel(RECV);
		}
		if (qtcdirection == 2) {			// in case of QTC=SEND
		    qtc_main_panel(SEND);
		}
		x = KEY_LEFT;
		continue;
	    }

	    // Ctrl-S (^S), open QTC window for sending QTCs.
	    case CTRL_S: {
		if (qtcdirection == 2 || qtcdirection == 3) {	// in case of QTC=SEND or QTC=BOTH
		    qtc_main_panel(SEND);
		}
		x = KEY_LEFT;
		continue;
	    }

	    // <Home>, enter call edit when call field is not empty.
	    case KEY_HOME: {
		if ((*current_qso.call != '\0') && (ungetch(x) == OK)) {
		    x = calledit(); // pass through KEY_ENTER and friends from editing
		}

		break;
	    }

	    // Left Arrow, enter call edit when call field is not empty, or band down.
	    case KEY_LEFT: {
		if (*current_qso.call != '\0') {
		    x = calledit(); // pass through KEY_ENTER and friends from editing
		} else {
		    handle_bandswitch(BAND_DOWN);
		}

		break;
	    }

	    // Right Arrow, band up when call field is empty.
	    case KEY_RIGHT: {
		handle_bandswitch(BAND_UP);
		break;
	    }

	    // Alt-v (M-v), change Morse speed in CW mode, else band down.
	    case ALT_V: {
		if (ctcomp) {
		    while (x != ESCAPE) {
			nicebox(1, 1, 2, 12, "Cw");
			attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
			mvprintw(2, 2, "Speed:   %2u ", speed);
			mvprintw(3, 2, "Weight: %3d ", weight);
			printcall();
			refreshp();

			x = key_get();
			if (x == KEY_UP) {
			    speedup();
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

			    mvprintw(0, 14, "%2u", speed);

			} else if (x == KEY_DOWN) {
			    speeddown();
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
			    mvprintw(0, 14, "%2u", speed);

			} else
			    x = ESCAPE;

			clear_display();
		    }
		} else {	// trlog compatible, band switch
		    handle_bandswitch(BAND_DOWN);
		}
		x = -1;

		break;
	    }

	    // <Enter>, log QSO in CT mode, else test if B4 message should be sent.
	    case '\n':
	    case KEY_ENTER: {
		if (strlen(current_qso.call) > 2 && ctcomp) {
		    /* There seems to be a call, log it in CT mode but only if
		     * the exchange field is not empty.
		     */
		    if (current_qso.comment[0] == '\0') {
			x = -1;
			break;
		    } else {
			/* Log without sending message. */
			x = BACKSLASH;
			break;
		    }
		}

		if (strlen(current_qso.call) < 3 || nob4)
		    break;

		/* check b4 QSO if call is long enough and 'nob4' off */

		dupe = is_dupe(current_qso.call, bandinx, trxmode);

		if (dupe == ISDUPE) {
		    // XXX: Before digi_message, SSB mode sent CW here. - W8BSD
		    send_standard_message(6);	/* as with F7 */
		    cleanup();
		    clear_display();
		}
		break;
	    }

	    /* <Insert>, send exchange in CT mode */
	    case KEY_IC: {
		if (ctcomp) {
		    /* F3 (RST macro) */
		    send_standard_message(2);
		    /* Set to space to move cursor to exchange field
		     * which will trigger autofill if available.
		     */
		    x = ' ';
		}
		break;
	    }

	    // Colon, prefix for entering commands or changing parameters.
	    case ':': {
		changepars();
		current_qso.call[0] = '\0';
		x = 0;
		clear_display();
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

		for (j = 13; j <= 23; j++) {
		    clear_line(j);
		}

		attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

		mvaddstr(12, 29, spaces(12));
		move(12, 29);
		refreshp();
		break;
	    }

	    // Hash, save xcvr freq to mem or restore mem to xcvr.
	    case '#': {
		handle_memory_operation(STORE_OR_POP);
		break;
	    }
	    // Dollar, pop memory (MEM -> TRX)
	    case '$': {
		handle_memory_operation(POP);
		break;
	    }
	    // Percent, swap memory (MEM <-> TRX)
	    case '%': {
		handle_memory_operation(SWAP);
		break;
	    }

	    // Minus, delete previous QSO from log.
	    case '-': {
		delete_qso();
		break;
	    }

	    // Semicolon or Alt-n (M-n), insert note in log.
	    case ';':
	    case ALT_N: {
		include_note();
		x = -1;
		break;
	    }

	    // F12, activate autocq timing and message.
	    case KEY_F(12): {
		x = auto_cq();
		break;
	    }

	    // Query, send call with " ?" appended or F5 message in voice mode.
	    case '?': {
		if (*current_qso.call != '\0') {
		    strcat(current_qso.call, " ?");
		    send_standard_message(4);
		    current_qso.call[strlen(current_qso.call) - 2] = '\0';
		}
		x = -1;
		break;
	    }

	    // <Backspace>, remove character left of cursor, move cursor left one position.
	    case KEY_BACKSPACE: {
		if (*current_qso.call != '\0') {
		    getyx(stdscr, cury, curx);
		    mvaddstr(cury, curx - 1, " ");
		    move(cury, curx - 1);
		    current_qso.call[strlen(current_qso.call) - 1] = '\0';
		}
		break;
	    }

	    // Alt-r (M-r) or Alt-s (M-s), toggle score window.
	    case ALT_R:
	    case ALT_S: {
		showscore_flag = !showscore_flag;
		clear_display();
		break;
	    }

	    // Alt-k (M-k), synonym for Ctrl-K (^K).
	    case ALT_K: {
		x = CTRL_K;		// Ctrl-K
		break;
	    }

	    // Alt-a (M-a), cycle cluster window.
	    case ALT_A: {
		if (cluster == NOCLUSTER) {
		    cluster = CLUSTER;	// alt-A
		    announcefilter = FILTER_ALL;
		} else if (cluster == CLUSTER) {
		    cluster = MAP;
		} else if (cluster == MAP) {
		    cluster = NOCLUSTER;
		}

		break;
	    }

	    // Alt-b (M-b), band-up for TR-Log mode.
	    case ALT_B: {
		if (!ctcomp) {
		    handle_bandswitch(BAND_UP);
		}
		break;
	    }

	    // Alt-j (M-j), show station frequencies.
	    case ALT_J: {
		if (cluster != FREQWINDOW) {
		    lastwindow = cluster;
		    cluster = FREQWINDOW;
		} else
		    cluster = lastwindow;

		break;
	    }

	    // Alt-h (M-h), show help.
	    case ALT_H: {
		show_help();
		break;
	    }

	    // Alt-, (M-,) or period, change bandmap filter config.
	    case 172:
	    case '.': {
		bm_menu();
		break;
	    }

	    // Alt-c (M-c), toggle check window.
	    case ALT_C: {
		searchflg = !searchflg;
		break;
	    }

	    // Alt-m (M-m), show multipliers.
	    case ALT_M: {
		show_mults();
		refreshp();
		break;
	    }

	    // Alt-p (M-p), toggle PTT via cwdaemon
	    case ALT_P: {
		if (k_ptt == 0) {
		    k_ptt = 1;
		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvaddstr(0, 2, "PTT on   ");
		    move(12, 29);
		    refreshp();
		    netkeyer(K_PTT, "1");	// ptt on
		    x = key_get();	// any character to stop tuning
		    if (x == ALT_P)	// Alt-P (M-p)
			netkeyer(K_PTT, "0");	// ptt off
		    k_ptt = 0;
		    show_header_line();
		    refreshp();
		} else
		    netkeyer(K_PTT, "0");	// ptt off in any case.

		break;
	    }

	    // Alt-z (M-z), show zones worked.
	    case ALT_Z: {
		if (CONTEST_IS(CQWW)) {
		    if (zonedisplay == 0)
			zonedisplay = 1;
		    else {
			zonedisplay = 0;
			clear_display();
		    }
		} else {
		    multiplierinfo();
		}

		break;
	    }

	    // Alt-x (M-x), Exit
	    case ALT_X: {
		mvaddstr(13, 29, "Do you want to leave Tlf? (y/n): ");
		while (x != 'N') {

		    x = toupper(key_get());

		    if (x == 'Y') {
			exit(EXIT_SUCCESS);
		    }
		}
		x = ESCAPE;
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

		} else {
		    /* restore content */
		    restore_hiscall();
		    restore_comment();
		}

		clear_display();
		break;

	    case CTRL_W:
		/* wipe out or restore call input field */
		if (current_qso.call[0] != '\0') {
		    cleanup_hiscall();
		} else {
		    restore_hiscall();
		}

		break;

	    // <Escape>, clear call input or stop sending.
	    case ESCAPE: {
		if (!stop_tx_only) {
		    if (!early_started) {
			/* if CW not started early drop call and start anew */
			cleanup();
			clear_display();
		    }
		    freqstore = 0;
		}

		break;
	    }

	    // Exclamation, open a new shell.
	    case '!': {
		const char *shell = getenv("SHELL");
		if (shell == NULL) {
		    shell = "sh";
		}
		endwin();
		IGNORE(system("clear"));;
		IGNORE(system(shell));;
		IGNORE(system("clear"));;
		set_term(mainscreen);
		clear_display();

		break;
	    }

	    // Ctrl-L (^L), resets screen.
	    case CTRL_L: {
		endwin();
		set_term(mainscreen);
		clear_display();

		break;
	    }

	    // Ctrl-P (^P), show MUF display.
	    case CTRL_P: {
		int currentterm = miniterm;
		miniterm = 0;
		muf();
		miniterm = currentterm;
		clear_display();

		break;
	    }

	    // Ctrl-A (^A), add a spot and share on LAN.
	    case CTRL_A: {
		add_local_spot();      // note: clears call input field
		grab.state = REACHED;
		grab.spotfreq = freq;
		break;
	    }

	    // Ctrl-B (^B), send spot to DX cluster.
	    case CTRL_B: {
		if (!nopacket && packetinterface > 0) {
		    add_cluster_spot();
		}

		break;
	    }

	    // Ctrl-F (^F), change frequency dialog.
	    case CTRL_F: {
		change_freq();

		break;
	    }

	    // Ctrl-G (^G), grab next DX spot from bandmap.
	    case CTRL_G: {
		freq_t f = grab_next();
		if (f > 0.0) {
		    grab.state = IN_PROGRESS;
		    grab.spotfreq = f;
		    show_header_line();
		    freqstore = 0;
		}

		break;
	    }

	    // Alt-g (M-g), grab first spot matching call field chars.
	    case ALT_G: {
		double f = grabspot();
		if (f > 0.0) {
		    grab.state = IN_PROGRESS;
		    grab.spotfreq = f;
		    show_header_line();
		    freqstore = 0;
		}

		break;
	    }

	    // Double quote, send talk message to other nodes.
	    case '\"': {
		if (lan_active)
		    talk();

		break;
	    }

	    // Ctrl-R (^R), toggle trx1, trx2 via lp0 pin 14.
	    case CTRL_R: {
		if (k_pin14 == 0) {
		    k_pin14 = 1;
		    netkeyer(K_SET14, "1");
		} else {
		    k_pin14 = 0;
		    netkeyer(K_SET14, "0");
		}
		break;
	    }

	    // Ctrl-T (^T) or Alt-i (M-i), show talk messages.
	    case CTRL_T:
	    case ALT_I: {
		if (lan_active) {

		    for (t = 0; t <= 5; t++)
			mvaddstr(14 + t, 1, spaces(60));

		    for (t = 0; t <= 4; t++)
			mvaddstr(15 + t, 1, talkarray[t]);
		    nicebox(14, 0, 5, 59, "Messages");

		    refreshp();
		    key_get();
		    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		    for (t = 0; t <= 6; t++)
			mvaddstr(14 + t, 0, spaces(61));

		    clear_display();
		}
		break;
	    }

	}	/* end switch */


	/* Add character to call input field. */
	if (valid_call_char(x)) {
	    x = g_ascii_toupper(x);

	    if (strlen(current_qso.call) < MAX_CALL_LENGTH) {
		instring[0] = x;
		instring[1] = '\0';
		addch(x);
		strcat(current_qso.call, instring);
		if (cqmode == CQ && cwstart > 0 &&
			trxmode == CWMODE && iscontest) {
		    /* early start keying after 'cwstart' characters but only
		     * if input field contains at least one nondigit */
		    if (strlen(current_qso.call) == cwstart && !plain_number(current_qso.call)) {
			x = autosend();
		    }
		}
	    }

	    freqstore = freq;

	}

	if (cqmode == CQ && cwstart < 0 && trxmode == CWMODE && iscontest) {
	    if (x == '\n' || x == KEY_ENTER) {
		/* early start keying after 'Enter' but only if input field
		 * contains at least two chars, one or more of it nondigit */
		if (strlen(current_qso.call) >= 2 && !plain_number(current_qso.call)) {
		    x = autosend();
		}
	    }
	}

	/* end call input */
	/* keep this list of keys in sync with the list in calledit() */
	if (x == '\n' || x == KEY_ENTER || x == SPACE || x == TAB
		|| x == CTRL_K || x == ',' || x == BACKSLASH) {
	    break;
	}

    }

    return x;
}

// accepts A-Z 0-9 /
bool valid_call_char(int ch) {
    return (ch >= 'A' && ch <= 'Z')
	   || (ch >= 'a' && ch <= 'z')
	   || (ch >= '0' && ch <= '9')
	   || ch == '/';
}

/** autosend function
 *
 * autosend allow an operator in RUN mode to just enter the call of the
 * other station. TLF will start sending the call and switch automatically
 * to sending the exchange when typing stops.
 *  - starts after 2..5 characters
 *  - shorter calls have to be finished with ENTER key
 *  - as soon as autosend starts only alfanumerical keys are accepted
 *  - no edit after input possible
 *  - calculates expected time to send call from cw speed and
 *  - switches to sending exchange after that time is reached
 *
 *  \return last typed key, ESC or \n
 *          ESC - transmission has stopped
 *          \n  - timeout or CR pressed -> send exchange
 */
int autosend() {

    GTimer *timer;
    double timeout, timeout_sent;
    int x;
    int char_sent;

    early_started = true;
    sending_call = true;
    sendmessage(current_qso.call);
    sending_call = false;
    strcpy(hiscall_sent, current_qso.call);

    char_sent = 0; 			/* no char sent so far */
    timeout_sent = (1.2 / speed) * getCWdots(current_qso.call[char_sent]);

    timer = g_timer_new();
    timeout = (1.2 / speed) * cw_message_length(current_qso.call);

    x = -1;
    while ((x != ESCAPE) && (x != '\n' && x != KEY_ENTER)) {
	x = -1;
	while ((x == -1) && (g_timer_elapsed(timer, NULL) < timeout)) {

	    highlightCall(char_sent + 1);

	    usleep(10000);

	    if (g_timer_elapsed(timer, NULL) > timeout_sent) {
		/* one char sent - display and set new timeout */
		char_sent ++;
		timeout_sent +=
		    (1.2 / speed) * getCWdots(current_qso.call[char_sent]);

	    }

	    /* make sure that the wrefresh() inside getch() shows the cursor
	     * in the input field */
	    wmove(stdscr, 12, 29 + strlen(current_qso.call));
	    x = key_poll();

	}

	if (x == -1) { 		/* timeout */
	    x = '\n';
	    continue;
	}

	// <Escape>
	if (x == ESCAPE) {
	    stoptx();
	    *hiscall_sent = '\0';
	    early_started = false;
	    continue;
	}


	int len = strlen(current_qso.call);
	if (len < 13 && valid_call_char(x)) {
	    char append[2];

	    /* convert to upper case */
	    x = g_ascii_toupper(x);

	    /* insert into current_qso.call */
	    current_qso.call[len] = x;
	    current_qso.call[len + 1] = '\0';

	    /* display it  */
	    printcall();

	    /* send it to cw */
	    append[0] = x;
	    append[1] = '\0';
	    sendmessage(append);

	    /* add char length to timeout */
	    timeout += (1.2 / speed) * getCWdots((char) x);

	    len = strlen(hiscall_sent);
	    hiscall_sent[len] = x;
	    hiscall_sent[len + 1] = '\0';
	}
    }

    g_timer_destroy(timer);
    return x;
}


void send_bandswitch(freq_t freq) {

    char outnibble[3];
    int bandswitch = 0;

    if (!use_bandoutput) {
	return;
    }

    const int khz = (int)(freq / 1000.0);
    if (khz > 15) {	// looks like a freq...
	switch (khz) {
	    case 1800 ... 2000:
		bandswitch = 1;
		break;
	    case 3500 ... 4000:
		bandswitch = 2;
		break;
	    case 7000 ... 7300:
		bandswitch = 3;
		break;
	    case 10100 ... 10150:
		bandswitch = 4;
		break;
	    case 14000 ... 14350:
		bandswitch = 5;
		break;
	    case 18068 ... 18168:
		bandswitch = 6;
		break;
	    case 21000 ... 21450:
		bandswitch = 7;
		break;
	    case 24890 ... 24990:
		bandswitch = 8;
		break;
	    case 28000 ... 29700:
		bandswitch = 9;
	}
    } else			// use the bandinx
	bandswitch = bandinx + 1;

    bandswitch = bandindexarray[bandswitch];

    sprintf(outnibble, "%d", bandswitch);
    netkeyer(K_SWITCH, outnibble);
}


/** handle bandswitch from keyboard
 *
 **/
void handle_bandswitch(int direction) {
    // make sure call field is empty and arrows are enabled
    if (*current_qso.call != '\0' || no_arrows) {
	return;
    }

    next_band(direction);

    if (iscontest && !CONTEST_IS(DXPED)) {
	while (IsWarcIndex(bandinx)) {	/* loop till next contest band */
	    next_band(direction);
	}
    }

    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
    mvaddstr(12, 0, band[bandinx]);

    if (trx_control) {
	set_outfreq(bandfrequency[bandinx]);
    }

    send_bandswitch(bandfrequency[bandinx]);
}

/** handle TRX memory operation and update screen
 *
 **/
void handle_memory_operation(memory_op_t op) {
    const freq_t newfreq = memory_get_freq();

    switch (op) {
	case STORE_OR_POP:
	    memory_store_or_pop();
	    break;
	case POP:
	    memory_pop();
	    break;
	case SWAP:
	    memory_swap();
	    break;
    }

    show_header_line();

    if (newfreq <= 0) {
	return;     // freq not changed
    }

    // wait until change is reported back from the rig (timeout: 1 sec)
    for (int i = 0; i < 20; i++) {
	if (fabs(freq - newfreq) < 100) {
	    break;
	}
	usleep(50000);
    }
}
