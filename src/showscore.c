/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 		 2010, 2011 Thomas Beierlein <tb@forth-ev.de>
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
	 *     showscore
	 *
	 *-------------------------------------------------------------*/

#include "globalvars.h"
#include "showscore.h"
#include <assert.h>


#define START_COL 45	/* start display in these column */

/* list of columns to display score for each band */
static int band_cols[6] =
	{ 50, 55, 60, 65, 70, 75 };

/* list of BANDINDEX entries to show in each column 
 * first  - for normal contest bands
 * second - if in warc band */
static int bi_normal[6] =
	{ BANDINDEX_160, BANDINDEX_80, BANDINDEX_40,
	  BANDINDEX_20,  BANDINDEX_15, BANDINDEX_10 };
static int bi_warc[6] =
	{ BANDINDEX_160, BANDINDEX_80, BANDINDEX_40,
	  BANDINDEX_30,  BANDINDEX_17, BANDINDEX_12 };

/* bands as numbers */
static int bands[NBANDS] =
	{ 160, 80, 40, 30, 20, 17, 15, 12, 10 };

void printfield (int x, int y, int number);


/* show summary line */
void show_summary( int points, int multi )
{
    mvprintw(5, START_COL, "                                   ");
    /* TODO: respect field boundaries for large numbers */ 
    mvprintw(5, START_COL, "Pts: %d  Mul: %d Score: %d", 
	points, multi, points * multi);
}


/* show header with active band and number of QSOs.
 * Use the list of bandindices in 'bi' for that */
void display_header(int *bi)
{
    int i;

    /* prepare header line */
    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
    mvprintw(1, START_COL, "Band ");
    for (i = 0; i < 6; i++) {
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	addstr("  ");
	if (bandinx == bi[i]) {		/* highlite active band */
	    attrset(COLOR_PAIR(C_DUPE));
	}
	printw("%3d", bands[bi[i]]);
    }

    /* show number of QSO */
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvprintw(2, START_COL, "QSO's ");
    for (i = 0; i < 6; i++) {
	printfield(2, band_cols[i], band_score[bi[i]]);
    }

    mvprintw(3, START_COL, "                                   ");
    mvprintw(4, START_COL, "                                   ");
    mvprintw(5, START_COL, "                                   ");

}



/* get total number of points */
int get_nr_of_points()
{
    return total;
}


/* get total number of multis */
int get_nr_of_mults()
{
    extern int fixedmult;
    extern int sprint;
    extern int multlist;
    extern int multscore[];

    int n;
    int totalzones;
    int totalcountries;
    int totalmults;

    /* precalculate summaries */
    totalzones = 0;
    totalcountries = 0;
    totalmults = 0;

    for (n = 0; n < 6; n++) {
	totalzones += zonescore[n];
	totalcountries += countryscore[n];
	totalmults += multscore[bi_normal[n]];
    }

    if (sprint == 1) {
	/* no multis used */
	return 1;
    }
    else if (arrlss == 1) {

	return multarray_nr;
    }
    else if (cqww == 1) {

	return totalcountries + totalzones;
    }
    else if (arrldx_usa == 1) {

	return totalcountries;
    }
    else if (arrl_fd == 1) {
	if (fixedmult != 0) {
	    return fixedmult;
	} else {
	    return 1;
	}
    }
    else if (universal == 1 && country_mult == 1) {

	return totalcountries;
    }
    else if (universal == 1 && multlist == 1 && arrlss != 1) {

	/* FIXME: Who provides totalmults here? */
	return totalmults ;
    }
    else if (pacc_pa_flg == 1) {

	return totalcountries;
    }
    else if (wysiwyg_once == 1) {

	return multarray_nr;
    }
    else if ((wysiwyg_multi == 1)
	|| (serial_section_mult == 1)
	|| (serial_grid4_mult == 1)
	|| (sectn_mult == 1)) {

	return totalmults;
    }
    else if (dx_arrlsections == 1) {

	return totalmults + totalcountries;
    }
    else if (wpx == 1) {

	return nr_of_px;
    }
    else 
	/* should never reach that point */
	assert(1 == 0);
	return 1;
}


int showscore(void)
{

    extern int showscore_flag;
    extern int band_score[9];
    extern int cqww;
    extern int arrldx_usa;
    extern int arrl_fd;
    extern int arrlss;
    extern int pacc_pa_flg;
    extern int universal;
    extern int country_mult;
    extern int wysiwyg_once;
    extern int wysiwyg_multi;
    extern int zonescore[6];
    extern int countryscore[6];
    extern int totalmults;
    extern int nr_of_px;
    extern int qsonum;
    extern int total;
    extern int wpx;
    extern int sprint;
    extern int bandinx;
    extern int multscore[];
    extern int serial_section_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;

    int i, p, q, r, l10;

    if (showscore_flag == 1) {

	/* show header with active band and number of QSOs */
	if ((bandinx != BANDINDEX_30) && (bandinx != BANDINDEX_17)
	    && (bandinx != BANDINDEX_12)) {

	    display_header(bi_normal);

	} else {

	    display_header(bi_warc);

	}

	/* show score per band */
	if ((wysiwyg_multi == 1)
	    || (serial_section_mult == 1)
	    || (serial_grid4_mult == 1)
	    || (sectn_mult == 1)) {

	    mvprintw(3, START_COL, "Mult ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], multscore[bi_normal[i]]);
	    }
	}

	if (dx_arrlsections == 1) {

	    mvprintw(3, START_COL, "Cty  ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], countryscore[i]);
	    }

	    mvprintw(4, START_COL, "Sect");
	    for (i = 0; i < 6; i++) {
	    	printfield(4, band_cols[i], multscore[bi_normal[i]]);
	    }
	}

	if (cqww == 1) {

	    mvprintw(3, START_COL, "Cty  ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], countryscore[i]);
	    }

	    mvprintw(4, START_COL, "Zone ");
	    for (i = 0; i < 6; i++) {
	    	printfield(4, band_cols[i], zonescore[i]);
	    }
	}

	if (arrldx_usa == 1) {

	    mvprintw(3, START_COL, "Cty  ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], countryscore[i]);
	    }
	}

	if (universal == 1 && country_mult == 1) {

	    mvprintw(3, START_COL, "Cty  ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], countryscore[i]);
	    }
	}

	if (pacc_pa_flg == 1) {

	    mvprintw(3, START_COL, "Cty  ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], countryscore[i]);
	    }
	}


	/* show score summary */
	if (sprint == 1) {

	    mvprintw(5, START_COL, "Score: %d", get_nr_of_points() );
	}
	else {

	    show_summary( get_nr_of_points(), get_nr_of_mults() );
	}


	if (wpx == 1) {		/* wpx */
	    mvhline(3, START_COL, ACS_HLINE, 35);
	    mvprintw(4, START_COL, "                                   ");


	    if (nr_of_px >= 2) {
		p = (qsonum - 1) / (nr_of_px);
		q = 10 * ((qsonum - 1) - (p * (nr_of_px)));
		r = q / (nr_of_px);
	    } else {
		p = 1;
		r = 0;
	    }
	}


	/* show statistics */
	attron(COLOR_PAIR(C_HEADER));
	if ((cqww == 1) || (wpx == 1) || (arrldx_usa == 1) || (pacc_pa_flg == 1) || (wysiwyg_once == 1) || (universal == 1)) {	/* cqww or wpx */

	    totalmults = get_nr_of_mults();
	    /** \todo fix calculation of Q/M */
	    if (totalmults >= 2)
		p = (total / totalmults);
	    else
		p = 1;

	    if ((l10 = last10()) >= 1)
		mvprintw(6, 55, "Q/M %d  Rate %d ", p, (60 * 10) / l10);

	    if (wpx == 1) {
		if (minute_timer > 0)
		    mvprintw(6, 75, "%d", minute_timer);
	    }
	}

	printcall();

    }
    return (0);
}

/** formated print of integer number 0..9999 */
void printfield (int y, int x, int number)
{
    attron(COLOR_PAIR(C_LOG));

    mvprintw(y, x, " %4d", number);
}

