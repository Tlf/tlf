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
	 *     showscore
	 *
	 *-------------------------------------------------------------*/

#include "globalvars.h"
#include "showscore.h"
#include "freq_display.h"
#include <glib.h>

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
    extern char whichcontest[];
    extern int total;
    extern int wpx;
    extern int sprint;
    extern int bandinx;
    extern int multscore[];
    extern int partials;
    extern int bigpartlist;
    extern char hiscall[];
    extern int multlist;
    extern int serial_section_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;

    int p, q, r, n, l10;

    if ((bigpartlist == 1) && (partials == 1) && (strlen(hiscall) > 2))
	return (1);

    if (showscore_flag == 1) {

	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	if ((bandinx != BANDINDEX_30) && (bandinx != BANDINDEX_17)
	    && (bandinx != BANDINDEX_12)) {
	    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
	    mvprintw(17, 40, "Band   160   80   40   20   15   10");
	    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);
	    mvprintw(18, 40, "QSO's ");

	    printfield(18, 45, band_score[BANDINDEX_160]);
	    printfield(18, 50, band_score[BANDINDEX_80]);
	    printfield(18, 55, band_score[BANDINDEX_40]);
	    printfield(18, 60, band_score[BANDINDEX_20]);
	    printfield(18, 65, band_score[BANDINDEX_15]);
	    printfield(18, 70, band_score[BANDINDEX_10]);
	} else {
	    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
	    mvprintw(17, 40, "Band   160   80   40   30   17   12");
	    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);
	    mvprintw(18, 40, "QSO's ");

	    printfield(18, 45, band_score[BANDINDEX_160]);
	    printfield(18, 50, band_score[BANDINDEX_80]);
	    printfield(18, 55, band_score[BANDINDEX_40]);
	    printfield(18, 60, band_score[BANDINDEX_30]);
	    printfield(18, 65, band_score[BANDINDEX_17]);
	    printfield(18, 70, band_score[BANDINDEX_12]);

	}
	mvprintw(19, 40, "                                   ");
	mvprintw(20, 40, "                                   ");
	mvprintw(21, 40, "                                   ");

	if ((wysiwyg_multi == 1)
	    || (serial_section_mult == 1)
	    || (serial_grid4_mult == 1)
	    || (sectn_mult == 1)) {
	    mvprintw(19, 40, "Mult ");
	    printfield(19, 45, multscore[BANDINDEX_160]);
	    printfield(19, 50, multscore[BANDINDEX_80]);
	    printfield(19, 55, multscore[BANDINDEX_40]);
	    printfield(19, 60, multscore[BANDINDEX_20]);
	    printfield(19, 65, multscore[BANDINDEX_15]);
	    printfield(19, 70, multscore[BANDINDEX_10]);

	}
	if (dx_arrlsections == 1) {
	    mvprintw(19, 40, "Cty  ");
	    printfield(19, 45, countryscore[0]);
	    printfield(19, 50, countryscore[1]);
	    printfield(19, 55, countryscore[2]);
	    printfield(19, 60, countryscore[3]);
	    printfield(19, 65, countryscore[4]);
	    printfield(19, 70, countryscore[5]);
	    mvprintw(20, 40, "Sect");
	    printfield(20, 45, multscore[BANDINDEX_160]);
	    printfield(20, 50, multscore[BANDINDEX_80]);
	    printfield(20, 55, multscore[BANDINDEX_40]);
	    printfield(20, 60, multscore[BANDINDEX_20]);
	    printfield(20, 65, multscore[BANDINDEX_15]);
	    printfield(20, 70, multscore[BANDINDEX_10]);

	}

	if (cqww == 1) {

	    mvprintw(19, 40, "Cty  ");
	    printfield(19, 45, countryscore[0]);
	    printfield(19, 50, countryscore[1]);
	    printfield(19, 55, countryscore[2]);
	    printfield(19, 60, countryscore[3]);
	    printfield(19, 65, countryscore[4]);
	    printfield(19, 70, countryscore[5]);

	    mvprintw(20, 40, "Zone ");
	    printfield(20, 45, zonescore[0]);
	    printfield(20, 50, zonescore[1]);
	    printfield(20, 55, zonescore[2]);
	    printfield(20, 60, zonescore[3]);
	    printfield(20, 65, zonescore[4]);
	    printfield(20, 70, zonescore[5]);

	}

	if (arrldx_usa == 1) {

	    mvprintw(19, 40, "Cty  ");
	    printfield(19, 45, countryscore[0]);
	    printfield(19, 50, countryscore[1]);
	    printfield(19, 55, countryscore[2]);
	    printfield(19, 60, countryscore[3]);
	    printfield(19, 65, countryscore[4]);
	    printfield(19, 70, countryscore[5]);
	}

	if (universal == 1) {
	    if (country_mult == 1) {
		mvprintw(19, 40, "Cty  ");
		printfield(19, 45, countryscore[0]);
		printfield(19, 50, countryscore[1]);
		printfield(19, 55, countryscore[2]);
		printfield(19, 60, countryscore[3]);
		printfield(19, 65, countryscore[4]);
		printfield(19, 70, countryscore[5]);
	    }
	}

	if (pacc_pa_flg == 1) {

	    mvprintw(19, 40, "Cty  ");
	    printfield(19, 45, countryscore[0]);
	    printfield(19, 50, countryscore[1]);
	    printfield(19, 55, countryscore[2]);
	    printfield(19, 60, countryscore[3]);
	    printfield(19, 65, countryscore[4]);
	    printfield(19, 70, countryscore[5]);
	}

	if (sprint == 1) {

	    mvprintw(21, 40, "Score: %d", total);
	}
	if (arrlss == 1) {

	    mvprintw(21, 40, "Sections: %d      Score: %d", multarray_nr,
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

	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (arrldx_usa == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (arrl_fd == 1) {
	    if (fixedmult != 0) {
		totalmults = fixedmult;
	    } else {
		totalmults = 1;
	    }
	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);
	}

	if (universal == 1 && country_mult == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (universal == 1 && multlist == 1 && arrlss != 1) {

	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Total: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (pacc_pa_flg == 1) {

	    totalzones = 0;
	    totalcountries = 0;

	    for (n = 0; n <= 5; n++) {
		totalcountries = totalcountries + countryscore[n];
	    }
	    totalmults = totalcountries;

	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (wysiwyg_once == 1) {

	    totalmults = multarray_nr;
	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
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
	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
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
	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 40, "Pts: %d  Mul: %d Score: %d", total,
		     totalmults, totalmults * total);

	}

	if (wpx == 1) {		/* wpx */
	    mvprintw(20, 40, "                                   ");
	    mvprintw(21, 40, "                                   ");
	    mvprintw(21, 42, "PX:%d  Pts:%d  Score: %d", nr_of_px, total,
		     (total * (nr_of_px)));
	    mvhline(19, 40, ACS_HLINE, 35);

	    if (nr_of_px >= 2) {
		p = (qsonum - 1) / (nr_of_px);
		q = 10 * ((qsonum - 1) - (p * (nr_of_px)));
		r = q / (nr_of_px);
	    } else {
		p = 1;
		r = 0;
	    }
	}

	nicebox(16, 39, 5, 35, "QSO's");
	mvprintw(16, 60, "%s", whichcontest);

	if ((cqww == 1) || (wpx == 1) || (arrldx_usa == 1) || (pacc_pa_flg == 1) || (wysiwyg_once == 1) || (universal == 1)) {	/* cqww or wpx */
	    if (wpx == 1)
		totalmults = nr_of_px;
	    /** \todo fix calculation of Q/M */
	    if (totalmults >= 2)
		p = (total / totalmults);
	    else
		p = 1;

	    if ((l10 = last10()) >= 1)
		mvprintw(22, 50, "Q/M %d  Rate %d ", p, (60 * 10) / l10);

	    if (wpx == 1) {
		if (minute_timer > 0)
		    mvprintw(22, 70, "%d", minute_timer);
	    }
	}

	attron(COLOR_PAIR(COLOR_MAGENTA) | A_STANDOUT);

	switch (bandinx) {
	case BANDINDEX_160:
	    {
		mvprintw(17, 47, "160");
		break;
	    }
	case BANDINDEX_80:
	    {
		mvprintw(17, 52, " 80");
		break;
	    }
	case BANDINDEX_40:
	    {
		mvprintw(17, 57, " 40");
		break;
	    }
	case BANDINDEX_30:
	    {
		mvprintw(17, 62, " 30");
		break;
	    }
	case BANDINDEX_20:
	    {
		mvprintw(17, 62, " 20");
		break;
	    }
	case BANDINDEX_17:
	    {
		mvprintw(17, 67, " 17");
		break;
	    }
	case BANDINDEX_15:
	    {
		mvprintw(17, 67, " 15");
		break;
	    }
	case BANDINDEX_12:
	    {
		mvprintw(17, 72, " 12");
		break;
	    }
	case BANDINDEX_10:
	    {
		mvprintw(17, 72, " 10");
		break;
	    }

	}

	printcall();

    }
//      r_multiplierinfo();

    return (0);
}

/*  --------------------------------------------------------------  */
int r_multiplierinfo(void)
{

    extern int use_rxvt;
    extern int arrlss;
    extern char mults[MAX_MULTS][12];
    extern GPtrArray *mults_possible;

    int j, vert, hor, cnt, found;
    char mprint[50];
    char chmult[4];

    if (arrlss == 1) {
	cnt = 0;

	if (use_rxvt == 0)
	    attron(COLOR_PAIR(COLOR_CYAN) | A_BOLD | A_STANDOUT);
	else
	    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	for (j = 2; j < 8; j++)
	    mvprintw(j, 42, "                                     ");

	for (vert = 2; vert < 8; vert++) {
	    if (cnt >= mults_possible->len)
		break;

	    for (hor = 11; hor < 20; hor++) {
		if (cnt >= mults_possible->len)
		    break;

		strcpy(mprint, g_ptr_array_index(mults_possible, cnt));
		strcat(mprint, " ");
		mprint[4] = '\0';

		found = 0;
		for (j = 0; j < multarray_nr; j++) {
		    strncpy(chmult, g_ptr_array_index(mults_possible, cnt), 4);
		    if (strlen(chmult) == 2)
			strcat(chmult, " ");

		    if (strcmp(mults[j], chmult) == 0) {
			found = 1;
			mprint[0] = '\0';
		    }
		}

		if (found != 1) {
		    mprint[3] = '\0';

		    if (use_rxvt == 0)
			attron(COLOR_PAIR(COLOR_CYAN) | A_BOLD |
			       A_STANDOUT);
		    else
			attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

		    if ((strlen(mprint) > 1)
			&& (strcmp(mprint, "W ") != 0))
			mvprintw(vert, (hor * 4) - 1, "%s ", mprint);

		} else
		    hor--;

		cnt++;

	    }
	}
    }

    nicebox(1, 41, 6, 37, "Sections");
    refresh();
    refresh();

    return (0);

}
