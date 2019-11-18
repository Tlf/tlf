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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ------------------------------------------------------------
 *        Callinput handles the call input field
 *
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "addspot.h"
#include "autocq.h"
#include "bandmap.h"
#include "calledit.h"
#include "callinput.h"
#include "changefreq.h"
#include "changepars.h"
#include "clear_display.h"
#include "cleanup.h"
#include "cw_utils.h"
#include "edit_last.h"
#include "err_utils.h"
#include "deleteqso.h"
#include "getctydata.h"
#include "gettxinfo.h"
#include "grabspot.h"
#include "ignore_unused.h"
#include "lancode.h"
#include "muf.h"
#include "netkeyer.h"
#include "nicebox.h"		// Includes curses.h
#include "note.h"
#include "prevqso.h"
#include "printcall.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "qtcwin.h"
#include "rtty.h"
#include "searchlog.h"		// Includes glib.h
#include "sendbuf.h"
#include "sendspcall.h"
#include "show_help.h"
#include "showinfo.h"
#include "showpxmap.h"
#include "speedupndown.h"
#include "splitscreen.h"
#include "stoptx.h"
#include "time_update.h"
#include "ui_utils.h"
#include "writeparas.h"
#include "showzones.h"
#include "bands.h"
#include "fldigixmlrpc.h"

#include <math.h>

#define TUNE_UP 6	/* tune up for 6 s (no more than 10) */

void send_bandswitch(freq_t freq);
int autosend(void);
int plain_number(char *str);
void handle_bandswitch(int direction);

extern int no_arrows;
extern char hiscall[];
extern int bandinx;
extern char band[NBANDS][4];
extern int contest;
extern int dxped;
extern freq_t freq;
extern int trx_control;
extern freq_t bandfrequency[];


/** callsign input loop
 *
 * \return code of last typed character */
int callinput(void) {

    extern int itumult;
    extern int wazmult;
    extern int isdupe;		// LZ3NY auto-b4 patch
    extern int cwstart;
    extern int early_started;
    extern char hiscall_sent[];
    extern char comment[];
    extern cqmode_t cqmode;
    extern int trxmode;
    extern char lastcall[];
    extern int cqdelay;
    extern char his_rst[];
    extern char backgrnd_str[];
    extern int cluster;
    extern int announcefilter;
    extern char ph_message[14][80];
    extern freq_t mem;
    extern SCREEN *mainscreen;
    extern int simulator;
    extern int simulator_mode;
    extern char talkarray[5][62];
    extern int lan_active;
    extern int zonedisplay;
    extern int showscore_flag;
    extern int searchflg;
    extern int cqww;
    extern char cqzone[];
    extern char ituzone[];
    extern int ctcomp;
    extern int nob4;
    extern int change_rst;
    extern int weight;
    extern int k_pin14;
    extern int k_ptt;
    extern int noautocq;
    extern int miniterm;
    extern int no_rst;

    extern int bmautoadd;
    extern int bmautograb;
    extern int digikeyer;

    static freq_t freqstore;		/* qrg during last callsign input
					   character, 0 if grabbed,
					   used to decide if a call in input
					   field was entered by hand or
					   grabbed from spot list */

    enum grabstate_t { NONE, IN_PROGRESS, REACHED };

    struct grab_t {
	enum grabstate_t state;
	freq_t spotfreq;
	char call[15];
    };

    static struct grab_t grab =  { .state = NONE };



    int cury, curx;
    int j, t, x = 0;
    char instring[2] = { '\0', '\0' };
    static int lastwindow;


    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    printcall();	/* print call input field */
    searchlog(hiscall);

    while (strlen(hiscall) <= 13) {

	show_zones(bandinx);
	printcall();

	/* wait for next char pressed, but update time, cluster and TRX qrg */
	/* main loop waiting for input */
	x = -1;
	while (x < 1) {

	    usleep(10000);

	    time_update();

	    if (trxmode == DIGIMODE) {
		show_rtty();
	    }

	    if (digikeyer == FLDIGI && fldigi_set_callfield == 1 && hiscall[0] != '\0') {
		freqstore = freq;
		fldigi_set_callfield = 0;
	    }

	    /* if BMAUTOADD is active and user has input a call sign
	     * (indicated by non-zero freqstore) check if he turns away
	     * from frequency and if so add call to spot list */
	    if (bmautoadd != 0 && freqstore != 0) {
		if (strlen(hiscall) >= 3) {
		    if (fabsf(freq - freqstore) > 500) {
			add_to_spots(hiscall, freqstore);
			hiscall[0] = '\0';
			HideSearchPanel();
			freqstore = 0;
		    }
		}
	    }

	    /* if BMAUTOGRAB is active in S&P mode and input field is empty and a spot has
	     * not already been grabbed here check if a spot is on freq
	     * and pick it up if one found */
	    if (bmautograb && cqmode == S_P && *hiscall == '\0' && grab.state == NONE) {
		get_spot_on_qrg(grab.call, freq);
		if (strlen(grab.call) >= 3) {
		    strncpy(hiscall, grab.call, sizeof(hiscall));
		    grab.state = REACHED;
		    grab.spotfreq = freq;

		    showinfo(getctydata_pfx(hiscall));
		    printcall();
		    searchlog(hiscall);
		    freqstore = 0;
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
	    if (fabsf(freq - grab.spotfreq) > 500 && grab.state == REACHED) {
		grab.state = NONE;
		hiscall[0] = '\0';
		printcall();
		HideSearchPanel();
		showinfo(SHOWINFO_DUMMY);
	    }


	    /* make sure that the wrefresh() inside getch() shows the cursor
	     * in the input field */
	    wmove(stdscr, 12, 29 + strlen(hiscall));
	    x = key_poll();

	}

	/* special handling of some keycodes if call field is empty */
	if (*hiscall == '\0') {
	    // <Enter>, sends CQ message (F1), starts autoCQ, or sends S&P message.
	    if ((x == '\n' || x == KEY_ENTER) && *hiscall == '\0') {
		if (cqmode == CQ) {
		    if (noautocq != 1)
			x = auto_cq();
		} else {
		    sendspcall();
		    break;
		}
	    }

	    // Up Arrow or Alt-e, edit last QSO
	    if (x == KEY_UP || x == 229) {
		edit_last();
		break;
	    }

	    // Equals, confirms last callsign already logged if call field is empty.
	    if (x == '=' && *hiscall == '\0') {
		char *str = g_strdup_printf("%s TU ", lastcall);
		sendmessage(str);
		g_free(str);
		break;
	    }
	}

	switch (x) {

	    // Plus (+)
	    // - in non-CT mode switch to other mode (CQ <-> S&P)
	    // - in CT mode send exchange, log QSO, no message sent.
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

		    if (strlen(hiscall) > 2) {
			send_standard_message(2);

			if (((cqww == 1) || (wazmult == 1))
				&& (*comment == '\0'))
			    strcpy(comment, cqzone);

			if ((itumult == 1) && (*comment == '\0'))
			    strcpy(comment, ituzone);
			x = '\\';   // key for logging QSO without message

		    }
		}
		break;
	    }

	    // Ctrl-Q (^Q), open QTC window for receiving or sending QTCs.
	    case 17: {
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
	    case 19: {
		if (qtcdirection == 2 || qtcdirection == 3) {	// in case of QTC=SEND ot QTC=BOTH
		    qtc_main_panel(SEND);
		}
		x = KEY_LEFT;
		continue;
	    }

	    // <Home>, enter call edit when call field is not empty.
	    case KEY_HOME: {
		if ((*hiscall != '\0') && (ungetch(x) == OK)) {
		    calledit();
		}

		break;
	    }

	    // Left Arrow, enter call edit when call field is not empty, or band down.
	    case KEY_LEFT: {
		if (*hiscall != '\0') {
		    calledit();
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

	    // Alt-w (M-w), set Morse weight.
	    case 247: {
		char weightbuf[5] = "";
		char *end;

		mvprintw(12, 29, "Wght: -50..50");

		nicebox(1, 1, 2, 12, "Cw");
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		mvprintw(2, 2, "Speed:   %2u ", GetCWSpeed());
		mvprintw(3, 2, "Weight: %3d ", weight);
		refreshp();

		usleep(800000);
		mvprintw(3, 10, "   ");

		echo();
		mvgetnstr(3, 10, weightbuf, 3);
		noecho();

		g_strchomp(weightbuf);

		int tmp = strtol(weightbuf, &end, 10);

		if ((weightbuf[0] != '\0') && (*end == '\0')) {
		    /* successful conversion */

		    if (tmp > -51 && tmp < 51) {
			weight = tmp;
			netkeyer(K_WEIGHT, weightbuf);
		    }
		}
		clear_display();
		printcall();

		break;
	    }

	    // Alt-v (M-v), change Morse speed in CW mode, else band down.
	    case 246: {
		if (ctcomp == 1) {
		    while (x != 27) {	//escape
			nicebox(1, 1, 2, 12, "Cw");
			attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
			mvprintw(2, 2, "Speed:   %2u ", GetCWSpeed());
			mvprintw(3, 2, "Weight: %3d ", weight);
			printcall();
			refreshp();

			x = key_get();
			if (x == KEY_UP) {
			    speedup();
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

			    mvprintw(0, 14, "%2u", GetCWSpeed());

			} else if (x == KEY_DOWN) {
			    speeddown();
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
			    mvprintw(0, 14, "%2u", GetCWSpeed());

			} else
			    x = 27;	// <Escape>

			clear_display();
		    }
		} else {	// trlog compatible, band switch
		    handle_bandswitch(BAND_DOWN);
		}
		x = -1;

		break;
	    }

	    // <Page-Up>, change RST if call field not empty, else increase CW speed.
	    case KEY_PPAGE: {
		if ((change_rst == 1) && (strlen(hiscall) != 0)) {	// change RST

		    if (his_rst[1] <= 56) {

			his_rst[1]++;

			if (!no_rst)
			    mvprintw(12, 44, his_rst);
			mvprintw(12, 29, hiscall);
		    }

		} else {	// change cw speed
		    speedup();

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2u", GetCWSpeed());
		}

		break;
	    }


	    // <Page-Down>, change RST if call field not empty, else decrease CW speed.
	    case KEY_NPAGE: {
		if ((change_rst == 1) && (strlen(hiscall) != 0)) {

		    if (his_rst[1] > 49) {
			his_rst[1]--;

			if (!no_rst)
			    mvprintw(12, 44, his_rst);
			mvprintw(12, 29, hiscall);
		    }

		} else {

		    speeddown();

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2u", GetCWSpeed());
		}
		break;
	    }

	    // <Enter>, log QSO in CT mode, else test if B4 message should be sent.
	    case '\n':
	    case KEY_ENTER: {
		if (strlen(hiscall) > 2 && ctcomp == 1) {
		    /* there seems to be a call
		     * means: log it (in CT mode */
		    x = 92;	// '\' log without sending message
		    break;
		}

		if (strlen(hiscall) < 3 || nob4 == 1)
		    break;

		/* check b4 QSO if call is long enough and 'nob4' off */
		isdupe = 0;	// LZ3NY  auto-b4 patch

		searchlog(hiscall);

		if (isdupe != 0) {
		    // XXX: Before digi_message, SSB mode sent CW here. - W8BSD
		    send_standard_message(6);	/* as with F7 */
		    cleanup();
		    clear_display();
		}
		break;
	    }

	    // <Insert>, send exchange in CT mode
	    case KEY_IC: {
		if (ctcomp != 0) {
		    send_standard_message(1);		// F2

		}
		break;
	    }

	    // Colon, prefix for entering commands or changing parameters.
	    case ':': {
		changepars();
		hiscall[0] = '\0';
		x = 0;
		clear_display();
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

		for (j = 13; j <= 23; j++) {
		    mvprintw(j, 0, backgrnd_str);
		}

		attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

		mvprintw(12, 29, "            ");
		mvprintw(12, 29, "");
		refreshp();
		break;
	    }

	    // Hash, save xcvr freq to mem or restore mem to xcvr.
	    case '#': {
		if (mem == 0.0) {
		    mem = freq;
		    mvprintw(14, 67, " MEM: %7.1f", mem / 1000.);
		} else {
		    freq = mem;

		    set_outfreq(mem);

		    mem = 0.0;
		    mvprintw(14, 67, "             ");
		}
		mvprintw(29, 12, " ");
		mvprintw(29, 12, "");
		refreshp();
		break;
	    }

	    // Minus, delete previous QSO from log.
	    case '-': {
		delete_qso();
		break;
	    }

	    // Semicolon or Alt-n (M-n), insert note in log.
	    case ';':
	    case 238: {
		include_note();
		x = -1;
		break;
	    }

	    // Alt-0 to Alt-9 (M-0...M-9), send CW/Digimode messages 15-24.
	    case 176 ... 185: {
		send_standard_message(x - 162);	/* alt-0 to alt-9 */

		break;
	    }

	    // F1, send CQ or S&P call message.
	    case KEY_F(1): {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {

		    if (cqmode == S_P) {
			sendspcall();
		    } else {
			send_standard_message(0);	/* CQ */
		    }


		    if (simulator != 0) {
			simulator_mode = 1;
		    }
		} else {

		    if (cqmode == S_P)
			play_file(ph_message[5]);	/* S&P */
		    else
			play_file(ph_message[0]);
		}
		break;
	    }

	    // F2-F11, send messages 2 through 11.
	    case KEY_F(2) ... KEY_F(11): {
		send_standard_message(x - KEY_F(1));	// F2...F11 - F1 = 1...10

		break;
	    }

	    // F12, activate autocq timing and message.
	    case KEY_F(12): {
		x = auto_cq();
		break;
	    }

	    // Query, send call with " ?" appended or F5 message in voice mode.
	    case '?': {
		if (*hiscall != '\0') {
		    strcat(hiscall, " ?");
		    send_standard_message(4);
		    hiscall[strlen(hiscall) - 2] = '\0';
		}
		x = -1;
		break;
	    }

	    // <Backspace>, remove chracter left of cursor, move cursor left one position.
	    case KEY_BACKSPACE: {
		if (*hiscall != '\0') {
		    getyx(stdscr, cury, curx);
		    mvprintw(cury, curx - 1, " ");
		    mvprintw(cury, curx - 1, "");
		    hiscall[strlen(hiscall) - 1] = '\0';

		    if (atoi(hiscall) < 1800) {	/*  no frequency */
			showinfo(getctydata_pfx(hiscall));
			searchlog(hiscall);
			refreshp();
		    }

		    x = -1;
		    break;
		}
		break;
	    }

	    // Alt-r (M-r) or Alt-s (M-s), toggle score window.
	    case 242:
	    case 243: {
		if (showscore_flag == 0)
		    showscore_flag = 1;
		else {
		    showscore_flag = 0;
		}
		clear_display();
		break;
	    }

	    // Alt-k (M-k), synonym for Ctrl-K (^K).
	    case 235: {
		x = 11;		// Ctrl-K
		break;
	    }

	    // Alt-a (M-a), cycle cluster window.
	    case 225: {
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
	    case 226: {
		if (ctcomp == 0) {
		    handle_bandswitch(BAND_UP);
		}
		break;
	    }

	    // Alt-j (M-j), show station frequencies.
	    case 234: {
		if (cluster != FREQWINDOW) {
		    lastwindow = cluster;
		    cluster = FREQWINDOW;
		} else
		    cluster = lastwindow;

		break;
	    }

	    // Alt-h (M-h), show help.
	    case 232: {
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
	    case 227: {
		if (searchflg != SEARCHWINDOW)
		    searchflg = SEARCHWINDOW;
		else
		    searchflg = 0;
		break;
	    }

	    // Alt-m (M-m), show multipliers.
	    case 237: {
		show_mults();
		refreshp();
		break;
	    }

	    // Alt-p (M-p), toggle PTT via cwdaemon
	    case 240: {
		if (k_ptt == 0) {
		    k_ptt = 1;
		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 2, "PTT on   ");
		    mvprintw(12, 29, "");
		    refreshp();
		    netkeyer(K_PTT, "1");	// ptt on
		    x = key_get();	// any character to stop tuning
		    if (x == 240)	// Alt-P (M-p)
			netkeyer(K_PTT, "0");	// ptt off
		    k_ptt = 0;
		    show_header_line();
		    refreshp();
		} else
		    netkeyer(K_PTT, "0");	// ptt off in any case.

		break;
	    }

	    // Alt-t (M-t), tune xcvr via cwdaemon.
	    case 244: {
		int count;
		gchar *buff;

		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 2, "Tune     ");
		mvprintw(12, 29, "");
		refreshp();

		buff = g_strdup_printf("%d", TUNE_UP);
		netkeyer(K_TUNE, buff);	// cw on
		g_free(buff);

		count = (int)(TUNE_UP / 0.25);

		while (count != 0) {
		    usleep(250000);
		    if (key_poll() != -1)	// any key pressed ?
			break;
		    count--;
		}

		netkeyer(K_ABORT, "");	// cw abort

		show_header_line();
		refreshp();

		break;
	    }

	    // Alt-z (M-z), show zones worked.
	    case 250: {
		if (cqww == 1) {
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

	    // Alt-q (M-q) or Alt-x (M-x), Exit
	    case 241:
	    case 248: {
		mvprintw(13, 29, "You want to leave tlf? (y/n): ");
		while (x != 'n' && x != 'N') {

		    x = key_get();

		    if (x == 'y' || x == 'Y') {
			writeparas();
			cleanup_telnet();
			endwin();

			puts("\n\nThanks for using TLF.. 73\n");
			exit(0);
		    }
		}
		x = 27;		// <Escape>
		break;
	    }

	    // <Escape>, clear call input or stop sending.
	    case 27: {
		if (early_started == 0) {
		    /* if CW not started early drop call and start anew */
		    cleanup();
		    clear_display();
		} else {
		    /* otherwise just stop sending */
		    stoptx();
		    *hiscall_sent = '\0';
		    early_started = 0;
		}

		freqstore = 0;
		break;
	    }

	    // Underscore, confirm last exchange.
	    case '_': {
		prev_qso();

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
	    case 12: {
		endwin();
		set_term(mainscreen);
		clear_display();

		break;
	    }

	    // Ctrl-P (^P), show MUF display.
	    case 16: {
		int currentterm = miniterm;
		miniterm = 0;
		muf();
		miniterm = currentterm;
		clear_display();

		break;
	    }

	    // Ctrl-A (^A), add a spot and share on LAN.
	    case 1: {
		addspot();
		HideSearchPanel();
		showinfo(SHOWINFO_DUMMY);

		grab.state = REACHED;
		grab.spotfreq = freq;
		break;
	    }

	    // Ctrl-B (^B), send spot to DX cluster.
	    case 2: {
		announcefilter = 0;
		cluster = CLUSTER;
		send_cluster();

		break;
	    }

	    // Ctrl-F (^F), change frequency dialog.
	    case 6: {
		change_freq();

		break;
	    }

	    // Ctrl-G (^G), grab next DX spot from bandmap.
	    case 7: {
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
	    case 231: {
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
		if (lan_active != 0)
		    talk();

		break;
	    }

	    // Ctrl-R (^R), toogle trx1, trx2 via lp0 pin 14.
	    case 18: {
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
	    case 20:
	    case 233: {
		if (lan_active != 0) {

		    for (t = 0; t <= 5; t++)
			mvprintw(14 + t, 1,
				 "                                                            ");
		    for (t = 0; t <= 4; t++)
			mvprintw(15 + t, 1, talkarray[t]);
		    nicebox(14, 0, 5, 59, "Messages");

		    refreshp();
		    key_get();
		    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		    for (t = 0; t <= 6; t++)
			mvprintw(14 + t, 0,
				 "                                                             ");

		    clear_display();
		}
		break;
	    }

	}	/* end switch */


	// Ctrl-<Page-Up>, increase cqdelay by 1/2 second.
	// Alt-<Page-Up>, same for terminals that consume Ctrl-<Page-Up>.
	if ((key_kPRV3 && x == key_kPRV3)
		|| (key_kPRV5 && x == key_kPRV5)) {

	    if (cqdelay <= 60) {
		cqdelay++;

		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 19, "  ");
		mvprintw(0, 19, "%i", cqdelay);
	    }

	    break;
	}


	// Ctrl-<Page-Down>, decrease cqdelay by 1/2 Second.
	// Alt-<Page-Down>, same for terminals that consume Ctrl-<Page-Down>.
	if ((key_kNXT3 && x == key_kNXT3)
		|| (key_kNXT5 && x == key_kNXT5)) {

	    if (cqdelay >= 4) {
		cqdelay--;

		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 19, "  ");
		mvprintw(0, 19, "%i", cqdelay);
	    }

	    break;
	}


	/* Convert to upper case */
	if (x >= 'a' && x <= 'z')
	    x = x - 32;

	/* Add character to call input field. */
	if (x >= '/' && x <= 'Z') {

	    if (strlen(hiscall) < 13) {
		instring[0] = x;
		instring[1] = '\0';
		addch(x);
		strcat(hiscall, instring);
		if (cqmode == CQ && cwstart > 0 &&
			trxmode == CWMODE && contest == 1) {
		    /* early start keying after 'cwstart' characters but only
		     * if input field contains at least one nondigit */
		    if (strlen(hiscall) == cwstart && !plain_number(hiscall)) {
			x = autosend();
		    }
		}
	    }

	    if (atoi(hiscall) < 1800) {	/*  no frequency */

		showinfo(getctydata_pfx(hiscall));
		searchlog(hiscall);
	    }

	    refreshp();

	    freqstore = freq;

	}

	if (cqmode == CQ && cwstart < 0 && trxmode == CWMODE &&
		contest == 1) {
	    if (x == '\n' || x == KEY_ENTER) {
		/* early start keying after 'Enter' but only if input field
		 * contains at least two chars, one or more of it nondigit */
		if (strlen(hiscall) >= 2 && !plain_number(hiscall)) {
		    x = autosend();
		}
	    }
	}

	if ((x == '\n' || x == KEY_ENTER) || x == 32 || x == 9 || x == 11
		|| x == 44 || x == 92) {
	    break;
	}

    }

    return (x);
}

/** check if string is plain number
 *
 * Check if string contains only digits
 * \param str    the string to check
 * \return true  if only digits inside
 *         false at least one none digit
 */
int plain_number(char *str) {
    int i;

    for (i = 0; i < strlen(str); i++) {
	if (!isdigit(str[i])) {
	    return false;
	}
    }

    return true;
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

    extern int early_started;
    extern int sending_call;
    extern char hiscall_sent[];
    extern char hiscall[];

    GTimer *timer;
    double timeout, timeout_sent;
    int x;
    int char_sent;

    early_started = 1;
    sending_call = 1;
    sendmessage(hiscall);
    sending_call = 0;
    strcpy(hiscall_sent, hiscall);

    char_sent = 0; 			/* no char sent so far */
    timeout_sent = (1.2 / GetCWSpeed()) * getCWdots(hiscall[char_sent]);

    timer = g_timer_new();
    timeout = (1.2 / GetCWSpeed()) * cw_message_length(hiscall);

    x = -1;
    while ((x != 27) && (x != '\n' && x != KEY_ENTER)) {
	x = -1;
	while ((x == -1) && (g_timer_elapsed(timer, NULL) < timeout)) {

	    highlightCall(char_sent + 1);

	    usleep(10000);

	    if (g_timer_elapsed(timer, NULL) > timeout_sent) {
		/* one char sent - display and set new timeout */
		char_sent ++;
		timeout_sent +=
		    (1.2 / GetCWSpeed()) * getCWdots(hiscall[char_sent]);

	    }

	    /* make sure that the wrefresh() inside getch() shows the cursor
	     * in the input field */
	    wmove(stdscr, 12, 29 + strlen(hiscall));
	    x = key_poll();

	}

	if (x == -1) { 		/* timeout */
	    x = '\n';
	    continue;
	}

	// <Escape>
	if (x == 27) {
	    stoptx();
	    *hiscall_sent = '\0';
	    early_started = 0;
	    continue;
	}

	/* convert to upper case */
	if (x >= 'a' && x <= 'z')
	    x = x - 32;

	int len = strlen(hiscall);
	if (len < 13 && x >= '/' && x <= 'Z') {
	    char append[2];

	    /* insert into hiscall */
	    hiscall[len] = x;
	    hiscall[len + 1] = '\0';

	    /* display it  */
	    printcall();

	    /* send it to cw */
	    append[0] = x;
	    append[1] = '\0';
	    sendmessage(append);

	    /* add char length to timeout */
	    timeout += (1.2 / GetCWSpeed()) * getCWdots((char) x);

	    len = strlen(hiscall_sent);
	    hiscall_sent[len] = x;
	    hiscall_sent[len + 1] = '\0';
	}
    }

    g_timer_destroy(timer);
    return x;
}


int play_file(char *audiofile) {

    extern int txdelay;
    extern unsigned char rigptt;

    int fd;
    char playcommand[120];

    if (*audiofile == '\0')
	return (0);

    if ((fd = open(audiofile, O_RDONLY, 0664)) < 0) {
	TLF_LOG_INFO("cannot open sound file %s!", audiofile);
    } else {
	close(fd);
	if (access("./play_vk", X_OK) == 0) {
	    sprintf(playcommand, "./play_vk %s", audiofile);
	} else {
	    sprintf(playcommand, "play_vk %s", audiofile);
	}
	/* CAT PTT wanted and available, use it. */
	if (rigptt == 0x03) {
	    /* Request PTT On */
	    rigptt |= (1 << 3);		/* 0x0b */
	} else {		/* Fall back to netkeyer interface */
	    netkeyer(K_PTT, "1");	// ptt on
	}

	usleep(txdelay * 1000);
	IGNORE(system(playcommand));;
	printcall();

	/* CAT PTT wanted, available, and active. */
	if (rigptt == 0x07) {

	    /* Request PTT Off */
	    rigptt |= (1 << 4);		/* 0x17 */
	} else {		/* Fall back to netkeyer interface */
	    netkeyer(K_PTT, "0");	// ptt off
	}
    }

    return 0;
}


void send_bandswitch(freq_t freq) {

    extern int use_bandoutput;
    extern int bandinx;
    extern int bandindexarray[];

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
    if (*hiscall != '\0' || no_arrows) {
	return;
    }

    next_band(direction);

    if (contest == 1 && dxped == 0) {
	while (IsWarcIndex(bandinx)) {	/* loop till next contest band */
	    next_band(direction);
	}
    }

    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
    mvprintw(12, 0, band[bandinx]);

    if (trx_control) {
	freq = bandfrequency[bandinx]; // TODO: is this needed?
	set_outfreq(bandfrequency[bandinx]);
    }

    send_bandswitch(bandinx);
}
