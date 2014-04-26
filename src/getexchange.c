/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@eudx.org>
 *               2011-2012                Thomas Beierlein <tb@forth-ev.de>
 *               2013                     Ervin Hegedüs - HA2OS <airween@gmail.com>
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

#include "getexchange.h"
#include "recall_exchange.h"
#include "addspot.h"
#include "logit.h"
#include "cw_utils.h"
#include <glib.h>
#include "qtcsend.h"
#include "qtcrecv.h"

#define MULTS_POSSIBLE(n) ((char *)g_ptr_array_index(mults_possible, n))
#define LEN(array) (sizeof(array) / sizeof(array[0]))


int play_file(char *audiofile);


int checkexchange (int x);
int getlastpattern (char *checkstring);
char *getgrid (char *comment);
void exchange_edit (void);

int getexchange(void)
{
    extern int contest;
    extern char comment[];
    extern char cqzone[];
    extern char ituzone[];
    extern char my_rst[];
    extern int change_rst;
    extern char message[][80];
    extern char ph_message[14][80];
    extern char hiscall[];
    extern char buffer[];
    extern char qsonrstr[];
    extern int cqww;
    extern int wpx;
    extern int pacc_pa_flg;
    extern int waedc_flg;
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
    extern char mycontinent[];
    extern int keyerport;
    extern int commentfield;
    extern int no_rst;
    extern char **qsos;
    extern int nr_qsos;
    extern int qtcdirection;
   
    int i;
    int x = 0;
    char instring[2];
    char commentbuf[40] = "";
    int retval = 0;
    char *gridmult = "";

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

    /* parse input and modify exchange field accordingly */

    commentfield = 1;

    i = strlen(comment);

    while (1) {

	refresh_comment();
	
       	/* wait for next char pressed, but update time, cluster and TRX qrg */
	nodelay(stdscr, TRUE);  /* main loop waiting for input */
	x = -1;
	while (x < 1) {

	    usleep(10000);

	    time_update();

	    if (trxmode == DIGIMODE && (keyerport == GMFSK
	           || keyerport == MFJ1278_KEYER)) {
	        show_rtty();
	    }

            /* make sure that the wrefresh() inside getch() shows the cursor
             * in the input field */
	    wmove(stdscr, 12, 54 + strlen(comment));
	    x = onechar();
        }
        nodelay(stdscr, FALSE);

	switch (x) {

	case 17:	// CTRL-Q
	    {
		if (qtcdirection & 1) {
		    qtc_recv_panel();
		}
		x=155;
		continue;
	    }
	case 19:	// CTRL+s
	    {
		qtc_send_panel();
		x=155;
		continue;
	    }
	/* case 195:	// ALT
	    {
		x = onechar();
		switch(x) {
		    case 178:	// ALT+r
			      qtc_recv_panel();
			      x = 155;
			      continue;
		    case 179:  // ALT+s
			      qtc_send_panel();
			      x = 155;
			      continue;
		}
	    }
	    break; */
	case 1:						/* ctrl-a */
	    {
		addspot();
		*comment = '\0';
		x = 9;
		break;
	    }

	case 127:					/* erase */
	    {
		if (i >= 1) {
		    comment[strlen(comment) - 1] = '\0';
		    i -= 1;
		}
		break;
	    }

	case 27:
	    {
		stoptx();			/* stop sending CW */
		if (comment[0] != '\0') {	/* if comment not empty */
		    /* drop exchange so far */
		    comment[0] = '\0';
		    i = 0;
		} else {
		    /* back to callinput */
		    x = 9;
		}
		break;
	    }

	case 160:		// for CT compatibility
	    {
		if (ctcomp != 0) {
		    if (trxmode == CWMODE || trxmode == DIGIMODE) {
			sendmessage(message[1]);

		    } else
			play_file(ph_message[1]);

		}
		break;
	    }

	case '+':		// for CT compatibility
	    {
		if ((ctcomp != 0) && (strlen(hiscall) > 2)) {
		    if (trxmode == CWMODE || trxmode == DIGIMODE) {
			sendmessage(message[2]);	/* F3 */

		    } else
			play_file(ph_message[2]);

		    x = 92;
		}
		break;
	    }

	case 129:
	    {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    strcat(buffer, call);	/* F1 */
		    sendbuf();
		} else
		    play_file(ph_message[5]);	// call

		break;
	    }

	case 130 ... 138:
	    {
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    sendmessage(message[x - 129]);	/* F2..F10 */
		} else
		    play_file(ph_message[x - 129]);

		break;
	    }
	case 140:
            {
                if (trxmode == CWMODE || trxmode == DIGIMODE) {
                    sendmessage(message[10]);        /* F11 */

                } else
                    play_file(ph_message[10]);

                break;
            }
	case 176 ... 186:
	    {
		sendmessage(message[x - 162]);	/* alt-0 to alt-9 */

		break;
	    }

	case 155:		/* edit exchange field */
	    {
		if (*comment != '\0')
		    exchange_edit();
		break;
	    }

	case 156:		/* change MY RST */
	    {
		if (change_rst == 1) {
		    if (my_rst[1] <= 56) {
			my_rst[1]++;

			no_rst ? : mvprintw(12, 49, my_rst);
		    }
		} else {	/* speed up */
		    speedup();

                    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2d", GetCWSpeed());
		}
		break;

	    }
	case 157:
	    {
		if (change_rst == 1) {

		    if (my_rst[1] > 49) {
			my_rst[1]--;

			no_rst ? : mvprintw(12, 49, my_rst);
		    }
		} else {
		    speeddown();

                    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2d", GetCWSpeed());
		}
		break;

	    }
	case 44:		// , keyer
	case 11:		// ctrl-k
	    {
		mvprintw(5, 0, "");
		keyer();
		x = 0;
		break;
	    }
	case '\n':
	    {			/* log QSO immediately if CT compatible
				 * or not in contest */
		if ((ctcomp == 1) || (contest != 1))
		    x = 92;
//                            if (dxped == 1) x = 92;
		break;
	    }
	}

	if (x >= 'a' && x <= 'z')
	    x = x - 32;

	if (i < 25) {		/* normal character -> insert if space left */
	    if (x >= ' ' && x <= 'Z') {
		instring[0] = x;
		addch(x);
		strcat(comment, instring);
		if (keyerport == GMFSK) {
		    show_rtty();
		    mvprintw(12, 54, comment);
		}
		i++;
		refreshp();
	    }
	}

	if ((serial_section_mult == 1) ||
	    (dx_arrlsections == 1) ||
	    (sectn_mult == 1) ||
	    (arrlss == 1) ||
	    (cqww == 1)) {

	    x = checkexchange(x);
	}

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

	    if (
	      (waedc_flg == 1) &&
	      (
		((trxmode != DIGIMODE) && ((strcmp(mycontinent,"EU")==0 && strcmp(continent,"EU")!=0) || (strcmp(mycontinent,"EU")!=0 && strcmp(continent,"EU")==0)))
		||
		(trxmode == DIGIMODE)
	      )
	    ) {
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
		refreshp();

		// x = 0;		//##debug 17jan10 tb
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

int checkexchange(int x)
{

    extern char comment[];
    extern char ssexchange[];
    extern GPtrArray *mults_possible;
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

    /* get the pattern sequence from comment string */
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

		for (jj = 0; jj < mults_possible->len; jj++) {

		    char *multi = g_strdup(MULTS_POSSIBLE(jj));
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
		    strncpy(checksection, comment + (hr), 3);
		    if (checksection[strlen(checksection) - 1] == ' ') {
			checksection[strlen(checksection) - 1] = '\0';
		    }

		    for (jj = 0; jj < mults_possible->len; jj++) {

			if ((strlen(MULTS_POSSIBLE(jj)) >= 1)
			    && (strcmp(checksection, MULTS_POSSIBLE(jj)) ==
				0)) {
			    strcpy(section, MULTS_POSSIBLE(jj));
			    break;	// new
			}
		    }
		}
	    }
	}			// end serial_section_mult
	if (sectn_mult == 1) {

	    for (ii = 0; ii < LEN(sectionpats); ii++) {

		hr = getlastpattern(sectionpats[ii]);

		strncpy(checksection, comment, 3);
		checksection[3] = '\0';
		for (jj = 0; jj < mults_possible->len; jj++) {

		    if ((strlen(MULTS_POSSIBLE(jj)) >= 1)
			&& (strstr(checksection, MULTS_POSSIBLE(jj)) !=
			    NULL)) {

			strcpy(section, MULTS_POSSIBLE(jj));
		    }
		}
	    }

	}			//  end sectn_mult
	if (dx_arrlsections == 1) {

	    for (ii = 0; ii < LEN(sectionpats); ii++) {

		hr = getlastpattern(sectionpats[ii]);

		strncpy(checksection, comment, 3);
		checksection[3] = '\0';

		for (jj = 0; jj < mults_possible->len; jj++) {

		    if ((strlen(MULTS_POSSIBLE(jj)) ==
			 strlen(checksection))
			&& (strstr(checksection, MULTS_POSSIBLE(jj)) !=
			    NULL)) {

			strcpy(section, MULTS_POSSIBLE(jj));

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

/* ------------------------------------------------------------------------
 * return a pointer to the start of grid locator
 */

char *getgrid(char *comment)
{

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

void exchange_edit (void)
{
    extern char comment[];

    int l, b;
    int i = 0, j;
    char comment2[27];

    l = strlen(comment);
    b = l - 1;

    while ((i != 27) && (b <= strlen(comment))) {
	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvprintw(12, 54, "                          ");
	mvprintw(12, 54, comment);
	mvprintw(12, 54 + b, "");

	i = onechar();

	if (i == 1) {		// ctrl-A, Home

	    b = 0;

	} else if (i == 5) {	// ctrl-E, End

	    b = strlen(comment) - 1;

	} else if (i == 155) {	// left

	    if (b > 0)
		b--;

	} else if (i == 154) {	// right

	    if (b < strlen(comment) - 1) {
		b++;
	    } else
		break;		/* stop edit */

	} else if (i == 161) {	/* delete */

	    l = strlen(comment);

	    for (j = b; j <= l; j++) {
		comment[j] = comment[j + 1];	/* move to left incl.\0 */
	    }

	} else if (i == 127) {	/* backspace */

	    if (b > 0) {
		b--;

		l = strlen(comment);

		for (j = b; j <= l; j++) {
		    comment[j] = comment[j + 1];
		}
	    }
	} else if (i != 27) {

	    if ((i >= 'a') && (i <= 'z'))
		i = i - 32;

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
		i = 27;
	}
    }

    attron(A_STANDOUT);
    refresh_comment();
}
