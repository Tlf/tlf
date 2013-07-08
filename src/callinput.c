/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2005 Rein Couperus <pa0r@eudxf.org>
 *               2009-2013 Thomas Beierlein <tb@forth-ev.de>
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

#include "callinput.h"
#include "addspot.h"
#include "changefreq.h"
#include "bandmap.h"
#include "glib.h"

#define TUNE_UP 6	/* tune up for 6 s (no more than 10) */

void send_bandswitch(int freq);

/** callsign input loop
 *
 * \return code of last typed character */
char callinput(void)
{
    extern int itumult;
    extern int wazmult;
    extern int use_rxvt;
    extern int no_arrows;
    extern int isdupe;		// LZ3NY auto-b4 patch
    extern int contest;
    extern int dxped;
    extern int cwstart;
    extern int early_started;
    extern int sending_call;
    extern char hiscall[];
    extern char hiscall_sent[];
    extern char comment[];
    extern char call[];
    extern int cqmode;
    extern int trxmode;
    extern char mode[];
    extern char lastcall[];
    extern char band[9][4];
    extern int bandinx;
    extern char speedstr[];
    extern int keyspeed;
    extern int cqdelay;
    extern char his_rst[];
    extern char backgrnd_str[];
    extern int demode;
    extern int cluster;
    extern int announcefilter;
    extern char buffer[];
    extern char message[15][80];
    extern char ph_message[14][80];
    extern float freq;
    extern float mem;
#ifdef HAVE_LIBHAMLIB
    extern freq_t outfreq;
#else
    extern int outfreq;
    extern int rignumber;
#endif
    extern int trx_control;
    extern float bandfrequency[];
    extern SCREEN *mainscreen;
    extern int simulator;
    extern int simulator_mode;
    extern char talkarray[5][62];
    extern int lan_active;
    extern int cluster;
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
    extern int keyerport;
    extern int miniterm;
    extern int no_rst;

    int cury, curx;
    int i, j, ii, rc, t, x = 0, y = 0;
    char instring[2] = { '\0', '\0' };
    char dupecall[17];
    char speedbuf[3] = "";
    char weightbuf[5];
    static int lastwindow;


    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    printcall();	/* print call input field */
    searchlog(hiscall);

    for (i = strlen(hiscall); i <= 13; i++) {

	printcall();

	/* wait for next char pressed, but update time, cluster and TRX qrg */
	nodelay(stdscr, TRUE);	/* main loop waiting for input */
	x = -1;
	while (x < 1) {

	    usleep(10000);

	    time_update();

	    if (trxmode == DIGIMODE && (keyerport == GMFSK
		    || keyerport == MFJ1278_KEYER)) {
		show_rtty();
		printcall();
	    }

	    /* make sure that the wrefresh() inside getch() shows the cursor
	     * in the input field */
	    wmove(stdscr, 12, 29 + strlen(hiscall));
	    x = onechar();

	}
	nodelay(stdscr, FALSE);


	/* special handling of some keycodes if call field is empty */
	if (i == 0 || *hiscall == '\0') {
	    if ((x == '+') && (*hiscall == '\0') && (ctcomp == 0)) {
		/* switch to other mode */
		if (cqmode == CQ) {
		    cqmode = S_P;
		} else
		    cqmode = CQ;

		/* and show new mode */
		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

		if (cqmode == CQ) {
		    mvprintw(0, 2, "Log     ");
		    strcpy(mode, "Log     ");
		} else {
		    mvprintw(0, 2, "S&P     ");
		    strcpy(mode, "S&P     ");
		}
		cleanup();

	    }

	    if (x == '\n' && *hiscall == '\0') {
		if (cqmode == CQ) {
		    if (noautocq != 1)
			x = auto_cq();
		} else {
		    sendspcall();
		    break;
		}
	    }

	    if (x == 152 || x == 229) {	// up-arrow or alt-e
		edit_last();
		break;
	    }

	    if (x == '=' && *hiscall == '\0') {
		strcat(buffer, lastcall);
		strcat(buffer, " OK ");
		sendbuf();
		break;
	    } else if (x == '=' && strlen(hiscall) != 0) {
		/** \todo check if unreachable code */
		strcat(buffer, lastcall);
		strcat(buffer, " OK ");
		sendbuf();
		break;
	    }
	}

	switch (x) {
	case '+':
	    {
		if ((ctcomp != 0) && (strlen(hiscall) > 2)) {
		    if (trxmode == CWMODE || trxmode == DIGIMODE) {
			strcat(buffer, message[2]);	/* F3 */
			sendbuf();

		    } else
			play_file(ph_message[2]);

		    if (((cqww == 1) || (wazmult == 1))
			&& (*comment == '\0'))
			strcpy(comment, cqzone);

		    if ((itumult == 1) && (*comment == '\0'))
			strcpy(comment, ituzone);
		    x = 92;

		}
		break;
	    }

	case 153:		// down - start sending call if cw mode
	case 32:		// space
	    {
		if (trxmode == CWMODE && contest == 1) {
		    strcpy(buffer, hiscall);
		    early_started = 1;
		    sending_call = 1;
		    sendbuf();
		    sending_call = 0;
		    strcpy(hiscall_sent, hiscall);
		    printcall();
		    x = 153;
		} 
		break;
	    }
	case 155:		/* left */
	    {
		if (*hiscall != '\0') {
		    calledit();
		}

		if (bandinx >= 0 && *hiscall == '\0' && no_arrows == 0) {

		    bandinx--;

		    if (bandinx == -1)
			bandinx = 8;

		    if ((contest == 1) && (dxped == 0)
			&& ((bandinx == 3) || (bandinx == 5)
			    || (bandinx == 7)))
			bandinx--;

		    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
		    mvprintw(12, 0, band[bandinx]);
		    i--;
#ifdef HAVE_LIBHAMLIB
		    if (trx_control == 1) {

			outfreq = (int) (bandfrequency[bandinx] * 1000);
		    }
#else
		    if (trx_control == 1 && rignumber >= 2000) {

			outfreq = (int) (bandfrequency[bandinx] * 1000);
		    }
#endif
		    send_bandswitch(bandinx);

		}

		break;
	    }

	case 154:		/* right */
	    {
		if (bandinx <= 8 && *hiscall == '\0' && no_arrows == 0) {

		    bandinx++;

		    if (bandinx > 8)
			bandinx = 0;

		    if ((contest == 1) && (dxped == 0)
			&& ((bandinx == 3) || (bandinx == 5)
			    || (bandinx == 7)))
			bandinx++;

		    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
		    mvprintw(12, 0, band[bandinx]);

#ifdef HAVE_LIBHAMLIB

		    if (trx_control == 1) {
			freq = bandfrequency[bandinx];

			outfreq = (int) (bandfrequency[bandinx] * 1000);
		    }
#else
		    if (trx_control == 1 && rignumber >= 2000) {
			freq = bandfrequency[bandinx];

			outfreq = (int) (bandfrequency[bandinx] * 1000);
		    }
#endif
		    send_bandswitch(bandinx);

		}
		break;
	    }
	case 247:		// Alt-w set weight
	    {
		strncpy(speedbuf, speedstr + (2 * keyspeed), 2);
		speedbuf[2] = '\0';
		nicebox(1, 1, 2, 11, "Cw");
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		mvprintw(2, 2, "Speed: %s  ", speedbuf);
		if (weight < 0)
		    mvprintw(3, 2, "Weight:%d ", weight);
		else
		    mvprintw(3, 2, "weight: %d  ", weight);
		mvprintw(3, 10, "");
		refreshp();
		x = onechar();

		if (x == '-') {
		    mvprintw(3, 9, "%c", '-');
		    refreshp();
		    weightbuf[0] = x;
		    x = onechar();
		    if (x != 27) {
			mvprintw(3, 10, "%c", (char) x);
			refreshp();
			weightbuf[1] = x;
			weightbuf[2] = '\0';
			x = onechar();
		    }
		    if (x != 27) {
			mvprintw(3, 11, "%c", (char) x);
			refreshp();
			weightbuf[2] = x;
			weightbuf[3] = '\0';
		    }
		} else {
		    weightbuf[0] = x;
		    weightbuf[1] = '\0';
		    mvprintw(3, 10, "%c", (char) x);
		    refreshp();
		    x = onechar();
		    if (x != 27) {
			weightbuf[1] = x;
			weightbuf[2] = '\0';
		    }
		}
		x = -1;
		weight = atoi(weightbuf);
		if (weight > -51 && weight < 50) {
		    netkeyer(K_WEIGHT, weightbuf);
		}
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		mvprintw(1, 1, "             ");
		mvprintw(2, 1, "             ");
		mvprintw(3, 1, "             ");
		mvprintw(4, 1, "             ");
		printcall();

		break;
	    }
	case 246:		// Alt-v
	    {
		if (ctcomp == 1) {
		    while (x != 27)	//escape
		    {
			strncpy(speedbuf, speedstr + (2 * keyspeed), 2);
			speedbuf[2] = '\0';
			nicebox(1, 1, 2, 9, "Cw");
			attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
			mvprintw(2, 2, "Speed: %s", speedbuf);
			mvprintw(3, 2, "Weight: %d", weight);
			printcall();
			refreshp();

			x = onechar();
			if (x == 152) {
			    keyspeed = speedup();
			    strncpy(speedbuf, speedstr + (2 * keyspeed),
				    2);
			    speedbuf[2] = '\0';
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

			    mvprintw(0, 14, "%s", speedbuf);

			} else if (x == 153) {
			    keyspeed = speeddown();
			    strncpy(speedbuf, speedstr + (2 * keyspeed),
				    2);
			    speedbuf[2] = '\0';
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
			    mvprintw(0, 14, "%s", speedbuf);

			} else
			    x = 27;

			attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
			mvprintw(1, 1, "           ");
			mvprintw(2, 1, "           ");
			mvprintw(3, 1, "           ");
			mvprintw(4, 1, "           ");
		    }
		} else {	// trlog compatible, band switch
		    if (bandinx >= 0 && *hiscall == '\0') {

			bandinx--;

			if (bandinx == -1)
			    bandinx = 8;

			if ((contest == 1) && (dxped == 0)
			    && ((bandinx == 3) || (bandinx == 5)
				|| (bandinx == 7)))
			    bandinx--;

			attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
			mvprintw(12, 0, band[bandinx]);
			printcall();
			i--;
#ifdef HAVE_LIBHAMLIB
			if (trx_control == 1) {

			    outfreq =
				(int) (bandfrequency[bandinx] * 1000);
			}
#endif
			send_bandswitch(bandinx);

		    }

		}
		x = -1;

		break;
	    }

	case 156:		/* pgup */
	    {
		if ((change_rst == 1) && (strlen(hiscall) != 0)) {	// change RST

		    if (his_rst[1] <= 56) {

			his_rst[1]++;
			
			no_rst ? : mvprintw(12, 44, his_rst);
			mvprintw(12, 29, hiscall);
		    }

		} else {	// change cw speed
		    keyspeed = speedup();

		    strncpy(speedbuf, speedstr + (2 * keyspeed), 2);
		    speedbuf[2] = '\0';

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%s", speedbuf);
		}

		break;
	    }
	case 412:		/* ctrl-pgup, cqdelay (not for TERM=linux */
	    {
		if (cqdelay <= 60) {
		    cqdelay++;

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 19, "  ");
		    mvprintw(0, 19, "%i", cqdelay);
		}

		break;
	    }

	case 157:		/* pgdown */
	    {
		if ((change_rst == 1) && (strlen(hiscall) != 0)) {

		    if (his_rst[1] > 49) {
			his_rst[1]--;

			no_rst ? : mvprintw(12, 44, his_rst);
			mvprintw(12, 29, hiscall);
		    }

		} else {

		    keyspeed = speeddown();

		    strncpy(speedbuf, speedstr + (2 * keyspeed), 2);
		    speedbuf[2] = '\0';
		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%s", speedbuf);
		}
		break;
	    }
	case 413:{		// ctrl-pgdown, cqdelay (not for TERM=linux)
		if (cqdelay >= 4) {
		    cqdelay--;

		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 19, "  ");
		    mvprintw(0, 19, "%i", cqdelay);
		}

		break;
	    }
	case '\n':{
		if (strlen(hiscall) > 2 && ctcomp == 1) {
		    /* there seems to be a call
		     * means: log it (in CT mode */
		    x = 92;
		    break;
		}

		if (strlen(hiscall) < 3 || nob4 == 1)
		    break;

		/* check b4 QSO if call is long enough and 'nob4' off */
		isdupe = 0;	// LZ3NY  auto-b4 patch

		searchlog(hiscall);

		if (isdupe != 0) {
		    strcat(buffer, message[6]);	/* as with F7 */
		    sendbuf();
		    cleanup();
		    clear_display();
		}
		break;
	    }
	case 160:		/* insert */
	    {
		if (ctcomp != 0) {
		    if (trxmode == CWMODE || trxmode == DIGIMODE) {
			strcat(buffer, message[1]);
			sendbuf();

		    } else
			play_file(ph_message[1]);

		}
		break;
	    }
	case 58:		/* :   change parameters */
	    {
		changepars();
		hiscall[0] = '\0';
		x = 0;
		clear_display();
		attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

		for (j = 13; j <= 23; j++) {
		    mvprintw(j, 0, backgrnd_str);
		}

		if (use_rxvt == 0)
		    attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
		else
		    attron(COLOR_PAIR(NORMCOLOR));

		mvprintw(12, 29, "            ");
		mvprintw(12, 29, "");
		refreshp();
		break;
	    }
	case 35:		/*  #  memory */
	    {
		if (mem == 0.0) {
		    mem = freq;
		    mvprintw(14, 68, "MEM: %7.1f", mem);
		} else {
		    freq = mem;
#ifdef HAVE_LIBHAMLIB
		    outfreq = (int) (mem * 1000);
#else
		    outfreq = (mem * 1000);
#endif

		    mem = 0.0;
		    mvprintw(14, 68, "            ");
		}
		mvprintw(29, 12, " ");
		mvprintw(29, 12, "");
		refreshp();
		break;
	    }
	case 45:		/* - delete qso */
	    {
		delete_qso();
		break;
	    }

	case 59:		/* ; note */
	case 238:		/* alt-n */
	    {
		include_note();
		x = -1;
		break;
	    }

	case 176 ... 186:
	    {
		strcat(buffer, message[x - 162]);	/* alt-0 to alt-9 */
		sendbuf();

		break;
	    }

	case 129:		/*  F1 */
	    {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {

		    if (cqmode == 0) {
			if (demode == SEND_DE)
			    strcat(buffer, "DE ");
			strcat(buffer, call);		/* S&P */
		    }
		    else {
			strcat(buffer, message[0]);	/* CQ */
		    }

		    sendbuf();

		    if (simulator != 0) {
			simulator_mode = 1;
		    }

		    break;
		} else {
		    if (cqmode == 0)
			play_file(ph_message[5]);	/* S&P */
		    else
			play_file(ph_message[0]);
		}
		break;
	    }
	case 130 ... 138:			/* F2.. F10 */
	    {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    strcat(buffer, message[x - 129]);	/* F2 */
		    sendbuf();

		} else
		    play_file(ph_message[x - 129]);

		break;
	    }
	case 140:
	    {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    strcat(buffer, message[10]);	/* F11 */
		    sendbuf();
		} else
		    play_file(ph_message[10]);

		break;
	    }
	case 141:		/* F12 */
	    {
		x = auto_cq();
	    }
	case 127:		/* backspace */
	    {
		if (*hiscall != '\0') {
		    getyx(stdscr, cury, curx);
		    mvprintw(cury, curx - 1, " ");
		    mvprintw(cury, curx - 1, "");
		    hiscall[strlen(hiscall) - 1] = '\0';

		    if (atoi(hiscall) < 1800) {	/*  no frequency */
			strncpy(dupecall, hiscall, 16);

			x = getctydata(dupecall);

			showinfo(x);

			searchlog(hiscall);
			refreshp();
		    }

		    i--;
		    x = -1;
		    break;
		}
		break;
	    }
	case 242:		// alt-R
	case 243:		// alt-S
	    {
		if (showscore_flag == 0)
		    showscore_flag = 1;
		else {
		    showscore_flag = 0;
		    /** \todo drop display of score */
		    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		    for (ii = 14; ii < 24; ii++)
			mvprintw(ii, 0, backgrnd_str);
		    refreshp();
		}
		break;
	    }
	case 142 ... 150:
	    {
		message_change(x);

		break;
	    }
	case 235:
	    {			//alt-K     == ctrl-K
		x = 11;
		break;
	    }
	case 225:		// Alt-a
	    {
		if (cluster == NOCLUSTER) {
		    cluster = CLUSTER;	// alt-A
		    announcefilter = FILTER_ALL;
		} else if (cluster == CLUSTER) {
		    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

		    for (ii = 14; ii < 24; ii++)
			mvprintw(ii, 0, backgrnd_str);
		    refreshp();

		    cluster = MAP;
		} else if (cluster == MAP) {
		    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

		    for (ii = 14; ii < 24; ii++)
			mvprintw(ii, 0, backgrnd_str);
		    refreshp();

		    cluster = NOCLUSTER;
		}

		break;
	    }
	case 226:		// alt-b  (band-up for trlog)
	    {
		if (ctcomp == 0) {
		    if (bandinx <= 8 && *hiscall == '\0') {
			bandinx++;

			if (bandinx > 8)
			    bandinx = 0;

			if ((contest == 1) && (dxped == 0)
			    && ((bandinx == 3) || (bandinx == 5)
				|| (bandinx == 7)))
			    bandinx++;

			attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
			mvprintw(12, 0, band[bandinx]);

#ifdef HAVE_LIBHAMLIB

			if (trx_control == 1) {
			    freq = bandfrequency[bandinx];
			    outfreq =
				(int) (bandfrequency[bandinx] * 1000);
			}
#endif
			send_bandswitch(bandinx);

		    }

		}
		break;
	    }
	case 234:		// alt-J
	    {
		if (cluster != FREQWINDOW) {
		    lastwindow = cluster;
		    cluster = FREQWINDOW;
		} else
		    cluster = lastwindow;

		break;
	    }
	case 232:		// alt-H
	    {
		endwin();
		rc=system("clear");
		rc=system("less help.txt");
		rc=system("clear");
		set_term(mainscreen);
		clear_display();
		break;
	    }

	case 172:		// alt-,
	case 46:		// . (dot)
	    {
		bm_menu();
		break;
	    }
	case 227:		//Alt-C
	    {
		if (searchflg != SEARCHWINDOW)
		    searchflg = SEARCHWINDOW;
		else
		    searchflg = 0;
		break;
	    }

	case 237:		// alt-M
	    {
		show_mults();
		refreshp();
		break;
	    }
	case 240:		// Alt-p (toggle ptt)
	    {
		if (k_ptt == 0) {
		    k_ptt = 1;
		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 2, "PTT on   ");
		    mvprintw(12, 29, "");
		    refreshp();
		    netkeyer(K_PTT, "1");	// ptt on
		    x = onechar();	// any character to stop tuning
		    if (x == 240)
			netkeyer(K_PTT, "0");	// ptt off
		    k_ptt = 0;
		    mvprintw(0, 2, "%s", mode);
		    refreshp();
		} else
		    netkeyer(K_PTT, "0");	// ptt off in any case.

		break;
	    }
	case 244:		// Alt-t (tune)
	    {
		int count;
		gchar *buff;

		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 2, "Tune     ");
		mvprintw(12, 29, "");
		refreshp();

		buff = g_strdup_printf("%d", TUNE_UP);
		netkeyer(K_TUNE, buff);	// cw on
		g_free(buff);

		nodelay (stdscr, TRUE);

		count = TUNE_UP / 0.25;

		while (count != 0) {
		    usleep( 250000 );
		    if ((onechar()) != -1)	// any key pressed ?
			break;
		    count--;
		}

		nodelay (stdscr, FALSE);

		netkeyer( K_ABORT, "");	// cw abort

		mvprintw(0, 2, "%s", mode);
		refreshp();

		break;
	    }

	case 250:		//Alt-z
	    {
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

	case 241:		/* EXIT */
	case 248:		// Alt-q || Alt-x
	    {
		mvprintw(13, 29, "You want to leave tlf? (y/n): ");
		while (x != 'n' && x != 'N') {

		    x = onechar();

		    if (x == 'y' || x == 'Y') {
			writeparas();
			endwin();

			puts("\n\nThanks for using TLF.. 73\n");
			exit(0);
		    }
		}
		x = 27;
		break;
	    }

	case 27:		// ESC
	    {
		if (early_started == 0) {
		    /* if CW not started early drop call and start anew */
		    cleanup();
		    clear_display();
		}
		else {
		    /* otherwise just stop sending */
		    stoptx();
		    *hiscall_sent = '\0';
		    early_started = 0;
		}

		break;
	    }
	case 95:
	    {
		prev_qso();

		break;
	    }
	case '!':
	    {
		endwin();
		rc=system("clear");
		rc=system("sh");
		rc=system("clear");
		set_term(mainscreen);
		clear_display();

		break;
	    }
	case 12:		// ctrl-L
	    {
		endwin();
		set_term(mainscreen);
		clear_display();

		break;
	    }
	case 16:		// ctrl-P
	    {
		int currentterm = miniterm;
		miniterm = 0;
		muf();
		miniterm = currentterm;
		clear_display();

		break;
	    }
	case 1:		// ctl-A
	    {
		addspot();
		HideSearchPanel();

		break;
	    }
	case 2:		// ctl-b
	    {
		announcefilter = 0;
		cluster = CLUSTER;
		send_cluster();

		break;
	    }

	case 6:		// ctl-f
	    {
		change_freq();

		break;
	    }
	case 7:		// ctl-g
	    {
		grab_next();

		break;
	    }
	case 231:		// alt-g
	    {
		grabspot();

		break;
	    }
	case '\"':		// "
	    {
		if (lan_active != 0)
		    talk();

		break;
	    }
	case 18:		// ctrl-r
	    {
		if (k_pin14 == 0) {
		    k_pin14 = 1;
		    netkeyer(K_SET14, "1");
		} else {
		    k_pin14 = 0;
		    netkeyer(K_SET14, "0");
		}
		break;
	    }

	case 20:		// ctrl-t
	case 233:		// alt-I
	    {
		if (lan_active != 0) {

		    for (t = 0; t <= 5; t++)
			mvprintw(14 + t, 1,
				 "                                                            ");
		    for (t = 0; t <= 4; t++)
			mvprintw(15 + t, 1, talkarray[t]);
		    nicebox(14, 0, 5, 59, "Messages");

		    refreshp();
		    getchar();
		    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
		    for (t = 0; t <= 6; t++)
			mvprintw(14 + t, 0,
				 "                                                             ");

		    clear_display();
		}
		break;
	    }

	}	/* end switch */

	/* convert to upper case */
	if (x >= 'a' && x <= 'z')
	    x = x - 32;

	if (x >= '/' && x <= 'Z') {
	    if (strlen(hiscall) < 13) {
		instring[0] = x;
		instring[1] = '\0';
		addch(x);
		strcat(hiscall, instring);
		if (cwstart != 0 && trxmode == CWMODE && contest == 1) {
		    /* early start keying after 'cwstart' characters */
		    if (strlen(hiscall) == cwstart) {
			strcpy(buffer, hiscall);
			early_started = 1;
			sending_call = 1;
			sendbuf();
			sending_call = 0;
			strcpy(hiscall_sent, hiscall);
		    }
		}
	    }

	    refreshp();

	    if (atoi(hiscall) < 1800) {	/*  no frequency */

		strncpy(dupecall, hiscall, 16);

		y = getctydata(dupecall);

		showinfo(y);

		searchlog(hiscall);


	    } else {
		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(24, 0,
			 "                                                           ");
		mvprintw(12, 29 + strlen(hiscall), "");
	    }
	    refreshp();

	}

	if ((x == '\n') || x == 32 || x == 9 || x == 11 || x == 44
	    || x == 92)
	    break;

	if (trxmode == DIGIMODE && (keyerport == GMFSK 
		|| keyerport == MFJ1278_KEYER)) {
	    show_rtty();
	    refreshp();
	}

    }

    return (x);
}

int play_file(char *audiofile)
{

    extern int txdelay;

    int fd,rc;
    char playcommand[120];

    if (*audiofile == '\0')
	return (0);

    if ((fd = open(audiofile, O_RDONLY, 0664)) < 0) {
	mvprintw(24, 0, "cannot open sound file %s!", audiofile);
    } else {
	close(fd);
	if (access("./play_vk", X_OK) == 0 ) {
	   sprintf( playcommand, "./play_vk %s", audiofile); 
	}
	else {
	   sprintf( playcommand, "play_vk %s", audiofile);
	}
	netkeyer(K_PTT, "1");	// ptt on
	usleep(txdelay * 1000);
	rc=system(playcommand);
	printcall();
	netkeyer(K_PTT, "0");	// ptt off
    }

    return (0);
}


void send_bandswitch(int freq)
{
    extern int use_bandoutput;
    extern int bandinx;
    extern int bandindexarray[];

    char outnibble[3];
    int bandswitch = 0;

    if (use_bandoutput == 1) {
	if (freq > 15) {	// cannot be a freq...
	    switch ((int) freq) {
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
}
