/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
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
	 *        Search log for calls / bands  /  countries
	 *
	 *--------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dxcc.h"
#include "getctydata.h"
#include "getpx.h"
#include "nicebox.h"		// Includes curses.h
#include "printcall.h"
#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "searchlog.h"		// Includes glib.h
#include "tlf_panel.h"
#include "ui_utils.h"
#include "zone_nr.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


GPtrArray *callmaster = NULL;

PANEL *search_panel;
WINDOW *search_win;
static int initialized = 0;
int nr_bands;

void show_needed_sections(void);

/** Check for all band mode
 *
 * Check if we work real contest (only nonWARC bands allowed)
 * or our mode allows also WARC bands (e.g. dxped or simple QSO mode)
 * \return - true if also WARC bands
 */
int IsAllBand()
{
    extern int dxped;
    extern int contest;

    return ((dxped != 0) || (contest == 0));
}


void InitSearchPanel()
{
    if (IsAllBand())
	nr_bands = 9;
    else
	nr_bands = 6;

    search_win = newwin( nr_bands + 2, 39, 1, 41 );
    search_panel = new_panel( search_win );
    hide_panel( search_panel );
}

void ShowSearchPanel(void)
{
    if (!initialized) {
	InitSearchPanel();
	initialized = 1;
    }
    show_panel( search_panel );
    top_panel( search_panel );
}

void HideSearchPanel(void)
{
    hide_panel(search_panel);
}

static char searchresult[MAX_CALLS][82];
static char result[MAX_CALLS][82];
 /* DJ1YFK "worked window"-patch */
static char band_yfk[5] = "";
static char testcall_yfk[14] = "";
static char hiscall_yfk[14] = "            ";
 /* */

void searchlog(char *searchstring)
{
    extern int isdupe;		// LZ3NY auto-b4 patch
    extern int searchflg;
    extern int dupe;
    extern char band[9][4];
    extern int bandinx;
    extern int partials;
    extern int cqww;
    extern int pacc_pa_flg;
    extern int pacc_qsos[10][10];
    extern t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];
    extern int pfxnummultinr;
    extern int countrynr;
    extern int contest;
    extern int wpx;
    extern int arrlss;
    extern int zones[MAX_ZONES];
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
    extern int zs_cty;
    extern int countries[MAX_DATALINES];
    extern int use_part;
    extern int block_part;
    extern int mixedmode;
    extern int ignoredupe;
    extern int qso_once;
    extern int trxmode;
    extern char qsos[MAX_QSOS][LOGLINELEN+1];
    extern char hiscall[];
    extern char zone_export[];
    extern char zone_fix[];
    extern int show_time;
    extern int wazmult;
    extern int itumult;
    extern int country_mult;

    int srch_index = 0;
    int r_index = 0;
    int xx;
    int yy;
    int bandnr;
    int bm[6];
    char s_inputbuffer[LOGLINELEN+1] = "";
    char s_inputbuffercpy[LOGLINELEN+1] = "";
    char printres[14] = "";
    char *loc;
    dxcc_data *dx;
    static char zonebuffer[3] = "";
    static int z, z1;
    static int i, j, k, l;
    static long int m;
    static int pxnr;
    static int qso_index = 0;
    static int xwin = 1;
    static int ywin = 1;
    char qtccall[15];	// temp str for qtc search
    char qtcflags[6] = {' ', ' ', ' ', ' ', ' ', ' '};
    int pfxnumcntidx;

    struct t_qtc_store_obj *qtc_temp_ptr;

    if (!initialized) {
	InitSearchPanel();
	initialized = 1;
    }

    l = 0;
    z = 0;
    s_inputbuffer[0] = '\0';
    zonebuffer[0] = '\0';

    /* show checkwindow and partials */
    if (strlen(hiscall) > 1 && searchflg == SEARCHWINDOW) {

	ShowSearchPanel();

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

		strncpy(s_inputbuffer, qsos[qso_index], LOGLINELEN);

		if (strstr(s_inputbuffer, hiscall) != 0) {
		    strcpy(searchresult[srch_index], s_inputbuffer);
		    searchresult[srch_index][80] = '\0';

		    if (srch_index++ > MAX_CALLS - 1)
			break;

		}
	    }

	    qso_index++;
	}

	// initialize array best matching callsigns
	for (xx = 0; xx < 6; xx++) {
	    bm[xx] = 0;
	}

	for (r_index = 0; r_index < srch_index; r_index++) {

	    strncpy(result[r_index], searchresult[r_index], 7);	/* band + mode */
	    result[r_index][6] = '\0';

	    if (show_time == 1)	// show qso time
		strncat(result[r_index], searchresult[r_index] + 17, 5);
	    else		// show qso number
		strncat(result[r_index], searchresult[r_index] + 22, 5);

	    strncat(result[r_index], searchresult[r_index] + 28, 12);	/* call */
	    strncat(result[r_index], searchresult[r_index] + 52, 16);	/* exch */
	}

	/* DJ1YFK worked-window patch */
	strncpy(band_yfk, searchresult[r_index], 3);
	band_yfk[3] = '\0';
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

	wbkgd( search_win, (chtype)(' ' | COLOR_PAIR(C_LOG)) );
	werase( search_win );

	wnicebox(search_win, 0, 0, nr_bands, 37, "Worked");
	if (qtcdirection > 0) {
	    mvwprintw(search_win, 0, 35, "Q");
	}

	wattrset(search_win, COLOR_PAIR(C_LOG) | A_STANDOUT );
	for (i = 0; i < nr_bands; i++)
	    mvwprintw(search_win, i + 1, 1,
		    "                                     ");

	mvwprintw(search_win, 1, 1, " 10");
	mvwprintw(search_win, 2, 1, " 15");
	mvwprintw(search_win, 3, 1, " 20");
	mvwprintw(search_win, 4, 1, " 40");
	mvwprintw(search_win, 5, 1, " 80");
	mvwprintw(search_win, 6, 1, "160");
	if (IsAllBand()) {
	    mvwprintw(search_win, 7, 1, " 12");
	    mvwprintw(search_win, 8, 1, " 17");
	    mvwprintw(search_win, 9, 1, " 30");
	}

	refreshp();

	wattrset(search_win, COLOR_PAIR(C_WINDOW) | A_STANDOUT);

	k = 0;

	/* print resulting call in line according to band in check window */
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
			    wattrset(search_win,
				    COLOR_PAIR(C_DUPE));
			    dupe = ISDUPE;
			    beep();
			} else {
			    if (((s_inputbuffer[3] == 'C') &&
				 (trxmode == CWMODE)) ||
				((s_inputbuffer[3] == 'S')
				 && (trxmode == SSBMODE))) {
				wattrset(search_win,
					COLOR_PAIR(C_DUPE));
				dupe = ISDUPE;
				beep();
			    }

			}	// end mixed
		    }		// end ignore
		}
	    }
	    if (s_inputbuffer[1] == '1' && s_inputbuffer[2] == '0')
		j = 1;
	    if (s_inputbuffer[1] == '1' && s_inputbuffer[2] == '5')
		j = 2;
	    if (s_inputbuffer[1] == '2')
		j = 3;
	    if (s_inputbuffer[1] == '4')
		j = 4;
	    if (s_inputbuffer[1] == '8')
		j = 5;
	    if (s_inputbuffer[1] == '6')
		j = 6;
	    if (s_inputbuffer[1] == '1' && s_inputbuffer[2] == '2')
		j = 7;
	    if (s_inputbuffer[1] == '1' && s_inputbuffer[2] == '7')
		j = 8;
	    if (s_inputbuffer[1] == '3' && s_inputbuffer[2] == '0')
		j = 9;

	    if ((j > 0) && (j < 10)) {
		if (qtcdirection > 0) {
		    qtccall[0] = '\0';
		    z = 12;	// first pos of callsign
		    l = 0;
		    do {
			qtccall[l] = s_inputbuffer[z];
			z++; l++;
		    } while(s_inputbuffer[z] != ' ');
		    qtccall[l] = '\0';

		    qtc_temp_ptr = qtc_get(qtccall);
		    qtcflags[j-1] = qtc_get_value(qtc_temp_ptr);
		}
		if ((j < 7) || IsAllBand()) {
		    mvwprintw(search_win, j, 1, "%s", s_inputbuffer);
		}
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

	    wattron(search_win, COLOR_PAIR(C_WINDOW) | A_STANDOUT);

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
	wattroff(search_win, A_STANDOUT);
	dx = dxcc_by_index(countrynr);

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

	wattron(search_win, COLOR_PAIR(C_BORDER));

	mvwprintw(search_win, nr_bands + 1, 2, dx->countryname);
	mvwprintw(search_win, nr_bands + 1, 32, "%02d", dx->cq);
	i = strlen(dx->countryname);

	if (itumult != 1)
	    mvwprintw(search_win, nr_bands + 1, 32, "%s", zonebuffer);
	else
	    mvwprintw(search_win, nr_bands + 1, 28, "ITU:%s", zonebuffer);

	s_inputbuffer[0] = '\0';

	if (wpx == 1) {
	    mvwprintw(search_win, nr_bands + 1, 2 + i + 3, pxstr);
	}

	/* print worked zones and countrys for each band in checkwindow */
	wattron(search_win, COLOR_PAIR(C_HEADER) | A_STANDOUT);

	if (qtcdirection > 0) {
	    for(l=0; l<6; l++) {
		if (qtcflags[l] != ' ') {
		    mvwprintw(search_win, l+1, 35, "%c", qtcflags[l]);
		}
	    }
	}

	if (cqww == 1 || contest == 0 || pacc_pa_flg == 1) {

	    if ((countries[countrynr] & BAND10) != 0) {
		mvwprintw(search_win, 1, 36, "C");
		mvwprintw(search_win, 1, 1, " 10");
	    }
	    if ((countries[countrynr] & BAND15) != 0) {
		mvwprintw(search_win, 2, 36, "C");
		mvwprintw(search_win, 2, 1, " 15");
	    }
	    if ((countries[countrynr] & BAND20) != 0) {
		mvwprintw(search_win, 3, 36, "C");
		mvwprintw(search_win, 3, 1, " 20");
	    }
	    if ((countries[countrynr] & BAND40) != 0) {
		mvwprintw(search_win, 4, 36, "C");
		mvwprintw(search_win, 4, 1, " 40");
	    }
	    if ((countries[countrynr] & BAND80) != 0) {
		mvwprintw(search_win, 5, 36, "C");
		mvwprintw(search_win, 5, 1, " 80");
	    }
	    if ((countries[countrynr] & BAND160) != 0) {
		mvwprintw(search_win, 6, 1, "160");
		mvwprintw(search_win, 6, 36, "C");
	    }
	    if (IsAllBand()) {
		if ((countries[countrynr] & BAND12) != 0) {
		    mvwprintw(search_win, 7, 1, " 12");
		    mvwprintw(search_win, 7, 36, "C");
		}
		if ((countries[countrynr] & BAND17) != 0) {
		    mvwprintw(search_win, 8, 1, " 17");
		    mvwprintw(search_win, 8, 36, "C");
		}
		if ((countries[countrynr] & BAND30) != 0) {
		    mvwprintw(search_win, 9, 1, " 30");
		    mvwprintw(search_win, 9, 36, "C");
		}
	    }
	}
	if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
	    if ((zones[z] & BAND10) != 0) {
		mvwprintw( search_win, 1, 37, "Z");
	    }
	    if ((zones[z] & BAND15) != 0) {
		mvwprintw( search_win, 2, 37, "Z");
	    }
	    if ((zones[z] & BAND20) != 0) {
		mvwprintw( search_win, 3, 37, "Z");
	    }
	    if ((zones[z] & BAND40) != 0) {
		mvwprintw( search_win, 4, 37, "Z");
	    }
	    if ((zones[z] & BAND80) != 0) {
		mvwprintw( search_win, 5, 37, "Z");
	    }
	    if ((zones[z] & BAND160) != 0) {
		mvwprintw( search_win, 6, 37, "Z");
	    }
	}

	if (pacc_pa_flg == 1) {

	    getpx(hiscall);

	    pxnr = pxstr[strlen(pxstr) - 1] - 48;

	    if ((countrynr == w_cty) ||
		    (countrynr == ve_cty) ||
		    (countrynr == ja_cty ) ||
		    (countrynr == py_cty ) ||
		    (countrynr == lu_cty ) ||
		    (countrynr == ua9_cty ) ||
		    (countrynr == zl_cty ) ||
		    (countrynr == ce_cty ) ||
		    (countrynr == zs_cty ) ||
		    (countrynr == vk_cty ))
	    {
		if ((pacc_qsos[0][pxnr] & BAND160) == BAND160)
		    mvwprintw(search_win, 6, 37, "M");

		if ((pacc_qsos[0][pxnr] & BAND80) == BAND80)
		    mvwprintw(search_win, 5, 37, "M");

		if ((pacc_qsos[0][pxnr] & BAND40) == BAND40)
		    mvwprintw(search_win, 4, 37, "M");

		if ((pacc_qsos[0][pxnr] & BAND20) == BAND20)
		    mvwprintw(search_win, 3, 37, "M");

		if ((pacc_qsos[0][pxnr] & BAND15) == BAND15)
		    mvwprintw(search_win, 2, 37, "M");

		if ((pacc_qsos[0][pxnr] & BAND10) == BAND10)
		    mvwprintw(search_win, 1, 37, "M");

	    }
	}

	if ((pfxnummultinr >= 0 || country_mult) && contest == 1) {
	    getpx(hiscall);
	    pxnr = pxstr[strlen(pxstr) - 1] - 48;

	    getctydata(hiscall);
	    pfxnumcntidx = -1;
	    int tbandidx = -1;

	    if (pfxnummultinr >= 0) {
		int pfxi = 0;
		while(pfxi < pfxnummultinr) {
		    if (pfxnummulti[pfxi].countrynr == countrynr) {
			pfxnumcntidx = pfxi;
			break;
		    }
		    pfxi++;
		}
	    }
	    if (pfxnumcntidx >= 0) {
		tbandidx = pfxnummulti[pfxnumcntidx].qsos[pxnr];
	    }
	    else {
		tbandidx = countries[countrynr];
	    }

	    if ((tbandidx & BAND160) == BAND160) {
		mvwprintw(search_win, 6, 37, "M");
	    }
	    if ((tbandidx & BAND80) == BAND80) {
		mvwprintw(search_win, 5, 37, "M");
	    }
	    if ((tbandidx & BAND40) == BAND40) {
		mvwprintw(search_win, 4, 37, "M");
	    }
	    if ((tbandidx & BAND20) == BAND20) {
		mvwprintw(search_win, 3, 37, "M");
	    }
	    if ((tbandidx & BAND15) == BAND15) {
		mvwprintw(search_win, 2, 37, "M");
	    }
	    if ((tbandidx & BAND10) == BAND10) {
		mvwprintw(search_win, 1, 37, "M");
	    }

	}

	refreshp();


	/* print list of partials in upper left region */
	if (partials == 1)
	{
	    l = 0;
	    j = 0;

	    attron(modify_attr(COLOR_PAIR(C_LOG) | A_STANDOUT));

	    for (k = 1; k <= 5; k++) {
		mvprintw(k, 0, "%s",
			 "                                        ");
	    }
	    attrset(COLOR_PAIR(C_DUPE));
	    mvprintw(1, 1, "??");
	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	    refreshp();

	    j = 0;
	    m = 0;

	    /* check what we have worked first */
	    /** \todo the method below parses through the array of already
	     * looked up search results from the search window. That is quick
	     * but has the drawback, that we have no band information and
	     * therefore print some entries more than once.
	     * Better would be to lookup the partial call in the array of
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

			/* cut string just at first space after call */
			loc = strchr(printres, ' ');
			if (loc)
			    *loc = '\0';

			if (dupe == ISDUPE) {
			    attrset(COLOR_PAIR(C_DUPE));
			} else {
			    attron(modify_attr(COLOR_PAIR(C_BORDER) |
				A_STANDOUT));
			}
			mvprintw(xwin + l, ywin + j, "%s ", printres);
			attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

			attroff(A_BOLD);

			refreshp();

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

	    if (strcmp(hiscall, printres) != 0) {

		/* and now check callmaster database */
		for (m = 0; m < callmaster->len; m++)
		{

		    if ( strstr(CALLMASTERARRAY(m), hiscall) != NULL ) {

			attron(modify_attr(COLOR_PAIR(C_LOG) | A_STANDOUT));

			mvprintw(xwin + l, ywin + j, "%s  ",
				 CALLMASTERARRAY(m));

			if (strlen(s_inputbuffercpy) == 0)
			    strcpy(s_inputbuffercpy, CALLMASTERARRAY(m));

			j += (strlen(CALLMASTERARRAY(m))) + 1;

			if (j >= 30) {
			    l++;
			    j = 0;

			}
			if (l > 4)
			    break;

		    }
		}
	    }

	    if ((j <= 13) && (l == 0) && (use_part == 1) && (block_part == 0)) {

		attron(modify_attr(COLOR_PAIR(C_HEADER) | A_STANDOUT));

		mvprintw(13, 0, s_inputbuffercpy);
		if (strlen(s_inputbuffercpy) > strlen(hiscall)) {

		    strcpy(hiscall, s_inputbuffercpy);
		    beep();
		}
	    }

	    refreshp();
	}

	/* show needed sections for ARRL_Sweep Stake*/
	if (dupe == NODUPE && arrlss == 1)
	    show_needed_sections();

	if (dupe == ISDUPE) {
	    isdupe = 1;		// LZ3NY auto-b4 patch
	    attrset(COLOR_PAIR(C_DUPE));
	    mvprintw(12, 29, hiscall);
	    refreshp();
	    usleep(100000);
	} else
	    isdupe = 0;		// LZ3NY auto-b4 patch
	    printcall();

    }
    else {
	HideSearchPanel();
    }
}

void init_callmaster(void) {
    if (callmaster)
	g_ptr_array_free(callmaster, TRUE);
    callmaster = g_ptr_array_new_full(16000, g_free);
}

/** loads callmaster database from file
 * returns number of loaded calls
 */
int load_callmaster(void)
{
    extern int arrlss;

    FILE *cfp = NULL;
    char callmaster_location[80];
    char s_inputbuffer[186] = "";

    init_callmaster();

    strcpy(callmaster_location, "callmaster");
    if ((cfp = fopen(callmaster_location, "r")) == NULL) {
	callmaster_location[0] = '\0';
	strcpy(callmaster_location, PACKAGE_DATA_DIR);
	strcat(callmaster_location, "/callmaster");

	if ((cfp = fopen(callmaster_location, "r")) == NULL) {
	    mvprintw(24, 0, "Error opening callmaster file.\n");
	    refreshp();
	    sleep(2);

	    return 0;
	}
    }

    while ( fgets(s_inputbuffer, 85, cfp) != NULL ) {

	g_strchomp(s_inputbuffer);

	if (s_inputbuffer[0] == '#')
	    /* skip comment lines */
	    continue;

	if ( strlen(s_inputbuffer) < 3 )
	    /* calls are at least 3 char long */
	    continue;

	s_inputbuffer[12] = '\0';		/* terminate line length */

	if (arrlss == 1) {

	    /* keep only NA stations */
	    if ((s_inputbuffer[0] == 'A') || (s_inputbuffer[0] == 'K')
		|| (s_inputbuffer[0] == 'W')
		|| (s_inputbuffer[0] == 'V')
		|| (s_inputbuffer[0] == 'C')
		|| (s_inputbuffer[0] == 'N')) {

		g_ptr_array_add(callmaster, g_strdup(s_inputbuffer));
	    }
	} else {

	    g_ptr_array_add(callmaster, g_strdup(s_inputbuffer));
	}
    }

    fclose(cfp);
    return callmaster->len;
}


/*  --------------------------------------------------------------  */
void show_needed_sections(void)
{
    extern int arrlss;
    extern int multarray_nr;
    extern char mults[MAX_MULTS][12];
    extern GPtrArray *mults_possible;

    int j, vert, hor, cnt, found;
    char mprint[50];

    if (arrlss == 1) {
	cnt = 0;

	wattron(search_win, modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

	for (j = 1; j < 7; j++)
	    mvwprintw(search_win, j, 1, "                                     ");

	for (vert = 1; vert < 7; vert++) {
	    if (cnt >= mults_possible->len)
		break;

	    for (hor = 0; hor < 9; hor++) {
		if (cnt >= mults_possible->len)
		    break;

		strcpy(mprint, g_ptr_array_index(mults_possible, cnt));

		found = 0;
		for (j = 0; j < multarray_nr; j++) {
		    if (strncmp(mults[j], mprint, strlen(mprint)) == 0) {
			found = 1;
			break;
		    }
		}

		if (found != 1) {
		    mprint[3] = '\0';

		    wattron(search_win,
			modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

		    if (strlen(mprint) > 1)
			mvwprintw(search_win, vert, (hor * 4) + 2, "%s ", mprint);

		} else
		    hor--;

		cnt++;

	    }
	}
    }

    wnicebox(search_win, 0, 0, 6, 37, "Needed Sections");
    refreshp();

}

void OnLowerSearchPanel(int x, char *str)
{
    wattrset(search_win, modify_attr(COLOR_PAIR(C_BORDER)));

    mvwprintw(search_win, 7, x, str);
}
