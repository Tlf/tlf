/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *			   2011 Thomas Beierlein <tb@forth-ev.de>
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
 *	add call area to list for one band
 *
 *--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "addmult.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "setcontest.h"
#include "tlf_curses.h"
#include "utils.h"
#include "bands.h"


GPtrArray *mults_possible;

enum { ALL_BAND, PER_BAND };


int addmult(void) {
    int found = 0;
    int i;
    int matching_len = 0, idx = -1;
    char *stripped_comment;

    shownewmult = -1;

    stripped_comment = strdup(comment);
    g_strchomp(stripped_comment);

    // --------------------------- arrlss ------------------------------------
    if (CONTEST_IS(ARRL_SS)) {

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < mults_possible->len; i++) {
	    int len = get_matching_length(ssexchange, i);
	    if (len > matching_len) {
		matching_len = len;
		idx = i;
	    }
	}

	if (idx >= 0) {
	    remember_multi(get_mult(idx), bandinx, ALL_BAND);
	}
    }

    // ---------------------------serial + section ---------------------------
    if ((serial_section_mult == 1) || (sectn_mult == 1)) {

	/* is it a possible mult? */
	for (i = 0; i < mults_possible->len; i++) {
	    if (get_matching_length(ssexchange, i) == strlen(ssexchange)) {
		idx = i;
		break;
	    }
	}

	if (idx >= 0) {
	    shownewmult =
		remember_multi(get_mult(idx), bandinx, PER_BAND);
	}
    }

    // --------------------------- section_mult_once--------------------------
    if (sectn_mult_once == 1) {

	/* is it a possible mult? */
	for (i = 0; i < mults_possible->len; i++) {
	    if (get_matching_length(ssexchange, i) == strlen(ssexchange)) {
		idx = i;
		break;
	    }
	}

	if (idx >= 0) {
	    shownewmult =
		remember_multi(get_mult(idx), bandinx, ALL_BAND);
	}
    }

    // ------------------------------- section ----------------------------
    if ((dx_arrlsections == 1) &&
	    ((countrynr == w_cty) || (countrynr == ve_cty))) {

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < mults_possible->len; i++) {
	    int len = get_matching_length(ssexchange, i);
	    if (len > matching_len) {
		matching_len = len;
		idx = i;
	    }
	}

	if (idx >= 0) {
	    shownewmult =
		remember_multi(get_mult(idx), bandinx, PER_BAND);
	}
    }

    // --------------------wysiwyg----------------
    if (wysiwyg_once == 1) {
	shownewmult = remember_multi(stripped_comment, bandinx, ALL_BAND);
    }

    if (wysiwyg_multi == 1) {
	shownewmult = remember_multi(stripped_comment, bandinx, PER_BAND);
    }

    if (serial_grid4_mult == 1) {
	section[4] = '\0';
	shownewmult = remember_multi(section, bandinx, PER_BAND);
    }

    /* -------------- unique call multi -------------- */
    if (unique_call_multi == UNIQUECALL_ALL) {
	shownewmult = remember_multi(hiscall, bandinx, ALL_BAND);
    }

    if (unique_call_multi == UNIQUECALL_BAND) {
	shownewmult = remember_multi(hiscall, bandinx, PER_BAND);
    }

    free(stripped_comment);

    return (found);
}


/* -------------------------------------------------------------------*/

int addmult2(void) {
    int found = 0;
    int i;
    int matching_len = 0, idx = -1;
    char ssexchange[21];
    char stripped_comment[21];
    char multi_call[20];

    shownewmult = -1;

    // --------------------------- arrlss ------------------------------------
    if (CONTEST_IS(ARRL_SS)) {
	g_strlcpy(ssexchange, lan_logline + 54, 21);

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < mults_possible->len; i++) {
	    int len = get_matching_length(ssexchange, i);
	    if (len > matching_len) {
		matching_len = len;
		idx = i;
	    }
	}

	if (idx >= 0) {
	    remember_multi(get_mult(idx), bandinx, ALL_BAND);
	}
    }

    // --------------------wysiwyg----------------
    if (wysiwyg_once == 1) {
	g_strlcpy(stripped_comment, lan_logline + 54, 15);
	g_strchomp(stripped_comment);

	shownewmult = remember_multi(stripped_comment, bandinx, ALL_BAND);
    }

    if (wysiwyg_multi == 1) {
	g_strlcpy(stripped_comment, lan_logline + 54, 15);
	g_strchomp(stripped_comment);

	shownewmult = remember_multi(stripped_comment, bandinx, PER_BAND);
    }

    /* -------------- unique call multi -------------- */
    g_strlcpy(multi_call, lan_logline + 68, 10);
    g_strchomp(multi_call);

    if (unique_call_multi == UNIQUECALL_ALL) {
	shownewmult = remember_multi(multi_call, bandinx, ALL_BAND);
    }

    if (unique_call_multi == UNIQUECALL_BAND) {
	shownewmult = remember_multi(multi_call, bandinx, PER_BAND);
    }


    return (found);
}


/* lookup n-th position in list of possible mults and
 * return pointer to data structure */
possible_mult_t *get_mult_base(int n) {
    return (possible_mult_t *)g_ptr_array_index(mults_possible, n);
}

/* look up n-th position in list of possible mults and
 * return pointer to multname */
char *get_mult(int n) {
    return get_mult_base(n)->name;
}

/* return alias list on n-th position of possible mults */
GSList *get_aliases(int n) {
    return get_mult_base(n)->aliases;
}

/* return number of possible mults */
int get_mult_count(void) {
    return mults_possible->len;
}

/* get best matching lenght of name or aliaslist of mult 'n' in 'str' */
unsigned int get_matching_length(char *str, unsigned int n) {
    unsigned len = 0;

    if (strstr(str, get_mult(n)) != NULL) {
	len = strlen(get_mult(n));
    }

    for (int i = 0; i < g_slist_length(get_aliases(n)); i++) {
	char *tmp = g_slist_nth_data(get_aliases(n), i);
	if (strstr(str, tmp) != NULL) {
	    if (strlen(tmp) >= len)
		len = strlen(tmp);
	}
    }
    return len;
}


/* function to free mults_possible entries */
void free_possible_mult(gpointer data) {
    possible_mult_t *tmp = (possible_mult_t *)data;
    g_free(tmp -> name);	/* free the name of the multi */
    g_slist_free_full(tmp -> aliases, g_free);
    g_free(tmp);
}

/* compare functions to sort multi by aphabetic order  */
gint	cmp_size(char **a, char **b) {

    possible_mult_t *t1 = (possible_mult_t *)*a;
    possible_mult_t *t2 = (possible_mult_t *)*b;
    return g_strcmp0(t1->name, t2->name);
}


/* parse a mult line and add data to databse
 *
 * multline consists of either
 *   multiplier
 * or
 *   multplier:followed,by,comma,separated,list,of,aliases
 *
 * There may be more than one alias line for a multi, so add all aliases to
 * that multi */
void add_mult_line(char *line) {
    possible_mult_t *multi;
    gchar **list;
    char *mult = NULL;
    int index = -1;

    list = g_strsplit(line, ":", 2);
    mult = g_strstrip(list[0]);

    /* find mult in already defined ones */
    for (int i = 0; i < get_mult_count(); i++) {
	if (strcmp(get_mult(i), mult) == 0) {
	    index = i;
	    break;
	}
    }

    if (index == -1) {
	/* not found -> prepare new one */
	multi = g_new0(possible_mult_t, 1);
	multi->name = g_strdup(mult);
	multi->aliases = NULL;
	g_ptr_array_add(mults_possible, multi);
    } else
	/* else use existing one */
	multi = get_mult_base(index);

    if (list[1] != NULL) {	    /* parse aliases if present */
	gchar **aliaslist;
	aliaslist = g_strsplit(list[1], ",", 0);
	for (int i = 0; aliaslist[i] != NULL; i++) {
	    multi->aliases =
		g_slist_append(multi->aliases,
			       g_strdup(g_strstrip(aliaslist[i])));
	}
	g_strfreev(aliaslist);
    }

    g_strfreev(list);
}

/** loads possible multipliers from external file
 *
 * Read in the file named by 'multiplierlist' and interpret it as list
 * of possible multis.
 *
 * Lines starting with '#' will be interpreted as comment.
 *
 * Each line should contain at max one word. Leading and trailing whitespaces
 * will be stripped and the remaining string is remembered in 'mults_possible[]'
 * growing array. Empty lines will be dropped.
 *
 * \return number of loaded multipliers (nr of entries in mults_possible)
 * */
int init_and_load_multipliers(void) {

    FILE *cfp;
    char s_inputbuffer[186] = "";
    char *mults_location;

    if (mults_possible) {
	/* free old array if exists */
	g_ptr_array_free(mults_possible, TRUE);
    }
    mults_possible = g_ptr_array_new_with_free_func(free_possible_mult);


    if (strlen(multsfile) == 0) {
	return 0;
    }

    mults_location = find_available(multsfile);

    if ((cfp = fopen(mults_location, "r")) == NULL) {
	mvprintw(9, 0, "Error opening multiplier file %s.\n", multsfile);
	refreshp();
	sleep(5);
    }

    g_free(mults_location);

    if (cfp == NULL) {
	return 0;       // couldn't open file
    }

    while (fgets(s_inputbuffer, 85, cfp) != NULL) {

	/* strip leading and trailing whitespace */
	g_strstrip(s_inputbuffer);

	/* drop comments starting with '#' and empty lines */
	if (*s_inputbuffer == '#' || *s_inputbuffer == '\0') {
	    continue;
	}

	add_mult_line(s_inputbuffer);

    }

    fclose(cfp);

    /* do not rely on the order in the mult file but sort it here */
    g_ptr_array_sort(mults_possible, (GCompareFunc)cmp_size);

    return get_mult_count();
}


/** initialize mults scoring
 *
 * empties multis[] array, set the number of multis and multscore per band to 0.
 */
void init_mults() {
    int n;

    for (n = 0; n < MAX_MULTS; n++) {
	multis[n].name[0] = '\0';
	multis[n].band = 0;
    }

    nr_multis = 0;

    for (n = 0; n < NBANDS; n++)
	multscore[n] = 0;
}

/** register worked multiplier and check if its new
 *
 * Check if multiplier is already registered. If not make a new entry in
 * multis[] array and increment the total mults count 'nr_multis'.
 * Mark the mult as worked on the actual band. If it is a new band
 * increase the bandspecific 'multscore[band]'.
 *
 * \param multiplier  - the multiplier as a string
 * \param band	      - the bandindex we are on
 * \param show_new_band -  1 -> check also if new band
 * \return	      - index in mults[] array if new mult or new on band
 *			(-1 if multiplier is an empty string or not new)
 */
int remember_multi(char *multiplier, int band, int show_new_band) {
    /* search multbuffer in mults arry */
    int found = 0, i, index = -1;

    if (*multiplier == '\0')
	return -1;			/* ignore empty string */

    for (i = 0; i < nr_multis; i++) {
	/* already in list? */
	if (strcmp(multis[i].name, multiplier) == 0) {
	    found = 1;

	    /* new band? */
	    if ((multis[i].band & inxes[band]) == 0) {
		multis[i].band |= inxes[band];
		multscore[band]++;

		/* if wanted, show it as new band */
		if (show_new_band == PER_BAND)
		    index = i;
	    }

	    break;
	}
    }

    /* add new multi */
    if (found == 0) {
	index = nr_multis;		/* return index of new mult */

	strcpy(multis[nr_multis].name, multiplier);
	multis[nr_multis].band |= inxes[band];
	multscore[band]++;
	nr_multis++;
    }

    return index;
}
