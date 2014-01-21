/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2011 Thomas Beierlein <tb@forth-ev.de>
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

#include "foc.h"
#include "tlf.h"
#include "dxcc.h"
#include <ncurses.h>
#include <glib.h>

extern int foc;
extern int contest;
extern int showscore_flag;
extern int searchflg;
extern int total;
extern int countries[];

int g4foc_index; 		/* index of Gx4FOC in callarray */
int g4foc_count; 		/* how often did we work him */
int five_banders;
int six_banders;
int cntry;
int cont;

void count_56_banders();

/** Initialize settings for FOC contest */
void foc_init(void) {
    foc = 1;
    contest = 1;
    showscore_flag = 1;
    searchflg = 1;
}

/** \todo got_g4foc needs to be reset */

/** calculate score for last QSO
 *
 * Calculate the point score for the last qso. Each qso counts one
 * point on every band. Only exception are qsos with Gx4FOC/xxx which
 * counts two points.
 * \param call 	call of the other station
 * \return 	number of points given
 */
int foc_score(char *call) {

    if (g_regex_match_simple("^G(|[A-Z])4FOC(|/.*)", call,
	    G_REGEX_CASELESS, 0)) {
	return 2;
    }
    else {
	return 1;
    }
}


/* count worked contest bands */
int nr_of_bands(int x) {
    int i;
    int nr = 0;

    for (i = 0; i < NBANDS; i++) {
	if (x & inxes[i])
	    nr++;
    }
    return nr;
}


/** count 5/6banders
 *
 * count the number of stations worked on 5 or 6 band (including G4FOC)
 */
void count_56_banders() {
    int i, nr;

    extern int call_band[];
    extern int callarray_nr;

    five_banders = 0;
    six_banders = 0;

    for (i = 0; i < callarray_nr; i++) {
    	nr = nr_of_bands(call_band[i]);
	if ( nr == 5)
	    five_banders++;
	else if (nr == 6)
	    six_banders++;
    }
}


int search_g4foc_in_callarray(void) {
    extern int callarray_nr;
    extern char callarray[MAX_CALLS][20];

    int found = -1;
    int i;

    for (i = 0; i < callarray_nr; i++) {

	if (g_regex_match_simple("^G(|[A-Z])4FOC(|/.*)", callarray[i],
		G_REGEX_CASELESS, 0)) {
            found = i;
            break;
        }

    }

    return (found);
}


/* extra score for 5 or 6 bands for G4FOC */
int g4foc_score() {

    int points = g4foc_count;
    if (g4foc_count == 5)
	points += 10;
    else if (g4foc_count == 6)
	points += 15;

    return points;
}



/* count nr of countries worked on all bands */
int get_nr_cntry() {
    int cnt = 0;
    int i;

    for (i = 0; i < dxcc_count(); i++) {
	if (countries[i] != 0)
	    cnt++;
    }

    return cnt;
}


/* count number of continents worked on all bands */
int get_nr_cont() {
    extern int call_country[];
    extern int callarray_nr;

    GHashTable *cont;
    dxcc_data *data;
    int nr, i;

    cont = g_hash_table_new(g_str_hash, g_str_equal);

    for (i = 0; i < callarray_nr; i++) {
	data = dxcc_by_index(call_country[i]);

	g_hash_table_add(cont, data->continent);
    }

    nr = g_hash_table_size(cont);
    g_hash_table_destroy(cont);

    return nr;
}

/** calculate total score for FOC marathon
 *
 * \return number of points
 */
int foc_totalscore() {
    extern int call_band[];

    int points;

    /* first find Gx4FOC in call array and see how often we worked him */
    g4foc_index = search_g4foc_in_callarray();

    if (g4foc_index != -1)
	g4foc_count = nr_of_bands(call_band[g4foc_index]);
    else
	g4foc_count = 0;

    /* count countries and continents */
    cntry  = get_nr_cntry();
    cont   = get_nr_cont();

    /* count 5 and 6 banders and eventually correct for G4FOC */
    count_56_banders();
    if (g4foc_count == 5)
	five_banders--;
    else if (g4foc_count == 6)
	six_banders--;

    points = total - (2 * g4foc_count) +
	five_banders * 10 +
	six_banders * 15 +
	cntry * 2 +
	cont * 5 +
	g4foc_score() * 2;

    return points;
}

/** display scoring for FOC marathon */
void foc_show_scoring(int start_column) {

    int points = foc_totalscore();

#ifdef old_format
    mvprintw(4, start_column, "Ctry: %3d  Cont: %1d  G4FOC: %c",
	    cntry, cont, got_g4foc ? 'x' : '-');
    mvprintw(5, start_column, "  5b: %3d  6b: %3d  Score: %d",
	    five_banders, six_banders, points);
#else
    mvprintw(4, start_column, "%s", "QSO  Cty Cont   5b    6b FOC  Score");
    mvprintw(5, start_column, "%3d  %3d  %2d  %4d  %4d  %2d   %4d",
	    total - (2 * g4foc_count), cntry * 2, cont * 5,
	    five_banders * 10, six_banders * 15, 2 * g4foc_score(), points);
#endif

}

