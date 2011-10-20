/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *                         2011 Thomas Beierlein <tb@forth-ev.de>
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

#include "background_process.h"
#include "set_tone.h"
#include "rtty.h"

extern int stop_backgrnd_process;
extern int this_second;
extern int cluster;
extern int packetinterface;
extern int lan_active;
extern char lan_message[];
extern char lan_recv_message[];
extern int recv_error;
extern char thisnode;
extern int lanspotflg;
extern char talkarray[5][62];
extern float node_frequencies[MAXNODES];
extern int qsonum;
extern char qsonrstr[5];
extern int lanqsos;
extern int highqsonr;
extern char zone_export[];
extern long lantime;
extern long timecorr;
extern int timeoffset;
extern char call[];
extern int trxmode;
extern int keyerport;

int background_process(void)
{

    extern int landebug;
    extern struct tm *time_ptr;

    static int i, t;
    static char prmessage[256];
    static int lantimesync = 0;

    int n;

    char debugbuffer[160];
    FILE *fp;

    i = 1;

    while (i) {

	while (stop_backgrnd_process == 1) {
	    sleep(1);
	}

	usleep(10000);

	if (packetinterface != 0) {
	    receive_packet();

	}

	if (trxmode == DIGIMODE
	    && (keyerport == MFJ1278_KEYER || keyerport == GMFSK))
	    rx_rtty();

	if (stop_backgrnd_process == 0) {
	    write_keyer();
	    cw_simulator();
	}

	if (lan_active == 1) {
	    if (lan_message[0] == '\0') {

		if (lan_recv() < 0) {
		    recv_error++;
		} else {
		    lan_message[strlen(lan_message) - 1] = '\0';

		}
	    }

	    if (landebug == 1) {
		if ((fp = fopen("debuglog", "a")) == NULL) {
		    fprintf(stdout,
			    "store_qso.c: Error opening debug file.\n");

		}
		get_time();
		strftime(debugbuffer, 80, "%H:%M:%S-", time_ptr);
		if (strlen(lan_message) > 2) {
		    strcat(debugbuffer, lan_message);
		    strcat(debugbuffer, "\n");
		    fputs(debugbuffer, fp);
		}
		debugbuffer[0] = '\0';

		fclose(fp);
	    }
	    if ((*lan_message != '\0') && (lan_message[0] == thisnode)) {
		mvprintw(24, 0,
		   "Warning: NODE ID CONFLICT ?! You should use another ID! ");
		refreshp();
		sleep(5);
	    }

	    if ((*lan_message != '\0')
		&& (lan_message[0] != thisnode)
		&& (stop_backgrnd_process != 1)) {

		switch (lan_message[1]) {

		case LOGENTRY:

		    log_to_disk(true);
		    break;

		case CLUSTERMSG:
		    strncpy(prmessage, lan_message + 2, 80);
		    if (strstr(prmessage, call) != NULL)	// alert for cluster messages
		    {
			mvprintw(24, 0,
				 "                                                                           ");
			mvprintw(24, 0, "%s", prmessage);
			refreshp();
		    }

		    addtext(prmessage);
		    break;
		case TLFSPOT:
		    strncpy(prmessage, lan_message + 2, 80);
		    lanspotflg = 1;
		    addtext(prmessage);
		    lanspotflg = 0;
		    break;
		case TLFMSG:
		    for (t = 0; t < 4; t++)
			strcpy(talkarray[t], talkarray[t + 1]);

		    talkarray[4][0] = lan_message[0];
		    talkarray[4][1] = ':';
		    talkarray[4][2] = '\0';
		    strncat(talkarray[4], lan_message + 2, 60);
		    mvprintw(24, 0,
			     "                                                                           ");
		    mvprintw(24, 0, " MSG from %s", talkarray[4]);
		    refreshp();
		    break;
		case FREQMSG:
		    if ((lan_message[0] >= 'A')
			&& (lan_message[0] <= 'A' + MAXNODES)) {
			node_frequencies[lan_message[0] - 'A'] =
			    atof(lan_message + 2);
			break;
		    }
		case INCQSONUM:

		    n = atoi(lan_message + 2);

		    if (highqsonr < n)
			highqsonr = n;

		    if ((qsonum <= n) && (n > 0)) {
			qsonum = highqsonr + 1;
			qsonr_to_str();
		    }
		    lan_message[0] = '\0';

		case TIMESYNC:
		    if ((lan_message[0] >= 'A')
			&& (lan_message[0] <= 'A' + MAXNODES)) {
			lantime = atoi(lan_message + 2);

			if (lantimesync == 1)
			    timecorr =
				((4 * timecorr) + lantime -
				 (time(0) + (timeoffset * 3600))) / 5;
			else {
			    timecorr =
				lantime - (time(0) + (timeoffset * 3600));
			    lantimesync = 1;
			}

			break;
		    }
		}

		lan_message[0] = '\0';
		lan_message[1] = '\0';

	    }

	}

	gettxinfo();		/* get freq info from TRX */

    }

    return (0);
}

int cw_simulator(void)
{

    extern char callmasterarray[MAX_CALLMASTER][14];
    extern int simulator;
    extern int simulator_mode;
    extern char buffer[];
    extern int simulator_seed;
    extern char simulator_tone[5];
    extern char tonestr[5];
    extern char tonecpy[5];
    extern int system_secs;
    extern int this_second;

    static int callnumber;
    char callcpy[80];
    static int x;
    char datacpy[160];

    if (simulator == 0)
	return (-1);

    if (simulator_mode == 1) {

	attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
	mvprintw(0, 3, "Sim");
	refreshp();

	strcpy(tonecpy, tonestr);

	switch (this_second) {
	case 48:
	    strcpy(simulator_tone, "625");
	    break;

	case 49:
	    strcpy(simulator_tone, "800");
	    break;

	case 50:
	    strcpy(simulator_tone, "650");
	    break;

	case 51:
	    strcpy(simulator_tone, "750");
	    break;

	case 52:
	    strcpy(simulator_tone, "700");
	    break;

	case 53:
	    strcpy(simulator_tone, "725");
	    break;

	case 54:
	    strcpy(simulator_tone, "675");
	    break;

	case 55:
	    strcpy(simulator_tone, "775");
	    break;

	case 56:
	    strcpy(simulator_tone, "600");
	    break;

	case 57:
	    strcpy(simulator_tone, "640");
	    break;

	default:
	    strcpy(simulator_tone, "750");
	    break;

	}

	strcpy(tonestr, simulator_tone);

	write_tone();

	callnumber =
	    callnumber + simulator_seed + system_secs -
	    (60 * (int) (system_secs / 60));

	if (callnumber >= 27000)
	    callnumber -= 27000;

	strcpy(buffer, callmasterarray[callnumber]);
	sendbuf();
	write_keyer();
	buffer[0] = '\0';
	simulator_mode = 0;

	strcpy(tonestr, tonecpy);
	write_tone();
    }

    if (simulator_mode == 2) {

	strcpy(tonecpy, tonestr);
	strcpy(tonestr, simulator_tone);
	write_tone();

	strcpy(callcpy, callmasterarray[callnumber]);

	x = getctydata(callcpy);

	buffer[0] = '\0';
	strcat(buffer, "TU 5NN ");

	strncpy(datacpy, zone_export, 2);
	strncat(buffer, datacpy, 2);

	sendbuf();
	buffer[0] = '\0';
	simulator_mode = 0;
	write_keyer();

	strcpy(tonestr, tonecpy);
	write_tone();

    }
    if (simulator_mode == 3) {

	strcpy(tonecpy, tonestr);
	strcpy(tonestr, simulator_tone);
	write_tone();

	strcpy(callcpy, callmasterarray[callnumber]);
	x = getctydata(callcpy);

	buffer[0] = '\0';
	strcat(buffer, "DE ");
	strcat(buffer, callmasterarray[callnumber]);
	strcat(buffer, " TU 5NN ");

	strncpy(datacpy, zone_export, 2);
	strncat(buffer, datacpy, 2);

	sendbuf();
	buffer[0] = '\0';
	simulator_mode = 0;
	write_keyer();

	strcpy(tonestr, tonecpy);
	write_tone();

    }

    return (0);
}
