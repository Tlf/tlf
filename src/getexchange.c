/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@eudx.org>
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

	/* ------------------------------------------------------------
	 *        Getexchange handles  the  comment field
	 *
	 *--------------------------------------------------------------*/

#include "getexchange.h"
#include "recall_exchange.h"

int play_file(char *audiofile);

int getexchange(void)
{
    extern int contest;
    extern int use_rxvt;
    extern char comment[];
    extern char cqzone[];
    extern char ituzone[];
    extern char my_rst[];
    extern char speedstr[];
    extern int change_rst;
    extern int cury, curx;
    extern char message[15][80];
    extern char ph_message[14][80];
    extern char hiscall[];
    extern char buffer[];
    extern char qsonrstr[];
    extern int cqww;
    extern int wpx;
    extern int pacc_pa_flg;
    extern int arrldx_usa;
    extern int arrl_fd;
    extern int exchange_serial;
    extern int countrynr;
    extern int mycountrynr;
    extern int sprint;
    extern int trxmode;
    extern int recall_mult;
    extern int arrlss;
    extern int lan_active;
    extern char lastqsonr[];
    extern char qsonrstr[];
    extern char call[];
    extern char section[];
    extern int serial_section_mult;
    extern int serial_grid4_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;
    extern int ctcomp;
    extern int wazmult;
    extern int itumult;
    extern int pfxmult;
    extern int exc_cont;
    extern char continent[];
    extern int keyerport;
    extern int commentfield;
//extern int dxped;

    int i;
    int x = 0;
    char instring[2];
    char commentbuf[40] = "";
    int retval;
    char *gridmult = "";
    int keyspeed = 30;
    char speedbuf[3] = "";

    instring[1] = '\0';

    if ((lan_active == 1) && (exchange_serial == 1)) {
	strncpy(lastqsonr, qsonrstr, 5);
	send_lan_message(INCQSONUM, qsonrstr);
    }

    if (recall_mult == 1)
	retval = recall_exchange();

    if ((arrldx_usa == 1) && (trxmode != CWMODE))
	retval = recall_exchange();

    if (arrl_fd == 1)
	retval = recall_exchange();

    if (((cqww == 1) || (wazmult == 1) || (itumult == 1))
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

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    mvprintw(12, 54, "                          ");
    mvprintw(12, 54, comment);
    commentfield = 1;
    for (i = strlen(comment); i <= 25; i++) {
	x = onechar();

	if (x == 127 && (strlen(comment) >= 1)) {	/* erase  */

	    getyx(stdscr, cury, curx);
	    mvprintw(12, curx - 1, " ");
	    mvprintw(12, curx - 1, "");
	    comment[strlen(comment) - 1] = '\0';
	    i -= 2;
	}

	switch (x) {

	case 27:
	    {
		if (comment[0] != '\0') {
		    comment[0] = '\0';
		    mvprintw(12, 54, "                          ");
		    mvprintw(12, 54, "");
		    stoptx();
		    i = 0;
		    break;
		} else {
		    hiscall[0] = '\0';
		    x = 9;
		    break;
		}
	    }
	case 160:		// for CT compatibility
	    {
		if (trxmode == CWMODE) {
		    strcat(buffer, message[1]);
		    sendbuf();
		    mvprintw(12, 29 + strlen(hiscall), "");
		} else
		    play_file(ph_message[1]);

		break;
	    }
	case '+':		// for CT compatibility
	    {
		if (strlen(hiscall) > 2) {
		    if (trxmode == CWMODE) {
			strcat(buffer, message[2]);	/* F3 */
			sendbuf();
			mvprintw(12, 29 + strlen(hiscall), "");
		    } else
			play_file(ph_message[2]);

		    x = 92;
		}
		break;
	    }

	case 129:
	    {
		if (trxmode == CWMODE) {
		    strcat(buffer, call);	/* F1 */
		    mvprintw(5, 0, "");
		    sendbuf();
		} else
		    play_file(ph_message[5]);	// call

		mvprintw(12, 54, comment);
		break;
	    }

	case 130 ... 137:
	    {
		if (trxmode == CWMODE) {
		    strcat(buffer, message[x - 129]);	/* F2 */
		    mvprintw(5, 0, "");
		    sendbuf();
		} else
		    play_file(ph_message[x - 129]);

		mvprintw(12, 54, comment);
		break;
	    }
	case 176 ... 186:
	    {
		strcat(buffer, message[x - 162]);	/* alt-0 to alt-9 */
		mvprintw(5, 0, "");
		sendbuf();
		mvprintw(12, 54, comment);

		break;
	    }

	case 156:		/* change MY RST */
	    {
		if (change_rst == 1) {
		    if (my_rst[1] <= 56) {

			my_rst[1]++;

			mvprintw(12, 49, my_rst);
			mvprintw(12, 54, comment);
		    }
		} else {	/* speed up */
		    keyspeed = speedup();
		    strncpy(speedbuf, speedstr + (2 * keyspeed), 2);
		    speedbuf[2] = '\0';
//                                                              attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);

		    mvprintw(0, 14, "%s", speedbuf);
		    mvprintw(12, 54, comment);
		    refresh();

		}
		break;

	    }
	case 157:
	    {
		if (change_rst == 1) {

		    if (my_rst[1] >= 49) {
			my_rst[1]--;

			mvprintw(12, 49, my_rst);
			mvprintw(12, 54, comment);

		    }
		} else {
		    keyspeed = speeddown();
		    strncpy(speedbuf, speedstr + (2 * keyspeed), 2);
		    speedbuf[2] = '\0';
		    //                                              attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
		    mvprintw(0, 14, "%s", speedbuf);
		    mvprintw(12, 54, comment);
		    refresh();
		}
		break;

	    }
	case 44:		// , keyer
	    {
		getyx(stdscr, cury, curx);
		mvprintw(5, 0, "");
		keyer();
		mvprintw(cury, curx, "");
		x = 0;
		break;
	    }
	case '\n':
	    {
		if (ctcomp == 1)
		    x = 92;	// enter logs immediately
		if (trxmode == SSBMODE)
		    x = 92;
		if (contest != 1)
		    x = 92;
//                                                      if (dxped == 1) x = 92;
		break;
	    }

	}			/* end switch */

	if (x >= 97 && x <= 122)
	    x = x - 32;

	if (i < 25) {
	    if (x >= 32 && x <= 90) {
		instring[0] = x;
		addch(x);
		strcat(comment, instring);
		if (keyerport == GMFSK) {
		    show_rtty();
		    mvprintw(12, 54, comment);
		}
		refresh();
	    }
	} else
	    i--;

	if (serial_section_mult == 1)
	    x = checkexchange(x);
	if (dx_arrlsections == 1)
	    x = checkexchange(x);
	if (sectn_mult == 1)
	    x = checkexchange(x);
	if (arrlss == 1)
	    x = checkexchange(x);
	if (cqww == 1)
	    x = checkexchange(x);

	if (x == '\n' || x == 9 || x == 11 || x == 92) {

	    if ((exchange_serial == 1 && comment[0] >= '0' && comment[0] <= '9')) {	/* align serial nr. */
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

	    if (wpx == 1 && pfxmult == 0) {	/* align serial nr. */

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

	    if (sprint == 1) {

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

	    if ((pacc_pa_flg == 1) && (countrynr != mycountrynr)) {
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

	    if ((arrlss == 1) && (x != 9) && (strlen(section) < 2)) {
		mvprintw(13, 54, "section?");
		mvprintw(12, 54, comment);
		x = 0;
	    } else if (((serial_section_mult == 1) || (sectn_mult == 1))
		       && ((x != 9) && (strlen(section) < 1))) {
		mvprintw(13, 54, "section?");
		mvprintw(12, 54, comment);
		refresh();

		// x = 0;		//##debug 17jan10 tb
		break;

	    } else if (serial_grid4_mult == 1) {
		//      mvprintw(13,54, "section?");
		mvprintw(12, 54, comment);
		refresh();
		gridmult = getgrid(comment);
		strcpy(section, gridmult);
		section[4] = '\0';

		break;
//                              x = 0; //##debug

	    } else
		break;

	}

    }
    commentfield = 0;

    return (x);
}

/*  --------------------------------------------------------------------------*/

char cmpattern[] = "                               ";	// global
char ssexchange[30] = "";
char section[8] = "";
char callupdate[7];
int call_update = 0;
char zone_export[3] = "  ";
char zone_fix[3] = "";
/*  --------------------------------------------------------------------------*/

int checkexchange(int x)
{

    extern char comment[];
    extern char ssexchange[];
    extern char mults_possible[MAX_MULTS][12];
    extern int cqww;
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
    extern int max_multipliers;

    char precedent[] = " ";
    char serial[5] = "    ";
    char check[3] = "  ";
    char checksection[30];
    char zone[] = "  ";

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
    char secpats[10][6] = {
	"fab",
	"faab",
	"faaab",
	"bab",
	"baab",
	"baaab",
	"bau",
	"baau",
	"baaau",
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
    char sectionpats[9][6] = {
	"uab",
	"uaab",
	"uaaab",
	"uau",
	"uaau",
	"uaaau",
	"bab",
	"baab",
	"baaab"
    };

    int i, s, hr, ii, pr, jj;

    // get the pattern

    if (strlen(comment) < 2)
	strcpy(cmpattern, "u                    ");

    if (strlen(comment) > 0) {

	for (i = 0; i < strlen(comment); i++) {

	    switch ((int) comment[i]) {

	    case 'A'...'Z':{
		    cmpattern[i + 1] = 'a';
		    cmpattern[i + 2] = 'u';
		    break;
		}

	    case '0'...'9':{
		    cmpattern[i + 1] = 'f';
		    cmpattern[i + 2] = 'u';
		    break;
		}

	    case ' ':{
		    cmpattern[i + 1] = 'b';
		    break;
		}

	    default:
		cmpattern[i + 1] = 'u';
	    }
	}
    }

    // -----------------------------------cqww-----------------------
    if (cqww == 1) {

	s = atoi(comment);
	snprintf( zone, sizeof(zone), "%02d", s);

	hr = 0;

	for (ii = 0; ii < 6; ii++) {

	    hr = getlastpattern(zonepats[ii]);

// logik und Verwendung zone_fix vs zone_export unklar
	    if ((hr > 1) && (atoi(comment + hr - 1) != 0)) {
		sprintf(zone, "%02d", atoi(comment + hr - 1));
		strcpy(zone_fix, zone);
	    } else
		strcpy(zone_export, zone);
	}

	if (strlen(hiscall) >= 2)
	    mvprintw(8, 73, "%s", zone_export);

	hr = 0;

	for (ii = 0; ii < 5; ii++) {

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

    }

    // ---------------------------arrls------------------------------
    if (arrlss == 1) {

	serial[4] = '\0';

	// get serial nr.

	s = atoi(comment);

	if (s != 0)
	    sprintf(serial, "%4d", atoi(comment));

	for (ii = 0; ii < 8; ii++) {

	    hr = getlastpattern(serpats[ii]);

	    if (hr > 0)
		sprintf(serial, "%4d", atoi(comment + hr - 1));

	    if (ii == 5 && hr > 0) {
		sprintf(serial, "%4d", atoi(comment + hr - 1));
		sprintf(check, "%2d", atoi(comment + hr + 2));
	    }

	}
	
	// get precedent

	if (((comment[0] == 'A')
	     || (comment[0] == 'B')
	     || (comment[0] == 'M')
	     || (comment[0] == 'Q')
	     || (comment[0] == 'S'))
	    && ((comment[1] == ' ') || (cmpattern[2] == 'f'))) {

	    precedent[0] = comment[0];
	}

	hr = 0;
	for (ii = 0; ii < 8; ii++) {

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
	hr = 0;

	for (ii = 0; ii < 5; ii++) {

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
	hr = 0;

	for (ii = 0; ii < 6; ii++) {

	    hr = getlastpattern(checkpats[ii]);

	    if (hr > 0) {
		if ((atoi(comment + hr) <= 100)
		    || ((atoi(comment + hr) <= 9)
			&& (comment[hr] == '0'))) {
		    check[0] = comment[hr];
		    check[1] = comment[hr + 1];
		    check[2] = '\0';
		}

	    }

	}
	// get section
	hr = 0;

	for (ii = 0; ii < 10; ii++) {

	    hr = getlastpattern(secpats[ii]);

	    if (hr > 0) {

		strncpy(checksection, comment + (hr), 3);
		checksection[3] = '\0';

		for (jj = 0; jj < max_multipliers - 1; jj++) {

		    if ((strlen(mults_possible[jj]) >= 1) &&
			(strstr(checksection, mults_possible[jj]) != NULL)
			&& (strlen(checksection) ==
			    strlen(mults_possible[jj]))) {

			strcpy(section, mults_possible[jj]);
			break;
		    }
		}
	    }
	}
	hr = 0;

//	callupdate[0] = '\0';  /* not needed tb 09jan10 */

	mvprintw(8, 47, "                            ");
	mvprintw(8, 47, "       %s %s %s %s        ", serial, precedent,
		 check, section);
	ssexchange[0] = '\0';

	strcat(ssexchange, serial);
	strcat(ssexchange, " ");
	strcat(ssexchange, precedent);
	strcat(ssexchange, " ");
	strcat(ssexchange, check);
	strcat(ssexchange, " ");
	strcat(ssexchange, section);

	mvprintw(12, 54, comment);
	refresh();

	return (x);		// end arrlss
    }

    // ----------------------serial+section--------------------------
    if ((serial_section_mult == 1) || (sectn_mult == 1)
	|| (dx_arrlsections == 1)) {

	if (serial_section_mult == 1) {

	    // get serial nr.
	    serial[4] = '\0';

	    s = atoi(comment);

	    if (s != 0)
		sprintf(serial, "%4d", atoi(comment));

	    for (ii = 0; ii < 8; ii++) {

		hr = getlastpattern(serpats[ii]);

		if (hr > 0)
		    sprintf(serial, "%4d", atoi(comment + hr - 1));

		if (ii == 5 && hr > 0) {
		    sprintf(serial, "%4d", atoi(comment + hr - 1));
		    sprintf(check, "%2d", atoi(comment + hr + 2));
		}

	    }

	    // get section

	    hr = 0;

	    for (ii = 0; ii < 7; ii++) {

		hr = getlastpattern(secpats[ii]);

		if (hr > 0) {

		    memset(checksection, 0, 29);
		    strncpy(checksection, comment + (hr), 3);
		    if (checksection[strlen(checksection) - 1] == ' ') {
			checksection[strlen(checksection) - 1] = '\0';
		    }

		    for (jj = 0; jj < MAX_MULTS; jj++) {

			if ((strlen(mults_possible[jj]) >= 1)
			    && (strcmp(checksection, mults_possible[jj]) ==
				0)) {
			    strcpy(section, mults_possible[jj]);
			    break;	// new
			}
		    }
		}
	    }
	}			// end serial_section_mult
	if (sectn_mult == 1) {

	    hr = 0;

	    for (ii = 0; ii < 7; ii++) {

		hr = getlastpattern(sectionpats[ii]);

		strncpy(checksection, comment, 3);
		checksection[3] = '\0';
		for (jj = 0; jj < max_multipliers; jj++) {

		    if ((strlen(mults_possible[jj]) >= 1)
			&& (strstr(checksection, mults_possible[jj]) !=
			    NULL)) {

			strcpy(section, mults_possible[jj]);
		    }
		}
	    }

	}			//  end sectn_mult
	if (dx_arrlsections == 1) {

	    hr = 0;

	    for (ii = 0; ii < 7; ii++) {

		hr = getlastpattern(sectionpats[ii]);

		strncpy(checksection, comment, 3);
		checksection[3] = '\0';

		for (jj = 0; jj < max_multipliers; jj++) {

		    if ((strlen(mults_possible[jj]) ==
			 strlen(checksection))
			&& (strstr(checksection, mults_possible[jj]) !=
			    NULL)) {

			strcpy(section, mults_possible[jj]);

			// if (strlen(section) == strlen(mults_possible[jj])) break;
		    }
		}

	    }

	}			// end dx_arrlsections

	hr = 0;

	callupdate[0] = '\0';

    }


    // get call update
    hr = 0;

    for (ii = 0; ii < 5; ii++) {

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
    mvprintw(8, 73, "   ");
    mvprintw(8, 73, "%s", section);	/* show section on lower frame of 
					   Worked window */
    ssexchange[0] = '\0';

/*	if (serial_section_mult == 1) {
		strcat(ssexchange,serial);
		strcat (ssexchange, " ");
	}
*/
    strcat(ssexchange, section);

    // ---------------------------end mults --------------------------
    mvprintw(12, 54, comment);
    refresh();

    return (x);
}


/*  --------------------------------------------------------------------------*/

int getlastpattern(char *checkstring)
{

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

/*  --------------------------------------------------------------------------*/

char *getgrid(char *comment)
{

    char *gridmult = "";
    int multposition = 0;
    int i = 0;

    for (i = 0; i < strlen(comment); i++) {
	if (comment[i] > 64 && comment[i] < 91) {
	    multposition = i;
	    break;
	}
    }
    gridmult = comment + multposition;

    return (gridmult);
}
