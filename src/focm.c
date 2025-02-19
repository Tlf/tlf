/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014 Thomas Beierlein <tb@forth-ev.de>
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


#include <glib.h>

#include "clear_display.h"
#include "dxcc.h"
#include "focm.h"
#include "getctydata.h"
#include "globalvars.h"
#include "initial_exchange.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "bands.h"

bool no_multi(spot * s) {
    return false;
}

struct pos {
    int column;
    int line;
};

int g4foc_index; 		/* index of Gx4FOC in callarray */
int g4foc_count; 		/* how often did we work him */
int five_banders;
int six_banders;
int cntry;
int cont;


int score_foc(struct qso_t *qso) {
    return foc_score(qso->call);
}

/** FOC contest configuration */
contest_config_t config_focm = {
    .id = FOCMARATHON,
    .name = "FOCMARATHON",
    .points = {
	.type = FUNCTION,
	.fn = score_foc
    },
    .is_multi = no_multi,
    .exchange_width = 5,    // 4 digit member number
};

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
			     G_REGEX_CASELESS, (GRegexMatchFlags)0)) {
	return 2;
    } else {
	return 1;
    }
}

/* display scoring */

/* count worked contest bands */
static int nr_of_bands(int x) {
    int i;
    int nr = 0;

    for (i = 0; i < NBANDS; i++) {
	/* if worked on band and band is not WARC band */
	if ((x & inxes[i]) && !IsWarcIndex(i))
	    nr++;
    }
    return nr;
}


/** count 5/6banders
 *
 * count the number of stations worked on 5 or 6 band (including G4FOC)
 */
static void count_56_banders() {
    int i, nr;

    five_banders = 0;
    six_banders = 0;

    for (i = 0; i < nr_worked; i++) {
	nr = nr_of_bands(worked[i].band);
	if (nr >= 5) 			/* sixbanders are also fivebanders */
	    five_banders++;
	if (nr == 6)
	    six_banders++;
    }
}


static int search_g4foc_in_callarray(void) {

    int found = -1;
    int i;

    for (i = 0; i < nr_worked; i++) {

	if (g_regex_match_simple("^G(|[A-Z])4FOC(|/.*)", worked[i].call,
				 G_REGEX_CASELESS, (GRegexMatchFlags)0)) {
	    found = i;
	    break;
	}

    }

    return (found);
}


/* count nr of countries worked on all bands */
static int get_nr_cntry() {
    int cnt = 0;
    int i;

    for (i = 0; i < dxcc_count(); i++) {
	if (countries[i] != 0)
	    cnt++;
    }

    return cnt;
}


/* count number of continents worked on all bands */
static int get_nr_cont() {

    GHashTable *cont;
    dxcc_data *data;
    int nr, i;

    cont = g_hash_table_new(g_str_hash, g_str_equal);

    for (i = 0; i < nr_worked; i++) {
	data = dxcc_by_index(worked[i].ctyinfo->dxcc_ctynr);

	g_hash_table_replace(cont, data->continent, data->continent);
    }

    nr = g_hash_table_size(cont);
    g_hash_table_destroy(cont);

    return nr;
}

/** calculate total score for FOC marathon
 *
 * \return number of points
 */
int foc_total_score() {

    int points;

    /* first find Gx4FOC in call array and see how often we worked him */
    g4foc_index = search_g4foc_in_callarray();

    if (g4foc_index != -1)
	g4foc_count = nr_of_bands(worked[g4foc_index].band);
    else
	g4foc_count = 0;

    /* count countries and continents */
    cntry  = get_nr_cntry();
    cont   = get_nr_cont();

    /* count 5 and 6 banders and eventually correct for G4FOC */
    count_56_banders();
    if (g4foc_count >= 5)
	five_banders++;
    if (g4foc_count == 6)
	six_banders++;

    points = total + 		/* total contains FOC qsos twice */
	     five_banders * 10 +
	     six_banders * 5 +
	     cntry * 2 +
	     cont * 5;

    return points;
}

/** display scoring for FOC marathon */
void foc_show_scoring(int start_column) {
    int points = foc_total_score();

    mvaddstr(4, start_column, " QSO   Cty  Cont    5b    6b  Score");
    mvprintw(5, start_column, "%4d   %3d    %2d  %4d  %4d   %4d",
	     total, cntry * 2, cont * 5,
	     five_banders * 10, six_banders * 5, points);

}


/* show needed countries */

/** build list of ´possible countries
 *
 * Scan initial exchange list and build a list of all countries
 * in that list. For each country check if we did already work that
 * country. Remember Cty and worked status in a GTree which
 * makes the entries unique and sorts it
 * \return 	pointer to the new GTree
 */
static GTree *build_country_list(struct ie_list *main_ie_list) {
    GTree *tree;
    int j;
    struct ie_list *list_head = main_ie_list;

    tree = g_tree_new_full((GCompareDataFunc)g_ascii_strcasecmp, NULL, g_free,
			   NULL);

    while (list_head) {
	j = getctydata(list_head->call);
	g_tree_insert(tree, g_strdup(dxcc_by_index(j)->pfx),
		      GINT_TO_POINTER(countries[j]));

	list_head = list_head->next;
    }

    return tree;
}


static gboolean show_it(gpointer key, gpointer val, gpointer data) {
    struct pos *pos = (struct pos *)data;

    if (GPOINTER_TO_INT(val) == 0) {
	standout();
	attron(COLOR_PAIR(C_INPUT));
    } else {
	standend();
	attron(COLOR_PAIR(C_HEADER));
    }

    mvprintw(pos->line, pos->column, "%-3s ", (char *)key);

    pos->column += 4;
    if (pos->column > 76) {
	pos->column = 0;
	pos->line ++;
	if (pos->line == 7) 		/* display full */
	    return TRUE; 		/* stop iterator */
    }

    return FALSE; 			/* do not stop until end of tree */
}


void foc_show_cty() {

    GTree *tree;
    struct pos pos;
    int l;


    tree = build_country_list(main_ie_list);

    attron(COLOR_PAIR(C_INPUT) | A_STANDOUT);
    for (l = 1; l < 6; l++)
	mvprintw(l, 0,
		 "                                                                                ");

    pos.line   = 1;
    pos.column = 0;

    g_tree_foreach(tree, (GTraverseFunc)show_it, &pos);

    mvaddstr(12, 29, "press a key...");
    refreshp();

    key_get();
    clear_display();

    g_tree_destroy(tree);
}

