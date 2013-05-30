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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
	/* ------------------------------------------------------------
	 *     showscore
	 *
	 *-------------------------------------------------------------*/

#include "globalvars.h"
#include "showscore.h"

#define START_COL 45	/* start display in these column */

/* list of columns to display score for each band */
static int band_cols[6] =
	{ 50, 55, 60, 65, 70, 75 };

/* list of BANDINDEX entries to show in each column 
 * first  - for normal contest bands
 * second - if in warc band */
static int bandindex_normal[6] =
	{ BANDINDEX_160, BANDINDEX_80, BANDINDEX_40,
	  BANDINDEX_20,  BANDINDEX_15, BANDINDEX_10 };
static int bandindex_warc[6] =
	{ BANDINDEX_160, BANDINDEX_80, BANDINDEX_40,
	  BANDINDEX_30,  BANDINDEX_17, BANDINDEX_12 };


void printfield (int x, int y, int number);

/* show summary line */
void show_summary( int points, int multi )
{
    mvprintw(5, START_COL, "                                   ");
    /* TODO: respect field boundaries for large numbers */ 
    mvprintw(5, START_COL, "Pts: %d  Mul: %d Score: %d", 
	points, multi, points * multi);
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
    extern int fixedmult;
    extern int zonescore[6];
    extern int countryscore[6];
    extern int totalmults;
    extern int totalcountries;
    extern int totalzones;
    extern int nr_of_px;
    extern int qsonum;
    extern int total;
    extern int wpx;
    extern int sprint;
    extern int bandinx;
    extern int multscore[];
    extern int multlist;
    extern int serial_section_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;

    int i, p, q, r, n, l10;

    if (showscore_flag == 1) {

	/* show header and number of QSOs */
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);

	if ((bandinx != BANDINDEX_30) && (bandinx != BANDINDEX_17)
	    && (bandinx != BANDINDEX_12)) {
	    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	    mvprintw(1, START_COL, "Band   160   80   40   20   15   10");

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
	    mvprintw(2, START_COL, "QSO's ");

	    for (i = 0; i < 6; i++) {
	    	printfield(2, band_cols[i], band_score[bandindex_normal[i]]);
	    }
	} else {
	    attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	    mvprintw(1, START_COL, "Band   160   80   40   30   17   12");

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
	    mvprintw(2, START_COL, "QSO's ");

	    for (i = 0; i < 6; i++) {
	    	printfield(2, band_cols[i], band_score[bandindex_warc[i]]);
	    }
	}
	mvprintw(3, START_COL, "                                   ");
	mvprintw(4, START_COL, "                                   ");
	mvprintw(5, START_COL, "                                   ");

	/* show score per band */
	if ((wysiwyg_multi == 1)
	    || (serial_section_mult == 1)
	    || (serial_grid4_mult == 1)
	    || (sectn_mult == 1)) {

	    mvprintw(3, START_COL, "Mult ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], multscore[bandindex_normal[i]]);
	    }
	}

	if (dx_arrlsections == 1) {

	    mvprintw(3, START_COL, "Cty  ");
	    for (i = 0; i < 6; i++) {
	    	printfield(3, band_cols[i], countryscore[i]);
	    }

	    mvprintw(4, START_COL, "Sect");
	    for (i = 0; i < 6; i++) {
	    	printfield(4, band_cols[i], multscore[bandindex_normal[i]]);
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
	    mvprintw(5, START_COL, "Score: %d", total);
	}

	if (arrlss == 1) {

	    show_summary( total, multarray_nr );
	}

	if (cqww == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalzones = totalzones + zonescore[n];
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries + totalzones;

	    show_summary( total, totalmults );
	}

	if (arrldx_usa == 1) {

	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    show_summary( total, totalmults );
	}

	if (arrl_fd == 1) {
	    if (fixedmult != 0) {
		totalmults = fixedmult;
	    } else {
		totalmults = 1;
	    }
	    show_summary( total, totalmults );
	}

	if (universal == 1 && country_mult == 1) {

	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    show_summary( total, totalmults );
	}

	if (universal == 1 && multlist == 1 && arrlss != 1) {

	    /* FIXME: Who provides totalmults here? */
	    show_summary( total, totalmults );
	}

	if (pacc_pa_flg == 1) {

	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    show_summary( total, totalmults );
	}

	if (wysiwyg_once == 1) {

	    totalmults = multarray_nr;

	    show_summary( total, totalmults );
	}

	if ((wysiwyg_multi == 1)
	    || (serial_section_mult == 1)
	    || (serial_grid4_mult == 1)
	    || (sectn_mult == 1)) {

	    totalmults =
		multscore[BANDINDEX_160] + multscore[BANDINDEX_80] +
		multscore[BANDINDEX_40] + multscore[BANDINDEX_20] +
		multscore[BANDINDEX_15] + multscore[BANDINDEX_10];

	    show_summary( total, totalmults );
	}

	if (dx_arrlsections == 1) {
	    totalmults =
		multscore[BANDINDEX_160] + multscore[BANDINDEX_80] +
		multscore[BANDINDEX_40] + multscore[BANDINDEX_20] +
		multscore[BANDINDEX_15] + multscore[BANDINDEX_10];
	    totalmults =
		totalmults + countryscore[0] + countryscore[1] +
		countryscore[2] + countryscore[3] + countryscore[4] +
		countryscore[5];

	    show_summary( total, totalmults );
	}

	if (wpx == 1) {		/* wpx */
	    mvhline(3, START_COL, ACS_HLINE, 35);
	    mvprintw(4, START_COL, "                                   ");

	    show_summary( total, nr_of_px );

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
	    if (wpx == 1)
		totalmults = nr_of_px;
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

	/* show active band */
	attrset(COLOR_PAIR(C_DUPE));

	switch (bandinx) {
	case BANDINDEX_160:
	    {
		mvprintw(1, 52, "160");
		break;
	    }
	case BANDINDEX_80:
	    {
		mvprintw(1, 57, " 80");
		break;
	    }
	case BANDINDEX_40:
	    {
		mvprintw(1, 62, " 40");
		break;
	    }
	case BANDINDEX_30:
	    {
		mvprintw(1, 67, " 30");
		break;
	    }
	case BANDINDEX_20:
	    {
		mvprintw(1, 67, " 20");
		break;
	    }
	case BANDINDEX_17:
	    {
		mvprintw(1, 72, " 17");
		break;
	    }
	case BANDINDEX_15:
	    {
		mvprintw(1, 72, " 15");
		break;
	    }
	case BANDINDEX_12:
	    {
		mvprintw(1, 77, " 12");
		break;
	    }
	case BANDINDEX_10:
	    {
		mvprintw(1, 77, " 10");
		break;
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

