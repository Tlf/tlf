/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2010 - 2015    Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------
 *     showscore
 *
 *-------------------------------------------------------------*/


#include <math.h>

#include "addpfx.h"
#include "bands.h"
#include "focm.h"
#include "globalvars.h"		// Includes tlf.h
#include "last10.h"
#include "nicebox.h"		// Includes curses.h
#include "printcall.h"
#include "bands.h"
#include "setcontest.h"
#include "ui_utils.h"

#define START_COL 45	/* start display in these column */


/* list of columns to display score for each band */
static int band_cols[6] =
{ 50, 55, 60, 65, 70, 75 };

/* list of BANDINDEX entries to show in each column
 * first  - for normal contest bands
 * second - if in warc band */
static int bi_normal[6] = {
    BANDINDEX_160, BANDINDEX_80, BANDINDEX_40,
    BANDINDEX_20,  BANDINDEX_15, BANDINDEX_10
};
static int bi_warc[6] = {
    BANDINDEX_160, BANDINDEX_80, BANDINDEX_40,
    BANDINDEX_30,  BANDINDEX_17, BANDINDEX_12
};


void printfield(int y, int x, int number);
void stewperry_show_summary(int points, float fixedmult);


/* show summary line for stewperry */
void stewperry_show_summary(int points, float fixedmult) {
    float mult;

    mvaddstr(5, START_COL, spaces(80 - START_COL));
    /* TODO: respect field boundaries for large numbers */
    mult = (fixedmult == 0.0) ? 1.0 : fixedmult;

    mvprintw(5, START_COL, "Pts: %d       Score: %d",
	     points, (int)floor(points * mult));

}

/* show summary line */
void show_summary(int points, int multi) {
    mvaddstr(5, START_COL, spaces(80 - START_COL));
    /* TODO: respect field boundaries for large numbers */
    mvprintw(5, START_COL, "Pts: %d  Mul: %d Score: %d",
	     points, multi, points * multi);
}


/** show scoring header
 *
 * show header with active band and number of QSOs.
 * Use the list of bandindices in 'bi' for that
 *
 * \param bi  list of band indices to use
 */
void display_header(int *bi) {
    int i;

    /* prepare header line */
    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
    mvaddstr(1, START_COL, "Band ");
    for (i = 0; i < 6; i++) {
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	addstr("  ");
	if (bandinx == bi[i]) {		/* highlight active band */
	    attrset(COLOR_PAIR(C_DUPE));
	}
	printw("%3d", bandindex2nr(bi[i]));
    }

    /* show number of QSO */
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvaddstr(2, START_COL, "QSO   ");
    for (i = 0; i < 6; i++) {
	printfield(2, band_cols[i], qsos_per_band[bi[i]]);
    }

    mvaddstr(3, START_COL, spaces(80 - START_COL));
    mvaddstr(4, START_COL, spaces(80 - START_COL));
    mvaddstr(5, START_COL, spaces(80 - START_COL));

}


/* get total number of points */
int get_nr_of_points() {
    return total;
}


/* get total number of multis */
int get_nr_of_mults() {

    if (!iscontest)
	return 1;

    /* precalculate weighted summaries */
    int totalzones = 0;
    int totalcountries = 0;
    int totalmults = 0;

    for (int n = 0; n < 6; n++) {
	int bandweight = bandweight_multis[bi_normal[n]];

	totalzones += zonescore[bi_normal[n]] * bandweight;
	totalcountries += countryscore[bi_normal[n]] * bandweight;
	totalmults += multscore[bi_normal[n]] * bandweight;
    }

    if (CONTEST_IS(SPRINT)) {
	/* no multis used */
	return 1;
    } else if (CONTEST_IS(ARRL_SS)) {

	return nr_multis;
    } else if (CONTEST_IS(CQWW)) {

	return totalcountries + totalzones;
    } else if (CONTEST_IS(ARRLDX_USA)) {

	return totalcountries;
    } else if (CONTEST_IS(ARRL_FD)) {
	/* arrl mults are always integers */
	int mult = (int)floor(fixedmult + 0.5); 	/* round to nearest integer */
	if (mult > 0) {
	    return mult;
	} else {
	    return 1;
	}
    } else if (dx_arrlsections) {

	return totalmults + totalcountries;
    } else if (country_mult) {

	return totalcountries;
    } else if (CONTEST_IS(PACC_PA)) {

	return totalcountries;
    } else if (wysiwyg_once
	       || unique_call_multi == MULT_ALL
	       || generic_mult == MULT_ALL
	       || sectn_mult_once) {

	return nr_multis;
    } else if (wysiwyg_multi
	       || unique_call_multi == MULT_BAND
	       || generic_mult == MULT_BAND
	       || serial_section_mult
	       || serial_grid4_mult
	       || sectn_mult) {

	return totalmults;
    } else if (CONTEST_IS(WPX) || pfxmult) {

	return GetNrOfPfx_once();
    } else if (pfxmultab) {

	return GetNrOfPfx_multiband();
    } else if (itumult || wazmult) {
	return totalzones;
    } else if (multlist == 1 && !CONTEST_IS(ARRL_SS)) {

	return totalmults ;
    } else
	/* should never reach that point
	 *
	 * \TODO: so we need some instrument of warning here
	 */
	return 1;
}


/* calculate total score */
int get_total_score() {
    if (CONTEST_IS(FOCMARATHON))
	return foc_total_score();
    else
	return get_nr_of_points() * get_nr_of_mults();
}


/** show contest score
 *
 * display scoring results of contest if activated by 'showscore_flag'
 */
void showscore(void) {

    extern int totalmults;

    int i, l10;
    float p;

    if (!showscore_flag) {
	return;
    }

    /* show header with active band and number of QSOs */
    if (!IsWarcIndex(bandinx)) {

	display_header(bi_normal);

    } else {

	display_header(bi_warc);
    }

    /* show mults per band, if applicable */
    if (wysiwyg_multi
	    || unique_call_multi == MULT_BAND
	    || generic_mult == MULT_BAND
	    || serial_section_mult
	    || serial_grid4_mult
	    || sectn_mult) {

	mvaddstr(3, START_COL, "Mult ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], multscore[bi_normal[i]]);
	}

    } else if (itumult || wazmult) {

	mvaddstr(3, START_COL, "Mult ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], zonescore[bi_normal[i]]);
	}

    } else if (pfxmultab) {

	mvaddstr(3, START_COL, "Mult ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], GetNrOfPfx_OnBand(bi_normal[i]));
	}

    } else if (dx_arrlsections) {

	mvaddstr(3, START_COL, "Cty  ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], countryscore[bi_normal[i]]);
	}

	mvaddstr(4, START_COL, "Sect");
	for (i = 0; i < 6; i++) {
	    printfield(4, band_cols[i], multscore[bi_normal[i]]);
	}

    } else if (CONTEST_IS(CQWW)) {

	mvaddstr(3, START_COL, "Cty  ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], countryscore[bi_normal[i]]);
	}

	mvaddstr(4, START_COL, "Zone ");
	for (i = 0; i < 6; i++) {
	    printfield(4, band_cols[i], zonescore[bi_normal[i]]);
	}

    } else if (CONTEST_IS(ARRLDX_USA)) {

	mvaddstr(3, START_COL, "Cty  ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], countryscore[bi_normal[i]]);
	}

    } else if (iscontest && country_mult) {

	mvaddstr(3, START_COL, "Cty  ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], countryscore[bi_normal[i]]);
	}

    } else if (CONTEST_IS(PACC_PA)) {

	mvaddstr(3, START_COL, "Cty  ");
	for (i = 0; i < 6; i++) {
	    printfield(3, band_cols[i], countryscore[bi_normal[i]]);
	}
    }

    /* show score summary */
    if (CONTEST_IS(SPRINT)) {

	mvprintw(5, START_COL, "Score: %d", get_nr_of_points());
    } else if (CONTEST_IS(FOCMARATHON)) {
	foc_show_scoring(START_COL);
    } else if (CONTEST_IS(STEWPERRY)) {
	/* no normal multis, but may have POWERMULT set (fixedmult != 0.) */
	stewperry_show_summary(get_nr_of_points(), fixedmult);
    } else {
	show_summary(get_nr_of_points(), get_nr_of_mults());
    }


    /* show statistics */
    attron(COLOR_PAIR(C_HEADER));
    mvaddstr(6, 55, spaces(19));

    if (iscontest) {   /* show statistics in any contest */

	totalmults = get_nr_of_mults();
	totalmults = totalmults ? totalmults : 1;	/* at least one */
	p = ((qsonum - 1) / (float)totalmults);

	if ((l10 = last10()) >= 1)
	    mvprintw(6, 55, "Q/M %.1f  Rate %d ", p, (60 * 10) / l10);
	else
	    mvprintw(6, 55, "Q/M %.1f ", p);
    }

    if (CONTEST_IS(WPX)) {
	if (minute_timer > 0)
	    mvprintw(6, 75, "%d", minute_timer);
    }

    printcall();

}

/** formatted print of integer number 0..9999 */
void printfield(int y, int x, int number) {
    attron(COLOR_PAIR(C_LOG));

    mvprintw(y, x, " %4d", number);
}

