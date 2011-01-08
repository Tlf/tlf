/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
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
	 *        Search log for calls / bands  /  countries
	 *
	 *--------------------------------------------------------------*/

#include "searchlog.h"
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

static char searchresult[MAX_CALLS][82];
static char result[MAX_CALLS][82];
 /* DJ1YFK "worked window"-patch */
static char band_yfk[5] = "";
static char testcall_yfk[14] = "";
static char hiscall_yfk[14] = "            ";
int xx = 0;
int bm[6];
int bandnr;
int yy = 0;
 /* */

int searchlog(char *searchstring)
{
    extern int use_rxvt;
    extern int isdupe;		// LZ3NY auto-b4 patch
    extern char hiscall[];
    extern int searchflg;
    extern int dupe;
    extern char band[9][4];
    extern int bandinx;
    extern int partials;
    extern int cqww;
    extern int pacc_pa_flg;
    extern int pacc_qsos[10][10];
    extern char datalines[MAX_DATALINES][81];
    extern int countrynr;
    extern int contest;
    extern int wpx;
    extern int arrlss;
    extern int zones[41];
    extern char pxstr[];
    extern int w_cty;
    extern int ve_cty;
    extern int ja_cty;
    extern int zl_cty;
    extern int vk_cty;
    extern int ua9_cty;
    extern int lu_cty;
    extern int py_cty;
    extern int ce_cty;
    extern int countries[MAX_DATALINES];
    extern int use_part;
    extern int block_part;
    extern int mixedmode;
    extern int ignoredupe;
    extern int qso_once;
    extern int trxmode;
    extern long int max_callmastercalls;
    extern char callmasterarray[MAX_CALLMASTER][14];
    extern char qsos[MAX_QSOS][82];
    extern char hiscall[];
    extern char zone_export[];
    extern char zone_fix[];
    extern int show_time;
    extern int wazmult;
    extern int itumult;

    int srch_index = 0;
    int r_index = 0;
    int o = 0;
    char s_inputbuffer[82] = "";
    char s_inputbuffercpy[82] = "";
    char printres[14] = "";
    char *loc;
    static char zonebuffer[3] = "";
    static int z, z1;
    static int i, j, k, l;
    static long int m;
    static int pxnr;
    static int qso_index = 0;
    static int xwin = 1;
    static int ywin = 1;

    char *tmpstr;

    l = 0;
    z = 0;
    s_inputbuffer[0] = '\0';
    zonebuffer[0] = '\0';

    /* show checkwindow and partials */
    if (strlen(hiscall) > 1 && searchflg == SEARCHWINDOW) {

	if (strlen(hiscall) == 2)
	    z1 = 0;

	qso_index = 0;

	srch_index = 0;

	r_index = 0;

	/* durchsuche komplettes Log nach 'hiscall' als substring und
	 * kopiere gefundene QSO's nach 'searchresults' */
	while (strlen(qsos[qso_index]) > 4) {

	    if (((qsos[qso_index][3] == 'C' && trxmode == CWMODE) ||
		 (qsos[qso_index][3] == 'S' && trxmode == SSBMODE) ||
		 (qsos[qso_index][3] == 'D' && trxmode == DIGIMODE)) ||
		mixedmode == 0) {
		// ist letzterTest korrekt?

		strncpy(s_inputbuffer, qsos[qso_index], 81);

		if (strstr(s_inputbuffer, hiscall) != 0) {
		    strcpy(searchresult[srch_index], s_inputbuffer);
		    searchresult[srch_index][80] = '\0';

		    if (srch_index++ > MAX_CALLS - 1)
			break;

		}
	    }

	    qso_index++;
	}

	s_inputbuffer[0] = '\0';

	// initialize array best matching callsigns
	for (xx = 0; xx < 6; xx++) {
	    bm[xx] = 0;
	}

	for (r_index = 0; r_index < srch_index; r_index++) {

	    strncpy(result[r_index], searchresult[r_index], 7);
	    result[r_index][6] = '\0';

	    if (show_time == 1)	// show qso time
		strncat(result[r_index], searchresult[r_index] + 17, 5);
	    else		// show qso number
		strncat(result[r_index], searchresult[r_index] + 22, 5);

	    strncat(result[r_index], searchresult[r_index] + 28, 12);
	    strncat(result[r_index], searchresult[r_index] + 52, 16);
	}

	/* DJ1YFK worked-window patch */
	strncpy(band_yfk, searchresult[r_index], 3);
	band_yfk[4] = '\0';
	bandnr = atoi(band_yfk);

	strncpy(testcall_yfk, searchresult[r_index] + 29, 12);
	testcall_yfk[13] = '\0';

	// Create string with his call + whitespaces
	strcpy(hiscall_yfk, "            ");
	for (xx = 0; xx < strlen(hiscall); xx++) {
	    hiscall_yfk[xx] = hiscall[xx];
	}
	// find out how many characters match
	yy = 0;
	for (xx = 0; xx < 13; xx++) {
	    if (hiscall_yfk[xx] == testcall_yfk[xx]) {
		yy++;
	    }
	}

	// delete QSOs that match worse than anything before
	// of course still less-good matching QSOs can be in the array,
	// but *before* the better matching one, so they will be
	// overwritten later.
	switch (bandnr) {
	case 160:{
		if (yy < bm[0]) {
		    result[r_index][0] = '\0';
		} else {
		    bm[0] = yy;
		}
		break;
	    }
	case 80:{
		if (yy < bm[1]) {
		    result[r_index][0] = '\0';
		} else {
		    bm[1] = yy;
		}
		break;
	    }
	case 40:{
		if (yy < bm[2]) {
		    result[r_index][0] = '\0';
		} else {
		    bm[2] = yy;
		}
		break;
	    }
	case 20:{
		if (yy < bm[3]) {
		    result[r_index][0] = '\0';
		} else {
		    bm[3] = yy;
		}
		break;
	    }
	case 15:{
		if (yy < bm[4]) {
		    result[r_index][0] = '\0';
		} else {
		    bm[4] = yy;
		}
		break;
	    }
	case 10:{
		if (yy < bm[5]) {
		    result[r_index][0] = '\0';
		} else {
		    bm[5] = yy;
		}
	    }
	}			/* end of patch */

	dupe = NODUPE;
	nicebox(1, 41, 6, 37, "Worked");
	refresh();

	attron(COLOR_PAIR(7) | A_STANDOUT);

	for (j = 2; j <= 7; j++) {
	    mvprintw(j, 42, "                                     ");
	}
	mvprintw(2, 42, " 10");
	mvprintw(3, 42, " 15");
	mvprintw(4, 42, " 20");
	mvprintw(5, 42, " 40");
	mvprintw(6, 42, " 80");
	mvprintw(7, 42, "160");

	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	s_inputbuffer[0] = '\0';

	k = 0;

	/* print resulting call in line according to band in scheck window */
	for (r_index = 0; r_index < srch_index; r_index++) {
	    strcpy(s_inputbuffer, result[r_index]);
	    s_inputbuffer[37] = '\0';

	    if ((hiscall[0] == s_inputbuffer[12]) &&
		(strlen(hiscall) >= 3 &&
		 (s_inputbuffer[12 + strlen(hiscall)] == ' '))) {
		if ((strncmp(band[bandinx], s_inputbuffer, 3) == 0)
		    || (qso_once == 1)) {
		    if (ignoredupe == 0) {

			if (mixedmode == 0) {
			    attron(COLOR_PAIR(DUPECOLOR) | A_STANDOUT);
			    dupe = ISDUPE;
			    beep();
			} else {
			    if (((s_inputbuffer[3] == 'C') &&
				 (trxmode == CWMODE)) ||
				((s_inputbuffer[3] == 'S')
				 && (trxmode == SSBMODE))) {
				attron(COLOR_PAIR(DUPECOLOR) | A_STANDOUT);
				dupe = ISDUPE;
				beep();
			    }

			}	// end mixed
		    }		// end ignore
		}
	    }
	    if (s_inputbuffer[1] == '1')
		j = 2;
	    if (s_inputbuffer[1] == '1' && s_inputbuffer[2] == '5')
		j = 3;
	    if (s_inputbuffer[1] == '2')
		j = 4;
	    if (s_inputbuffer[1] == '4')
		j = 5;
	    if (s_inputbuffer[1] == '8')
		j = 6;
	    if (s_inputbuffer[1] == '6')
		j = 7;

	    if (j != 8) {
		mvprintw(j, 42, "%s", s_inputbuffer);
	    }

	    if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
		z = 0;
		if (strlen(s_inputbuffer) >= 24) {
		    strncpy(zonebuffer, s_inputbuffer + 25, 2);
		    zonebuffer[2] = '\0';
		    z1 = zone_nr(zonebuffer);
		} else
		    z = zone_nr(zone_export);

		if (z1 != 0)
		    z = z1;
	    }

	    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	    if ((partials == 1) && (strlen(hiscall) >= 2)) {
		if (strlen(s_inputbuffer) != 0)
		    strncpy(s_inputbuffercpy, s_inputbuffer + 12, 6);
		if (s_inputbuffercpy[5] == ' ')
		    s_inputbuffercpy[5] = '\0';
		if (s_inputbuffercpy[4] == ' ')
		    s_inputbuffercpy[4] = '\0';
	    }

	    s_inputbuffer[0] = '\0';
	}

	/* prepare and print lower line of checkwindow */
	attroff(A_STANDOUT);
	strncpy(s_inputbuffer, datalines[countrynr], 28);

	for (i = 0; i <= 27; i++) {

	    if (s_inputbuffer[i] == ':') {
		s_inputbuffer[i] = ' ';
		break;
	    }
	}

	if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {

	    if (z == 0) {
		if (strlen(zone_fix) > 1) {
		    strncpy(zonebuffer, zone_fix, 2);
		} else
		    strncpy(zonebuffer, zone_export, 2);

		zonebuffer[2] = '\0';
		z = zone_nr(zonebuffer);
	    }
	}

	s_inputbuffer[28] = '\0';

	attron(COLOR_PAIR(COLOR_YELLOW));

	mvprintw(8, 47, s_inputbuffer);
	mvhline(8, 47 + i, ACS_HLINE, 26 - i);
	if (itumult != 1)
	    mvprintw(8, 73, "%s", zonebuffer);
	else
	    mvprintw(8, 69, "ITU:%s", zonebuffer);

	s_inputbuffer[0] = '\0';

	if (wpx == 1) {
	    mvprintw(8, 47 + i + 5, pxstr);
	}

	/* print worked zones and countrays for each band in checkwindow */
	attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);

	if (cqww == 1 || contest == 0 || pacc_pa_flg == 1) {
	    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);

	    if ((countries[countrynr] & BAND10) != 0) {
		mvprintw(2, 77, "C");
		mvprintw(2, 42, " 10");
	    }
	    if ((countries[countrynr] & BAND15) != 0) {
		mvprintw(3, 77, "C");
		mvprintw(3, 42, " 15");
	    }
	    if ((countries[countrynr] & BAND20) != 0) {
		mvprintw(4, 77, "C");
		mvprintw(4, 42, " 20");
	    }
	    if ((countries[countrynr] & BAND40) != 0) {
		mvprintw(5, 77, "C");
		mvprintw(5, 42, " 40");
	    }
	    if ((countries[countrynr] & BAND80) != 0) {
		mvprintw(6, 77, "C");
		mvprintw(6, 42, " 80");
	    }
	    if ((countries[countrynr] & BAND160) != 0) {
		mvprintw(7, 42, "160");
		mvprintw(7, 77, "C");
	    }
	}
	if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
	    if ((zones[z] & BAND10) != 0) {
		mvprintw(2, 78, "Z");
	    }
	    if ((zones[z] & BAND15) != 0) {
		mvprintw(3, 78, "Z");
	    }
	    if ((zones[z] & BAND20) != 0) {
		mvprintw(4, 78, "Z");
	    }
	    if ((zones[z] & BAND40) != 0) {
		mvprintw(5, 78, "Z");
	    }
	    if ((zones[z] & BAND80) != 0) {
		mvprintw(6, 78, "Z");
	    }
	    if ((zones[z] & BAND160) != 0) {
		mvprintw(7, 78, "Z");
	    }
	}

	if (pacc_pa_flg == 1) {

	    getpx(hiscall);

	    pxnr = pxstr[strlen(pxstr) - 1] - 48;

	    if (countrynr == w_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == ve_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == ja_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == py_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == lu_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == ua9_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == zl_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == lu_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == ce_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }
	    if (countrynr == vk_cty) {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvprintw(7, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvprintw(6, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvprintw(5, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvprintw(4, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvprintw(3, 78, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvprintw(2, 78, "M");
	    }

	}


	/* print list of partials in upper left region */
	if (partials == 1)
	{

	    l = 0;
	    j = 0;

	    if (use_rxvt == 0)
		attron(COLOR_PAIR(COLOR_WHITE | A_BOLD | A_STANDOUT));
	    else
		attron(COLOR_PAIR(COLOR_WHITE | A_STANDOUT));

	    for (k = 1; k <= 5; k++) {
		mvprintw(k, 0, "%s",
			 "                                        ");
	    }
	    attron(COLOR_PAIR(COLOR_MAGENTA) | A_STANDOUT);
	    mvprintw(1, 1, "??");
	    attron(COLOR_PAIR(COLOR_WHITE | A_STANDOUT));

	    refresh();

	    j = 0;
	    m = 0;
	    
	    /* check what we have worked first */
	    /** \todo the method below parses through the array of already 
	     * looked up search results from the search window. That is quick
	     * but has the drawback, that we have no band information and
	     * therefore print some entries more than once.
	     * Better would be to lookup the aprtial call in the arrayy of 
	     * worked stations 'callarray' - it is there only once and we can 
	     * also see from 'call_band' if it is a dupe here.
	     * be aware of the problem of marking it dupe only for a complete 
	     * match.
	     */
	    for (m = 0; m < srch_index; m++) {
		if (strlen(hiscall) > 2 && strlen(searchresult[m]) > 2) {
		    if ( strstr(searchresult[m], hiscall) != NULL ) {
			printres[0] = '\0';
			strncat(printres, searchresult[m] + 29, 12);

			loc = strchr(printres, ' ');

			int length = (int) (loc - printres);

			strncpy(printres, printres, length);

			printres[length] = '\0';

			if (dupe == ISDUPE) {
			    attron(COLOR_PAIR(COLOR_MAGENTA) | A_STANDOUT);
			} else {
			    attron(COLOR_PAIR
				   (COLOR_YELLOW | A_BOLD | A_STANDOUT));
			}
			mvprintw(xwin + l, ywin + j, "%s ", printres);
			attron(COLOR_PAIR(COLOR_WHITE | A_STANDOUT));

			refresh();

			j += (strlen(printres) + 1);

			if (j >= 30) {
			    l++;
			    j = 0;

			}
			if (l > 4)
			    break;
		    }
		}
	    }

	    o = m;
	    if (strcmp(hiscall, printres) != 0) {

		/* and now check callmaster database */
		for (m = 0; m < max_callmastercalls; m++)
		{

		    if ( strstr(callmasterarray[m], hiscall) != NULL ) {

			if (use_rxvt == 0)
			    attron(COLOR_PAIR
				   (COLOR_WHITE | A_BOLD |
				    A_STANDOUT));
			else
			    attron(COLOR_PAIR
				   (COLOR_WHITE | A_STANDOUT));

			mvprintw(xwin + l, ywin + j, "%s  ",
				 callmasterarray[m]);

			if (strlen(s_inputbuffercpy) == 0)
			    strcpy(s_inputbuffercpy, callmasterarray[m]);

			j += (strlen(callmasterarray[m])) + 1;

			if (j >= 30) {
			    l++;
			    j = 0;

			}
			if (l > 4)
			    break;

		    }
		}
	    }

	    if ((j <= 13) && (l == 0) && (use_part == 1)
		&& (block_part == 0)) {
		if (use_rxvt == 0)
		    attron(COLOR_PAIR(COLOR_GREEN | A_BOLD | A_STANDOUT));
		else
		    attron(COLOR_PAIR(COLOR_GREEN | A_STANDOUT));

		mvprintw(13, 0, s_inputbuffercpy);
	    }

	    if ((block_part == 0) && (use_part == 1) && (j <= 13)
		&& (l == 0)
		&& (strlen(s_inputbuffercpy) > strlen(hiscall))) {

		strcpy(hiscall, s_inputbuffercpy);
		beep();

	    }
	    refresh();

	}

	/* show multiplierinfo */
	if (dupe == NODUPE && arrlss == 1)
	    r_multiplierinfo();

	if (dupe == ISDUPE) {
	    isdupe = 1;		// LZ3NY auto-b4 patch
	    attron(COLOR_PAIR(DUPECOLOR) | A_STANDOUT);
	    mvprintw(12, 29, hiscall);
	    refresh();
	    usleep(100000);
	} else
	    isdupe = 0;		// LZ3NY auto-b4 patch
	    printcall();

    }

    return (0);
}

// --------------------------------------------load callmaster ------------------
/** loads callmaster database from file
 * */
int load_callmaster(void)
{
    extern char callmasterarray[MAX_CALLMASTER][14];
    extern int arrlss;

    FILE *cfp = NULL;
    char callmaster_location[80];
    char s_inputbuffer[186] = "";
    long int count = 0;
    int file_ok = 0;

    strcpy(callmaster_location, "callmaster");
    if ((cfp = fopen(callmaster_location, "r")) == NULL) {
	callmaster_location[0] = '\0';
	strcpy(callmaster_location, PACKAGE_DATA_DIR);
	strcat(callmaster_location, "/callmaster");

	if ((cfp = fopen(callmaster_location, "r")) == NULL) {
	    mvprintw(24, 0, "Error opening callmaster file.\n");
	    refresh();
	    sleep(2);
	} else
	    file_ok = 1;
    } else
	file_ok = 1;

    if (file_ok == 1) {

	count = 0;

	while ( fgets(s_inputbuffer, 85, cfp) != NULL ) {

	    if ( strlen(s_inputbuffer) < 3 )
		/* calls are at least 3 char long */
		continue;

	    if (arrlss == 1) {

		if ((s_inputbuffer[0] == 'A') || (s_inputbuffer[0] == 'K')
		    || (s_inputbuffer[0] == 'W')
		    || (s_inputbuffer[0] == 'V')
		    || (s_inputbuffer[0] == 'C')
		    || (s_inputbuffer[0] == 'N')) {
		    s_inputbuffer[strlen(s_inputbuffer) - 1] = '\0';

		    s_inputbuffer[12] = '\0';
		    strcpy(callmasterarray[count], s_inputbuffer);
		    count++;
		}

	    } else {

		if (strlen(s_inputbuffer) > 0)
		    s_inputbuffer[strlen(s_inputbuffer) - 1] = '\0';

		s_inputbuffer[12] = '\0';
		strcpy(callmasterarray[count], s_inputbuffer);
		count++;

	    }

	}

	fclose(cfp);
    } else {
	if (cfp)
	    fclose(cfp);
    }

    return (count);
}

// -------------------------------------------------------------------------------
//char multsfile[80] = "";              // global... (to be fixed)
// ----------------------------------------------load mults ------------------------

/** loads possible multipliers from external file
 * */
int load_multipliers(void)
{
    extern char mults_possible[MAX_MULTS][12];
    extern char multiplierlist[];
    extern char multsfile[];

    FILE *cfp;
    char s_inputbuffer[186] = "";
    int count = 0, i;

    for (i = 0; i < MAX_MULTS; i++) {
	mults_possible[i][0] = '\0';
    }

    if (strlen(multiplierlist) != 0)
	strncpy(multsfile, multiplierlist, strlen(multiplierlist) - 1);
    else {
	mvprintw(9, 0, "No multiplier file specified, exiting.. !!\n");
	refresh();
	sleep(5);
	exit(1);
    }

    if ((cfp = fopen(multsfile, "r")) == NULL) {
	mvprintw(9, 0, "Error opening multiplier file %s.\n", multsfile);
	refresh();
	sleep(2);
    } else {

	count = 0;

	while ( fgets(s_inputbuffer, 85, cfp) != NULL ) {

	    if (strlen(s_inputbuffer) > 0)
		s_inputbuffer[strlen(s_inputbuffer) - 1] = '\0';
	    s_inputbuffer[9] = '\0';
	    /* BUG todo: fix strcpy. poss. buffer overun 16jan10 tb */
	    strcpy(mults_possible[count], s_inputbuffer);

	    count++;

	}

	fclose(cfp);
    }

    return (count);
}
