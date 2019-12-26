/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
 *               2018           Thomas Beierlein - <dl1jbe@darc.de>
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
#include "err_utils.h"
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
#include "searchcallarray.h"
#include "get_time.h"
#include "addmult.h"


char *callmaster_filename = NULL;
GPtrArray *callmaster = NULL;

char searchresult[MAX_CALLS][82];
char result[MAX_CALLS][82];
int srch_index = 0;

char qtcflags[6] = {' ', ' ', ' ', ' ', ' ', ' '};

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
int IsAllBand() {
    extern int dxped;
    extern int contest;

    return ((dxped != 0) || (contest == 0));
}


void InitSearchPanel() {
    if (IsAllBand())
	nr_bands = 9;
    else
	nr_bands = 6;

    search_win = newwin(nr_bands + 2, 39, 1, 41);
    search_panel = new_panel(search_win);
    hide_panel(search_panel);
}

void ShowSearchPanel(void) {
    if (!initialized) {
	InitSearchPanel();
	initialized = 1;
    }
    show_panel(search_panel);
    top_panel(search_panel);
}

void HideSearchPanel(void) {
    hide_panel(search_panel);
}

void drawSearchWin(void) {
    int i;

    wbkgd(search_win, (chtype)(' ' | COLOR_PAIR(C_LOG)));
    werase(search_win);

    wnicebox(search_win, 0, 0, nr_bands, 37, "Worked");
    if (qtcdirection > 0) {
	mvwprintw(search_win, 0, 35, "Q");
    }

    wattrset(search_win, COLOR_PAIR(C_LOG) | A_STANDOUT);
    for (i = 0; i < nr_bands; i++)
	mvwprintw(search_win, i + 1, 1, spaces(37));

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
}

void displayCallInfo(dxcc_data *dx, int z, char *pxstr) {
    int i;

    wattroff(search_win, A_STANDOUT);
    wattron(search_win, COLOR_PAIR(C_BORDER));

    mvwprintw(search_win, nr_bands + 1, 2, dx->countryname);
    mvwprintw(search_win, nr_bands + 1, 32, "%02d", dx->cq);

    if (itumult != 1)
	mvwprintw(search_win, nr_bands + 1, 32, "%02d", z);
    else
	mvwprintw(search_win, nr_bands + 1, 28, "ITU:%02d", z);

    if (wpx == 1) {
	i = strlen(dx->countryname);
	mvwprintw(search_win, nr_bands + 1, 2 + i + 3, pxstr);
    }
}


#define PARTIALS_ROWS   5
#define PARTIALS_COLS   40      // including margins (1+1 columns)
#define PARTIALS_Y0     1
#define PARTIALS_X0     0

//
// return: true if display is full
//
static bool show_partial(int *row, int *col, char *call,
			 GHashTable *callset,
			 int *nr_suggested, char *suggested_call) {

    const int len = strlen(call);
    bool space = (*col > 1);         // whether to put leading space
    int new_col = *col + (space ? 1 : 0) + len;
    if (new_col >= PARTIALS_COLS) { // doesn't fit on this line
	*row += 1;                  // start next one
	*col = 1;
	space = false;
	new_col = *col + len;
    }
    if (*row >= PARTIALS_ROWS) {
	return true;    // display full
    }

    if (!g_hash_table_add(callset, call)) {
	return false;   // already shown
    }

    mvprintw(PARTIALS_Y0 + *row, PARTIALS_X0 + *col, "%s%s",
	     (space ? " " : ""), call);

    *col = new_col;

    if (*nr_suggested == 0) {   // remember first partial call
	strcpy(suggested_call, call);
    }
    *nr_suggested += 1;

    return false;   // assume it's not full yet
}

int displayPartials(char *suggested_call) {
    extern int dupe;

    int row, col, k;
    char *loc;
    char printres[14] = "";
    int suggested = 0;

    const int hislen = strlen(hiscall);

    if (hislen < 2) {
	return 0;   // input too short
    }

    row = 0;
    col = 1;        // 1 column left margin

    attron(modify_attr(COLOR_PAIR(C_LOG) | A_STANDOUT));

    for (k = 0; k < PARTIALS_ROWS; k++) {
	mvprintw(PARTIALS_Y0 + k, 0, "%*s", PARTIALS_COLS, "");
    }

    attrset(COLOR_PAIR(C_DUPE));
    mvprintw(1, 1, "??");
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

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
    if (dupe == ISDUPE) {
	attrset(COLOR_PAIR(C_DUPE));
    } else {
	attron(modify_attr(COLOR_PAIR(C_BORDER) | A_STANDOUT));
    }

    GHashTable *callset = g_hash_table_new(g_str_hash, g_str_equal);
    int full = 0;

    for (k = 0; k < srch_index && !full; k++) {
	if (strlen(searchresult[k]) <= 2) {
	    continue;   // too short (unlikely)
	}
	if (strstr(searchresult[k], hiscall) == NULL) {
	    continue;   // not matching
	}

	printres[0] = '\0';
	strncat(printres, searchresult[k] + 29, 12);

	/* cut string just at first space after call */
	loc = strchr(printres, ' ');
	if (loc)
	    *loc = '\0';

	full = show_partial(&row, &col, printres, callset,
			    &suggested, suggested_call);

    }

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    attroff(A_BOLD);

    /* and now check callmaster database */

    attron(modify_attr(COLOR_PAIR(C_LOG) | A_STANDOUT));

    // make 2 runs: fist look for calls starting with
    // then the ones containing 'hiscall'
    for (int run = 1; run <= 2; run++) {
	for (k = 0; k < callmaster->len && !full; k++) {

	    char *mastercall = CALLMASTERARRAY(k);

	    int match;
	    if (run == 1) { // starts with
		match = (strncmp(mastercall, hiscall, hislen) == 0);
	    } else {        // contains
		match = (strstr(mastercall, hiscall) != NULL);
	    }

	    if (!match) {
		continue;   // not matching
	    }

	    full = show_partial(&row, &col, mastercall, callset,
				&suggested, suggested_call);

	}
    }

    g_hash_table_destroy(callset);

    return suggested;
}

/* Display list of partials and handle USEPARTIALS auto-completion */
void handlePartials(void) {
    extern int use_part;
    extern int block_part;

    char suggested_call[LOGLINELEN + 1] = "";
    int nr_suggested;

    /* print list of partials in upper left region */
    nr_suggested = displayPartials(suggested_call);

    /* If only one partial call found and USEPARTIALS set,
    * use that call for auto-completion. Can be blocked by
    * pressing tab in calledit() function
    */
    if ((nr_suggested == 1) && use_part && !block_part
	    && strlen(suggested_call) > strlen(hiscall)) {

	strcpy(hiscall, suggested_call);
	beep();
    }
}


/* Parses searchresult and prepare string for searchwindow display from it */
void extractData(int index) {
    extern int show_time;

    g_strlcpy(result[index], searchresult[index], 7);    /* band + mode */

    if (show_time == 1) // show qso time
	strncat(result[index], searchresult[index] + 17, 5);
    else                // show qso number
	strncat(result[index], searchresult[index] + 22, 5);

    strncat(result[index], searchresult[index] + 28, 12); /* call */
    strncat(result[index], searchresult[index] + 52, 16); /* exch */
}


/* find band from bandstring and choose line 'j' for display */
int bandstr2line(char *buffer) {
    int j = 0;

    if (buffer[1] == '1' && buffer[2] == '0')
	j = 1;
    if (buffer[1] == '1' && buffer[2] == '5')
	j = 2;
    if (buffer[1] == '2')
	j = 3;
    if (buffer[1] == '4')
	j = 4;
    if (buffer[1] == '8')
	j = 5;
    if (buffer[1] == '6')
	j = 6;
    if (buffer[1] == '1' && buffer[2] == '2')
	j = 7;
    if (buffer[1] == '1' && buffer[2] == '7')
	j = 8;
    if (buffer[1] == '3' && buffer[2] == '0')
	j = 9;
    return j;
}

//
// return true if the qso was in the same mode as the current one
//
static bool is_current_mode(const char *qso) {
    extern int trxmode;
    extern int mixedmode;

    if (mixedmode) {
        return true;    // always true in mixed mode
    }

    if (qso[3] == 'S') {
        return trxmode == SSBMODE;
    }
    if (qso[3] == 'D') {
        return trxmode == DIGIMODE;
    }
    return trxmode == CWMODE;   // default
}


/* search complete Log for 'hiscall' as substring in callsign field and
 * copy found QSO to 'searchresults'. Extract relevant data to 'result'.
 * */
void filterLog() {
    extern char qsos[MAX_QSOS][LOGLINELEN + 1];

    char s_inputbuffer[LOGLINELEN + 1] = "";

    srch_index = 0;

    for (int qso_index = 0; strlen(qsos[qso_index]) > 4; qso_index++) {

        if (!is_current_mode(qsos[qso_index])) {
            continue;   // different mode
        }

        g_strlcpy(s_inputbuffer, qsos[qso_index] + 29, 13); /* call */
	if (strstr(s_inputbuffer, hiscall) == 0) {
            continue;   // no match
        }

        g_strlcpy(searchresult[srch_index], qsos[qso_index], 81);
        extractData(srch_index);

        if (srch_index++ > MAX_CALLS - 1) {
            break;
	}
    }
}

void displaySearchResults(void) {
    extern int dupe;
    extern int ignoredupe;
    extern int qso_once;
    extern int mixedmode;
    extern int ignoredupe;
    extern int minitest;
    extern int bandinx;
    extern char band[NBANDS][4];

    int r_index;
    char s_inputbuffer[LOGLINELEN + 1] = "";
    char qtccall[15];	// temp str for qtc search
    int found, display_dupe, mod;
    int z, l, j;
    time_t currtime;
    struct t_qtc_store_obj *qtc_temp_ptr;


    dupe = NODUPE;

    /* print resulting call in line according to band in check window */
    for (r_index = 0; r_index < srch_index; r_index++) {
	wattrset(search_win, COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	g_strlcpy(s_inputbuffer, result[r_index], 38);

	if ((hiscall[0] == s_inputbuffer[12]) &&
		(strlen(hiscall) >= 3 &&
		 (s_inputbuffer[12 + strlen(hiscall)] == ' '))) {
	    /* full call match */
	    if ((strncmp(band[bandinx], s_inputbuffer, 3) == 0)
		    || (qso_once == 1)) {
		/* band matches */
		if (ignoredupe == 0) { /* do we ignore dupes? */

		    display_dupe = 1;
		    if (minitest > 0) {
			found = searchcallarray(hiscall);
			if (found > -1) {
			    currtime = mktime(time_ptr);
			    mod = ((long)currtime) % minitest;	/* how many secods passed till last period */
			    if (worked[found].qsotime[trxmode][bandinx] < (((long)currtime) - mod)) {
				display_dupe = 0;
			    }
			}
		    }

		    if (display_dupe && is_current_mode(s_inputbuffer)) {
                        wattrset(search_win, COLOR_PAIR(C_DUPE));
                        dupe = ISDUPE;
                        beep();
		    }
		}		// end ignoredupe
	    }
	}

	/* display line in search window */
	j = bandstr2line(s_inputbuffer);

	if ((j < 7) || IsAllBand()) {
	    mvwprintw(search_win, j, 1, "%s", s_inputbuffer);
	}


	if ((j > 0) && (j < 7)) {  /* no WARC band */
	    if (qtcdirection > 0) {
		qtccall[0] = '\0';
		z = 12;	// first pos of callsign
		l = 0;
		do {
		    qtccall[l] = s_inputbuffer[z];
		    z++; l++;
		} while (s_inputbuffer[z] != ' ');
		qtccall[l] = '\0';

		qtc_temp_ptr = qtc_get(qtccall);
		qtcflags[j - 1] = qtc_get_value(qtc_temp_ptr);
	    }
	}

	s_inputbuffer[0] = '\0';
    }
}

int getZone() {
    char zonebuffer[3];
    int z;

    static int z1;

    z = 0;

    if (strlen(hiscall) == 2)
	z1 = 0;

    if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
	for (int i = 0; i < srch_index; i++) {

	    /* get zone nr from previous QSO */
	    /* \FIXME use nr from last displayed entry
	     * which is the right one if we have a full match
	     */
	    if (strlen(result[i]) >= 24) {
		/* convert exchange back to zone nr */
		g_strlcpy(zonebuffer, result[i] + 25, 3);
		z1 = zone_nr(zonebuffer);
	    }
	}

	if (z1 != 0) {
	    z = z1;
	} else {
	    if (strlen(zone_fix) > 1) {
		g_strlcpy(zonebuffer, zone_fix, 3);
	    } else
		g_strlcpy(zonebuffer, zone_export, 3);

	    z = zone_nr(zonebuffer);
	}
    }
    return z;
}

void displayWorkedZonesCountries(int z) {
    extern t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];
    extern int pfxnummultinr;
    extern int countries[MAX_DATALINES];
    extern int zones[MAX_ZONES];
    extern int contest;
    extern int pacc_qsos[10][10];
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

    static int pxnr;
    int pfxnumcntidx;

    /* print worked zones and countrys for each band in checkwindow */
    wattron(search_win, COLOR_PAIR(C_HEADER) | A_STANDOUT);

    if (qtcdirection > 0) {
	for (int l = 0; l < 6; l++) {
	    if (qtcflags[l] != ' ') {
		mvwprintw(search_win, l + 1, 35, "%c", qtcflags[l]);
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
	    mvwprintw(search_win, 1, 37, "Z");
	}
	if ((zones[z] & BAND15) != 0) {
	    mvwprintw(search_win, 2, 37, "Z");
	}
	if ((zones[z] & BAND20) != 0) {
	    mvwprintw(search_win, 3, 37, "Z");
	}
	if ((zones[z] & BAND40) != 0) {
	    mvwprintw(search_win, 4, 37, "Z");
	}
	if ((zones[z] & BAND80) != 0) {
	    mvwprintw(search_win, 5, 37, "Z");
	}
	if ((zones[z] & BAND160) != 0) {
	    mvwprintw(search_win, 6, 37, "Z");
	}
    }

    if (pacc_pa_flg == 1) {

	getpx(hiscall);

	pxnr = pxstr[strlen(pxstr) - 1] - 48;

	if ((countrynr == w_cty) ||
		(countrynr == ve_cty) ||
		(countrynr == ja_cty) ||
		(countrynr == py_cty) ||
		(countrynr == lu_cty) ||
		(countrynr == ua9_cty) ||
		(countrynr == zl_cty) ||
		(countrynr == ce_cty) ||
		(countrynr == zs_cty) ||
		(countrynr == vk_cty)) {
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
	    while (pfxi < pfxnummultinr) {
		if (pfxnummulti[pfxi].countrynr == countrynr) {
		    pfxnumcntidx = pfxi;
		    break;
		}
		pfxi++;
	    }
	}
	if (pfxnumcntidx >= 0) {
	    tbandidx = pfxnummulti[pfxnumcntidx].qsos[pxnr];
	} else {
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
}


void searchlog(char *searchstring) {

    extern int isdupe;		// LZ3NY auto-b4 patch
    extern int searchflg;
    extern int dupe;
    extern int partials;
    extern int cqww;
    extern int countrynr;
    extern int wpx;
    extern int arrlss;
    extern char pxstr[];
    extern char hiscall[];
    extern char zone_export[];
    extern char zone_fix[];
    extern int wazmult;
    extern int itumult;

    dxcc_data *dx;
    int zone;

    get_time();

    if (!initialized) {
	InitSearchPanel();
	initialized = 1;
    }

    /* show checkwindow and partials */
    if (strlen(hiscall) > 1 && searchflg == SEARCHWINDOW) {

	ShowSearchPanel();
	drawSearchWin();

	filterLog();
	displaySearchResults();


	/* prepare and print lower line of checkwindow */
	dx = dxcc_by_index(countrynr);
	zone = getZone();
	displayCallInfo(dx, zone, pxstr);
	displayWorkedZonesCountries(zone);

	refreshp();


	if (partials == 1) {
	    handlePartials();
	}

	/* show needed sections for ARRL_Sweep Stake*/
	if (dupe == NODUPE && arrlss == 1)
	    show_needed_sections();

	if (dupe == ISDUPE) {
	    isdupe = 1;		// LZ3NY auto-b4 patch
	    attrset(COLOR_PAIR(C_DUPE));
	    mvprintw(12, 29, hiscall);
	    refreshp();
	    usleep(500000);
	} else
	    isdupe = 0;		// LZ3NY auto-b4 patch

	printcall();

    } else {
	HideSearchPanel();
    }
}

/* loading callmaster database */
static void init_callmaster(void) {
    if (callmaster) {
	g_ptr_array_free(callmaster, TRUE);
    }
    callmaster = g_ptr_array_new_full(CALLMASTER_SIZE, g_free);
}

/** loads callmaster database from file
 * returns number of loaded calls
 */
int load_callmaster(void) {

    FILE *cfp;
    char callmaster_location[80];
    char s_inputbuffer[186] = "";

    init_callmaster();

    if (callmaster_filename == NULL)
	callmaster_filename = g_strdup("callmaster");

    strcpy(callmaster_location, callmaster_filename);
    if ((cfp = fopen(callmaster_location, "r")) == NULL) {
	callmaster_location[0] = '\0';
	strcpy(callmaster_location, PACKAGE_DATA_DIR);
	strcat(callmaster_location, "/");
	strcat(callmaster_location, callmaster_filename);

	if ((cfp = fopen(callmaster_location, "r")) == NULL) {
	    g_ptr_array_free(callmaster, TRUE);
	    callmaster = NULL;
	    TLF_LOG_WARN("Error opening callmaster file.");
	    return 0;
	}
    }

    GHashTable *callset = g_hash_table_new(g_str_hash, g_str_equal);

    while (fgets(s_inputbuffer, 85, cfp) != NULL) {

	g_strstrip(s_inputbuffer);

	/* skip comment lines and calls shorter than 3 chars */
	if (s_inputbuffer[0] == '#' || strlen(s_inputbuffer) < 3) {
	    continue;
	}

	char *call = g_ascii_strup(s_inputbuffer, 11);

	if (arrlss) {
	    /* keep only NA stations */
	    if (strchr("AKWVCN", call[0]) == NULL) {
		g_free(call);
		continue;
	    }
	}

	if (g_hash_table_contains(callset, call)) { // already have this call
	    g_free(call);
	    continue;
	}

	g_hash_table_add(callset, call);

	g_ptr_array_add(callmaster, call);
    }

    g_hash_table_destroy(callset);

    fclose(cfp);
    return callmaster->len;
}


/*  --------------------------------------------------------------  */
void show_needed_sections(void) {
    extern int arrlss;
    extern int nr_multis;
    extern struct mults_t multis[MAX_MULTS];

    int j, vert, hor, cnt, found;
    char mprint[50];

    if (arrlss == 1) {
	cnt = 0;

	wattron(search_win, modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

	for (j = 1; j < 7; j++)
	    mvwprintw(search_win, j, 1, spaces(37));

	for (vert = 1; vert < 7; vert++) {
	    if (cnt >= get_mult_count())
		break;

	    for (hor = 0; hor < 9; hor++) {
		if (cnt >= get_mult_count())
		    break;

		strcpy(mprint, get_mult(cnt));

		found = 0;
		for (j = 0; j < nr_multis; j++) {
		    if (strncmp(multis[j].name, mprint, strlen(mprint)) == 0) {
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

void OnLowerSearchPanel(int x, char *str) {
    int y = 1 + (IsAllBand() ? 9 : 6);

    wattrset(search_win, modify_attr(COLOR_PAIR(C_BORDER)));
    mvwprintw(search_win, y, x, str);
}
