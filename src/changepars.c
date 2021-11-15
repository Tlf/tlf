/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011-2013      Thomas Beierlein <tb@forth-ev.de>
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
 *
 *          parameterdialog
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "audio.h"
#include "cqww_simulator.h"
#include "changepars.h"
#include "clear_display.h"
#include "dxcc.h"
#include "editlog.h"
#include "err_utils.h"
#include "fldigixmlrpc.h"
#include "gettxinfo.h"
#include "ignore_unused.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "listmessages.h"
#include "logview.h"
#include "messagechange.h"
#include "muf.h"
#include "netkeyer.h"
#include "parse_logcfg.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "readcalls.h"
#include "readqtccalls.h"
#include "rules.h"
#include "scroll_log.h"
#include "searchlog.h"
#include "setcontest.h"
#include "sendbuf.h"
#include "set_tone.h"
#include "show_help.h"
#include "showpxmap.h"
#include "splitscreen.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "writecabrillo.h"
#include "addmult.h"

void center_fkey_header();

void wipe_display();

int changepars(void) {

    char parameterstring[20] = "";
    char parameters[52][19];
    int i, k, x, nopar = 0;
    int maxpar = 50;
    int volumebuffer;
    int currentmode = 0;

    strcpy(parameters[0], "SPOT");
    strcpy(parameters[1], "MAP");
    strcpy(parameters[2], "CLOFF");
    strcpy(parameters[3], "CLUSTER");
    strcpy(parameters[4], "SHORT");
    strcpy(parameters[5], "LONG");
    strcpy(parameters[6], "MESSAGE");
    strcpy(parameters[7], "LIST");
    strcpy(parameters[8], "CHECK");
    strcpy(parameters[9], "NOCHECK");
    strcpy(parameters[10], "TONE");
    strcpy(parameters[11], "EDIT");
    strcpy(parameters[12], "VIEW");
    strcpy(parameters[13], "HELP");
    strcpy(parameters[14], "DEMODE");
    strcpy(parameters[15], "CONTEST");
    strcpy(parameters[16], "FILTER");
    strcpy(parameters[17], "SCORE");
    strcpy(parameters[18], "WRITE");
    strcpy(parameters[19], "EXIT");
    strcpy(parameters[20], "TXFILE");
    strcpy(parameters[21], "ZONES");
    strcpy(parameters[22], "CTY");
    strcpy(parameters[23], "MODE");
    strcpy(parameters[24], "SET");
    strcpy(parameters[25], "MULTI");
    strcpy(parameters[26], "PROP");
    strcpy(parameters[27], "RITCLEAR");
    strcpy(parameters[28], "TRXCONTROL");
    strcpy(parameters[29], "CFG");
    //strcpy(parameters[30],  "CWMODE");
    strcpy(parameters[30], "CW");
    strcpy(parameters[31], "SSBMODE");
    strcpy(parameters[32], "DIGIMODE");
    strcpy(parameters[33], "PACKET");
    strcpy(parameters[34], "SIMULATOR");
    strcpy(parameters[35], "INFO");
    strcpy(parameters[36], "FREQ");
    strcpy(parameters[37], "RECONNECT");
    strcpy(parameters[38], "QUIT");
    strcpy(parameters[39], "CQDELAY");
    strcpy(parameters[40], "ADIF");
    strcpy(parameters[41], "SYNC");
    strcpy(parameters[42], "RESCORE");
    strcpy(parameters[43], "SCVOLUME");
    //strcpy(parameters[44], "SCAN");	/* 05jan18 no longer supported */
    strcpy(parameters[44], "");
    strcpy(parameters[45], "");         //"DEBUG");
    strcpy(parameters[46], "MINITERM");
    strcpy(parameters[47], "RTTY");
    strcpy(parameters[48], "SOUND");
    strcpy(parameters[49], "CWMODE");
    strcpy(parameters[50], "CHARS");
    strcpy(parameters[51], "FLDIGI");

    nopar = 0;

    attroff(A_STANDOUT);
    attron(COLOR_PAIR(C_HEADER));
    mvprintw(12, 29, "PARAMETER?  ");
    refreshp();
    usleep(400000);
    mvprintw(12, 29, "            ");

    echo();
    mvgetnstr(12, 29, parameterstring, 19);
    noecho();

    for (k = 0; parameterstring[k]; k++)
	parameterstring[k] = toupper(parameterstring[k]);

    for (i = 0; i <= maxpar; i++) {
	if (strncmp(parameterstring, parameters[i], 3) == 0) {
	    break;
	}
    }

    switch (i) {
	case 0: {		/* SPOTS) */
	    /* SPOTS not supported anymore
	     * - default to MAP*/
	    cluster = MAP;
	    break;
	}
	case 1: {		/* BANDMAP */
	    cluster = MAP;
	    break;
	}
	case 2: {		/* CLOFF  */
	    cluster = NOCLUSTER;
	    break;
	}
	case 3: {		/* CLUSTER  */
	    cluster = CLUSTER;
	    announcefilter = FILTER_ALL;
	    break;
	}
	case 4: {		/* SHORTNR  */
	    shortqsonr = SHORTCW;
	    break;
	}
	case 5: {		/* LONGNR  */
	    shortqsonr = LONGCW;
	    break;
	}
	case 6: {		/* MESSAGE  */
	    message_change();
	    break;
	}

	case 7: {		/* LIST  */
	    listmessages();
	    break;
	}
	case 8: {		/* CHECK  */
	    searchflg = SEARCHWINDOW;
	    break;
	}
	case 9: {		/* NOCHECK  */
	    searchflg = 0;
	    break;
	}
	case 10: {		/*  TONE   */
	    set_tone();
	    break;
	}
	case 11: {		/*  EDIT   */
	    logedit();
	    break;
	}
	case 12: {		/*  VIEW   */
	    logview();
	    break;
	}
	case 13: {		/*  HELP   */
	    show_help();
	    break;
	}
	case 14: {		/*  DEMODE   */
	    if (demode == SEND_DE)
		demode = 0;
	    else
		demode = SEND_DE;
	    mvprintw(13, 29, "DE-mode is %d", demode);
	    refreshp();
	    sleep(1);

	    break;
	}
	case 15: {		/*  CONTEST   */
	    if (iscontest)
		iscontest = false;
	    else {
		iscontest = true;
		searchflg = SEARCHWINDOW;
	    }
	    mvprintw(13, 29, "CONTEST-mode is %s", iscontest ? "on" : "off");
	    refreshp();
	    sleep(1);

	    break;
	}
	case 16: {		/*  FILTER   */
	    announcefilter++;
	    if (announcefilter > 3)
		announcefilter = 0;
	    mvprintw(13, 29, "FILTER-mode is %d", announcefilter);
	    refreshp();
	    sleep(1);

	    break;
	}
	case 17: {		/*  SCORE   */
	    if (showscore_flag == 0)
		showscore_flag = 1;
	    else {
		showscore_flag = 0;

	    }
	    mvprintw(13, 29, "Show score-mode is %d", showscore_flag);
	    refreshp();
	    sleep(1);

	    break;
	}
	case 18: {		/*  WRITE CABRILLO FILE   */
	    int old_cluster = cluster;
	    cluster = NOCLUSTER;

	    write_cabrillo();

	    cluster = old_cluster;


	    break;
	}
	case 19:		/* EXIT */
	case 38: {		/* QUIT */
	    exit(EXIT_SUCCESS);
	}
	case 20: {		/*  TXFILE   */
	    break;
	}
	case 21: {		/*  ZONES   */
	    if (zonedisplay == 0)
		zonedisplay = 1;
	    else {
		zonedisplay = 0;
	    }

	    break;
	}
	case 22: {		/* COUNTRIES */
	    show_mults();
	    refreshp();
	    sleep(1);

	    break;
	}
	case 23: {		/*  MODE   */
	    if (trxmode == CWMODE)
		trxmode = SSBMODE;
	    else if (trxmode == SSBMODE)
		trxmode = DIGIMODE;
	    else
		trxmode = CWMODE;

	    if (trxmode == CWMODE) {
		mvprintw(13, 29, "TRXMODE = CW");
	    } else if (trxmode == SSBMODE)
		mvprintw(13, 29, "TRXMODE = SSB");
	    else
		mvprintw(13, 29, "TRXMODE = DIG");
	    refreshp();
	    sleep(1);

	    break;
	}
	case 24:		/* SET PARAMETERS */
	case 29: {		/* CFG PARAMETERS */
	    clear();

	    edit(config_file);

	    read_logcfg();
	    read_rules();	/* also reread rules file */
	    TLF_LOG_INFO("Logcfg.dat loaded, parameters written.");
	    center_fkey_header();
	    clear_display();
	    break;
	}
	case 25: {		/*  MULTI   */
	    multiplierinfo();

	    break;
	}
	case 26: {		/* PROPAGATION */
	    muf();
	    clear_display();
	    break;
	}
	case 27: {		/*  RITCLEAR   */
	    if (rit == RITCLEAR)
		rit = 0;
	    else {
		rit = RITCLEAR;

	    }
	    if (rit == RITCLEAR) {
		mvprintw(13, 29, "RIT clear on");
	    } else {
		mvprintw(13, 29, "RIT clear off");
	    }
	    refreshp();
	    sleep(1);

	    break;
	}
	case 28: {		/*  trx ctl   */
	    trx_control = !trx_control;

	    if (trx_control) {
		mvprintw(13, 29, "TRX control on");
	    } else {
		mvprintw(13, 29, "TRX control off");
	    }
	    refreshp();
	    sleep(1);

	    break;
	}
	case 30:			/* CW  */
	case 49: {
	    if (cwkeyer == MFJ1278_KEYER) {
		sendmessage("MODE CW\015K\015");
	    }
	    trxmode = CWMODE;
	    set_outfreq(SETCWMODE);
	    break;
	}
	case 31: {		/* SSBMODE  */
	    trxmode = SSBMODE;
	    set_outfreq(SETSSBMODE);
	    break;
	}
	case 32: {		/* DIGIMODE  */
	    trxmode = DIGIMODE;
	    set_outfreq(SETDIGIMODE);
	    break;
	}
	case 33: {		/* PACKET  */
	    if (!nopacket && packetinterface > 0)
		packet();
	    break;
	}
	case 34: {		/* SIMULATOR  */
	    if (!CONTEST_IS(CQWW)) {
		TLF_LOG_INFO(
		    "Simulator mode is only supported for CQWW contest!");
		break;
	    }

	    if (callmaster == NULL) {
		TLF_LOG_INFO(
		    "Simulator mode needs callmaster database");
		break;
	    }

	    if (!simulator) {

		simulator = true;
		cqmode = CQ;
		if (ctcomp == 1) {
		    TLF_LOG_INFO(
			"The simulator only works in TRmode. Switching to TRmode");
		    ctcomp = 0;
		} else {
		    mvprintw(13, 29, "Simulator on");
		    refreshp();
		    sleep(1);
		}

		if (cwkeyer == NET_KEYER) {

		    if (netkeyer(K_WORDMODE, NULL) < 0) {
			TLF_LOG_INFO("keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
		    }
		}
	    } else {
		simulator = false;
		mvprintw(13, 29, "Simulator off");
		refreshp();
		sleep(1);

		if (cwkeyer == NET_KEYER) {

		    if (netkeyer(K_RESET, NULL) < 0) {
			TLF_LOG_INFO("keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
		    }
		}
	    }
	    break;
	}
	case 35: {		/* INFO  */
	    int currentterm = miniterm;
	    miniterm = 0;
	    networkinfo();
	    miniterm = currentterm;

	    if (currentmode == DIGIMODE)
		trxmode = DIGIMODE;
	    break;
	}
	case 36: {		/* CLOFF  */
	    cluster = FREQWINDOW;
	    break;
	}

	case 37: {		/* RECONNECT  */
	    if (!nopacket && packetinterface > 0) {
		cleanup_telnet();
		init_packet();
		packet();
	    }
	    break;
	}

	case 39: {		/* CQDELAY */
	    mvprintw(12, 29, "CQD: pgup/dwn");
	    refreshp();

	    x = 1;
	    while (x) {
		x = key_get();

		switch (x) {

		    // <Page-Up>, increase autoCQ delay by 1/2 second.
		    case KEY_PPAGE: {
			if (cqdelay <= 60) {
			    cqdelay++;
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
			    mvprintw(0, 19, "  ");
			    mvprintw(0, 19, "%i", cqdelay);
			    break;

			}
		    }

		    // <Page-Down>, decrease autoCQ delay by 1/2 second.
		    case KEY_NPAGE: {
			if (cqdelay >= 1) {
			    cqdelay--;
			    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

			    mvprintw(0, 19, "  ");
			    mvprintw(0, 19, "%i", cqdelay);
			    break;

			}
			default:
			    x = 0;
			}

		}
	    }

	    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

	    move(12, 29 + strlen(hiscall));
	    break;

	}
	case 40: {		/* ADIF */
	    write_adif();

	    break;
	}
	case 41: {		/* SYNC */
	    if (strlen(synclogfile) > 0)
		synclog(synclogfile);
	    scroll_log();
	    /** \todo register return value */
	    log_read_n_score();
	    clear_display();
	    break;
	}
	case 42: {		/* RESCORE */
	    /** \todo register return value */
	    log_read_n_score();
	    clear_display();
	    break;
	}
	case 43: {		/* SCVOLUME - set soundcard volume */
	    volumebuffer = atoi(sc_volume);
	    mvprintw(12, 29, "Vol: pgup/dwn");
	    refreshp();
	    usleep(500000);
	    mvprintw(12, 29, "Vol:         ");
	    mvprintw(12, 29, "Vol: %d", volumebuffer);

	    x = 1;
	    while (x) {
		x = key_get();

		switch (x) {

		    // <Page-Up>, increase volume by 5%.
		    case KEY_PPAGE: {
			if (volumebuffer < 95)
			    volumebuffer += 5;

			break;
		    }
		    // <Page-Down>, decrease volume by 5%.
		    case KEY_NPAGE: {
			if (volumebuffer >= 5)
			    volumebuffer -= 5;

			break;
		    }
		    default:
			x = 0;

		}

		attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
		mvprintw(12, 34, "  ");
		mvprintw(12, 34, "%d", volumebuffer);

		if (volumebuffer >= 0 && volumebuffer <= 99)
		    sprintf(sc_volume, "%d", volumebuffer);

		netkeyer(K_STVOLUME, sc_volume);
	    }

	    clear_display();
	    break;
	}
	case 46: {		/* MINITERM ON/OFF */
	    if (miniterm == 1)
		miniterm = 0;
	    else
		miniterm = 1;
	    break;
	}
	case 47: {		/* RTTY Initialize mode (MFJ1278B controller) */
	    sendmessage("MODE VB\015K\015");
	    trxmode = DIGIMODE;

	    break;
	}
	case 48: {		/* SOUND */
	    clear_display();
	    record();
	    clear_display();
	    break;
	}
	case 50: {		/* CHARS */
	    mvprintw(13, 29, "Autosend: (0, 2..5, m)?");
	    refreshp();
	    x = 1;

	    /* wait for correct input or ESC */
	    while ((x != 0) && !((x >= 2) && (x <= 5)) && !(x == 'm' - '0')) {
		x = key_get();
		if (x == ESCAPE)
		    break;
		x = x - '0';
	    }

	    /* remember new setting */
	    if (x != ESCAPE) {
		if (x == 0 || (x >= 2 && x <= 5))
		    cwstart = x;
		else
		    cwstart = -1;
	    }

	    if (cwstart > 0)
		mvprintw(13, 29, "Autosend now: %1d        ",
			 cwstart);
	    else {
		if (cwstart < 0)
		    mvprintw(13, 29, "Autosend now: Manual   ");
		else
		    mvprintw(13, 29, "Autosend now: OFF      ");
	    }
	    refreshp();
	    break;

	}
	case 51: {              /* FLDIGI - turn on/off */
	    if (digikeyer == FLDIGI) {
		if (fldigi_toggle()) {
		    fldigi_clear_connerr();
		    mvprintw(13, 29, "FLDIGI ON");
		} else {
		    mvprintw(13, 29, "FLDIGI OFF");
		}
		refreshp();
	    }
	    break;
	}
	default: {
	    nopar = 1;
	}
    }

    if (nopar != 1) {
	mvprintw(12, 29, "OK !        ");
    } else {
	if (!nopacket && packetinterface > 0)
	    packet();
    }

    refreshp();

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvprintw(12, 29, "            ");
    move(12, 29);
    refreshp();
    hiscall[0] = '\0';

    return (0);
}

/* -------------------------------------------------------------- */

void networkinfo(void) {

    wipe_display();

    if (lan_active)
	mvprintw(1, 10, "Network status: on");
    else
	mvprintw(1, 10, "Network status: off");

    mvprintw(3, 28, "Packets rcvd: %d | %d", recv_packets, recv_error);

    for (int i = 0; i < nodes; i++) {
	mvprintw(4 + i, 10, "%s", bc_hostaddress[i]);
	mvprintw(4 + i, 28, "Packets sent: %d | %d ",
		 send_packets[i], send_error[i]);
    }

    if (strlen(config_file) > 0)
	mvprintw(6 + nodes, 10, "Config file: %s", config_file);
    else
	mvprintw(6 + nodes, 10,
		 "Config file: /usr/local/share/tlf/logcfg.dat");//FIXME
    mvprintw(7 + nodes, 10, "Contest    : %s", whichcontest);
    mvprintw(8 + nodes, 10, "Logfile    : %s", logfile);

    mvprintw(9 + nodes, 10, "Cluster    : %s", pr_hostaddress);
    mvprintw(10 + nodes, 10, "TNCport    : %s", tncportname);
    mvprintw(11 + nodes, 10, "RIGport    : %s", rigportname);
    if (use_bandoutput == 1)
	mvprintw(12 + nodes, 10, "Band output: on");
    else
	mvprintw(12 + nodes, 10, "Band output: off");

    mvprintw(13 + nodes, 10, "callmaster : %s",
	     (callmaster_version[0] != 0 ? callmaster_version : "n/a"));
    mvprintw(14 + nodes, 10, "cty.dat    : %s",
	     (cty_dat_version[0] != 0 ? cty_dat_version : "n/a"));

    refreshp();

    mvprintw(23, 22, " --- Press a key to continue --- ");
    refreshp();

    (void)key_get();

    clear_display();
    return;
}

/* -------------------------------------------------------------- */

void multiplierinfo(void) {

    int j, k, vert, hor, cnt, found;
    char mprint[50];

    wipe_display();

    if (CONTEST_IS(ARRL_SS)) {
	int attributes;
	char chmult[6];
	char ch2mult[6];

	mvprintw(2, 20, "ARRL SWEEPSTAKES -- REMAINING SECTIONS");
	cnt = 0;
	for (vert = 9; vert < 18; vert++) {

	    if (cnt >= get_mult_count())
		break;

	    for (hor = 5; hor < 15; hor++) {
		if (cnt >= get_mult_count())
		    break;

		g_strlcpy(chmult, get_mult(cnt), sizeof(chmult));

		/* check if in worked multis */
		found = 0;
		for (j = 0; j < nr_multis; j++) {
		    g_strlcpy(ch2mult, multis[j].name, sizeof(ch2mult));

		    if (strcmp(g_strchomp(ch2mult), chmult) == 0)
			found = 1;
		}

		if (found == 1)
		    attributes = COLOR_PAIR(C_HEADER) | A_STANDOUT;
		else
		    attributes = COLOR_PAIR(C_WINDOW) | A_STANDOUT;

		attron(modify_attr(attributes));

		g_strlcpy(mprint, get_mult(cnt), 5);
		mvprintw(vert, hor * 4, "%s", mprint);

		cnt++;
	    }
	}
    }

    if (serial_section_mult == 1 || sectn_mult_once
	    || (sectn_mult == 1 && !CONTEST_IS(ARRL_SS))) {
	char *tmp;
	int worked_at;

	mvprintw(0, 30, "REMAINING SECTIONS");
	cnt = 0;
	for (vert = 2; vert < LINES - 2; vert++) {
	    if (cnt >= get_mult_count())
		break;

	    for (hor = 0; hor < 7; hor++) {
		if (cnt >= get_mult_count())
		    break;

		worked_at = 0;

		/* lookup if already worked */
		for (k = 0; k < nr_multis; k++) {
		    if (strstr(multis[k].name, get_mult(cnt)) != NULL) {
			worked_at = multis[k].band;
			break;
		    }
		}

		tmp = g_strndup(get_mult(cnt), 4);
		sprintf(mprint, "%-4s", tmp);
		g_free(tmp);

		strcat(mprint, (worked_at & BAND160) ? "*" : "-");
		strcat(mprint, (worked_at & BAND80) ? "*" : "-");
		strcat(mprint, (worked_at & BAND40) ? "*" : "-");
		strcat(mprint, (worked_at & BAND20) ? "*" : "-");
		strcat(mprint, (worked_at & BAND15) ? "*" : "-");
		strcat(mprint, (worked_at & BAND10) ? "*" : "-");

		mprint[11] = '\0';
		mvprintw(vert, 2 + hor * 11, "%s", mprint);

		cnt++;
	    }
	}
    }

    attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    mvprintw(LINES - 2, 22, " --- Press a key to continue --- ");

    refreshp();

    (void)key_get();

    clear_display();
    return;
}

void wipe_display() {
    int j;

    attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (j = 0; j < LINES; j++)
	mvprintw(j, 0, "%s", backgrnd_str);
}
