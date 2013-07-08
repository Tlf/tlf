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

void printfield (int x, int y, int number);

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

    int p, q, r, n, l10;

    if (showscore_flag == 1) {

	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	if ((bandinx != BANDINDEX_30) && (bandinx != BANDINDEX_17)
	    && (bandinx != BANDINDEX_12)) {
	    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);
	    mvprintw(1, 45, "Band   160   80   40   20   15   10");
	    attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);
	    mvprintw(2, 45, "QSO's ");

	    printfield(2, 50, band_score[BANDINDEX_160]);
	    printfield(2, 55, band_score[BANDINDEX_80]);
	    printfield(2, 60, band_score[BANDINDEX_40]);
	    printfield(2, 65, band_score[BANDINDEX_20]);
	    printfield(2, 70, band_score[BANDINDEX_15]);
	    printfield(2, 75, band_score[BANDINDEX_10]);
	} else {
	    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);
	    mvprintw(1, 45, "Band   160   80   40   30   17   12");
	    attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);
	    mvprintw(2, 45, "QSO's ");

	    printfield(2, 50, band_score[BANDINDEX_160]);
	    printfield(2, 55, band_score[BANDINDEX_80]);
	    printfield(2, 60, band_score[BANDINDEX_40]);
	    printfield(2, 65, band_score[BANDINDEX_30]);
	    printfield(2, 70, band_score[BANDINDEX_17]);
	    printfield(2, 75, band_score[BANDINDEX_12]);

	}
	mvprintw(3, 45, "                                   ");
	mvprintw(4, 45, "                                   ");
	mvprintw(5, 45, "                                   ");

	if ((wysiwyg_multi == 1)
	    || (serial_section_mult == 1)
	    || (serial_grid4_mult == 1)
	    || (sectn_mult == 1)) {
	    mvprintw(3, 45, "Mult ");
	    printfield(3, 50, multscore[BANDINDEX_160]);
	    printfield(3, 55, multscore[BANDINDEX_80]);
	    printfield(3, 60, multscore[BANDINDEX_40]);
	    printfield(3, 65, multscore[BANDINDEX_20]);
	    printfield(3, 70, multscore[BANDINDEX_15]);
	    printfield(3, 75, multscore[BANDINDEX_10]);

	}
	if (dx_arrlsections == 1) {
	    mvprintw(3, 45, "Cty  ");
	    printfield(3, 50, countryscore[0]);
	    printfield(3, 55, countryscore[1]);
	    printfield(3, 60, countryscore[2]);
	    printfield(3, 66, countryscore[3]);
	    printfield(3, 70, countryscore[4]);
	    printfield(3, 75, countryscore[5]);
	    mvprintw(4, 45, "Sect");
	    printfield(4, 50, multscore[BANDINDEX_160]);
	    printfield(4, 55, multscore[BANDINDEX_80]);
	    printfield(4, 60, multscore[BANDINDEX_40]);
	    printfield(4, 65, multscore[BANDINDEX_20]);
	    printfield(4, 70, multscore[BANDINDEX_15]);
	    printfield(4, 75, multscore[BANDINDEX_10]);

	}

	if (cqww == 1) {

	    mvprintw(3, 45, "Cty  ");
	    printfield(3, 50, countryscore[0]);
	    printfield(3, 55, countryscore[1]);
	    printfield(3, 60, countryscore[2]);
	    printfield(3, 65, countryscore[3]);
	    printfield(3, 70, countryscore[4]);
	    printfield(3, 75, countryscore[5]);

	    mvprintw(4, 45, "Zone ");
	    printfield(4, 50, zonescore[0]);
	    printfield(4, 55, zonescore[1]);
	    printfield(4, 60, zonescore[2]);
	    printfield(4, 65, zonescore[3]);
	    printfield(4, 70, zonescore[4]);
	    printfield(4, 75, zonescore[5]);

	}

	if (arrldx_usa == 1) {

	    mvprintw(3, 45, "Cty  ");
	    printfield(3, 50, countryscore[0]);
	    printfield(3, 55, countryscore[1]);
	    printfield(3, 60, countryscore[2]);
	    printfield(3, 65, countryscore[3]);
	    printfield(3, 70, countryscore[4]);
	    printfield(3, 75, countryscore[5]);
	}

	if (universal == 1) {
	    if (country_mult == 1) {
		mvprintw(3, 45, "Cty  ");
		printfield(3, 50, countryscore[0]);
		printfield(3, 55, countryscore[1]);
		printfield(3, 60, countryscore[2]);
		printfield(3, 65, countryscore[3]);
		printfield(3, 70, countryscore[4]);
		printfield(3, 75, countryscore[5]);
	    }
	}

	if (pacc_pa_flg == 1) {

	    mvprintw(3, 45, "Cty  ");
	    printfield(3, 50, countryscore[0]);
	    printfield(3, 55, countryscore[1]);
	    printfield(3, 60, countryscore[2]);
	    printfield(3, 65, countryscore[3]);
	    printfield(3, 70, countryscore[4]);
	    printfield(3, 75, countryscore[5]);
	}

	if (sprint == 1) {

	    mvprintw(5, 45, "Score: %d", total);
	}
	if (arrlss == 1) {

	    mvprintw(5, 45, "Sections: %d      Score: %d", multarray_nr,
		     multarray_nr * total);
	}

	if (cqww == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalzones = totalzones + zonescore[n];
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries + totalzones;

	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (arrldx_usa == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (arrl_fd == 1) {
	    if (fixedmult != 0) {
		totalmults = fixedmult;
	    } else {
		totalmults = 1;
	    }
	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);
	}

	if (universal == 1 && country_mult == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (universal == 1 && multlist == 1 && arrlss != 1) {

	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Total: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (pacc_pa_flg == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (wysiwyg_once == 1) {

	    totalmults = multarray_nr;
	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if ((wysiwyg_multi == 1)
	    || (serial_section_mult == 1)
	    || (serial_grid4_mult == 1)
	    || (sectn_mult == 1)) {

	    totalmults =
		multscore[BANDINDEX_160] + multscore[BANDINDEX_80] +
		multscore[BANDINDEX_40] + multscore[BANDINDEX_20] +
		multscore[BANDINDEX_15] + multscore[BANDINDEX_10];
	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

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
	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 45, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (wpx == 1) {		/* wpx */
	    mvprintw(4, 45, "                                   ");
	    mvprintw(5, 45, "                                   ");
	    mvprintw(5, 47, "PX:%d  Pts:%d  Score: %d", nr_of_px, total,
		     (total * (nr_of_px)));
	    mvhline(3, 45, ACS_HLINE, 35);

	    if (nr_of_px >= 2) {
		p = (qsonum - 1) / (nr_of_px);
		q = 10 * ((qsonum - 1) - (p * (nr_of_px)));
		r = q / (nr_of_px);
	    } else {
		p = 1;
		r = 0;
	    }
	}


	attron(COLOR_PAIR(COLOR_GREEN));
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

	attron(COLOR_PAIR(COLOR_MAGENTA) | A_STANDOUT);

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
    attron(COLOR_PAIR(COLOR_WHITE));

    mvprintw(y, x, " %4d", number);
}

