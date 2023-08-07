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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------
 *	add call area to list for one band
 *
 *--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "addmult.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "setcontest.h"
#include "tlf_curses.h"
#include "utils.h"
#include "bands.h"


GPtrArray *mults_possible;

/*
 * \return	      - index in mults[] array if new mult or new on band
 *			-1 if not a (new) mult
 */
static int addmult_internal(struct qso_t *qso, bool check_only) {
    int mult_index = -1;
    int idx;
    char *stripped_comment;

    stripped_comment = strdup(qso->comment);
    g_strchomp(stripped_comment);

    // --------------------------- arrlss ------------------------------------
    if (CONTEST_IS(ARRL_SS)) {

	idx = get_exact_mult_index(qso->mult1_value);
	if (idx >= 0) {
	    remember_multi(get_mult(idx), qso->bandindex, MULT_ALL, check_only);
	    // NOTE: return value not used, new mult is not marked in log
	}
    }

    // ---------------------------serial + section ---------------------------
    else if (serial_section_mult || sectn_mult) {

	/* is it a mult? */
	idx = get_exact_mult_index(qso->mult1_value);
	if (idx >= 0) {
	    mult_index =
		remember_multi(get_mult(idx), qso->bandindex, MULT_BAND, check_only);
	}
    }

    // --------------------------- section_mult_once--------------------------
    else if (sectn_mult_once) {

	/* is it a mult? */
	idx = get_exact_mult_index(qso->mult1_value);
	if (idx >= 0) {
	    mult_index =
		remember_multi(get_mult(idx), qso->bandindex, MULT_ALL, check_only);
	}
    }

    // ------------------------------- section ----------------------------
    else if (dx_arrlsections && (countrynr == w_cty || countrynr == ve_cty)) {

	idx = get_exact_mult_index(qso->mult1_value);
	if (idx >= 0) {
	    mult_index =
		remember_multi(get_mult(idx), qso->bandindex, MULT_BAND, check_only);
	}
    }

    // --------------------wysiwyg----------------
    else if (wysiwyg_once) {
	mult_index = remember_multi(stripped_comment, qso->bandindex, MULT_ALL,
				    check_only);
    }

    else if (wysiwyg_multi) {
	mult_index = remember_multi(stripped_comment, qso->bandindex, MULT_BAND,
				    check_only);
    }

    /* -------------- unique call multi -------------- */
    else if (unique_call_multi != MULT_NONE) {
	mult_index = remember_multi(qso->call, qso->bandindex, unique_call_multi,
				    check_only);
    }

    /* ------------ grid mult (per band) ------------- */
    else if (serial_grid4_mult) {
	mult_index = remember_multi(qso->mult1_value, qso->bandindex, MULT_BAND,
				    check_only);
    }

    // -----------   generic: use mult1   -----------
    else if (generic_mult != MULT_NONE) {
	mult_index = remember_multi(qso->mult1_value, qso->bandindex, generic_mult,
				    check_only);
    }

    free(stripped_comment);
    return mult_index;
}

int addmult(struct qso_t *qso) {
    return addmult_internal(qso, false);
}

int check_mult(struct qso_t *qso) {
    return addmult_internal(qso, true); // check_only mode
}

/* -------------------------------------------------------------------*/

void addmult_lan(void) {
    int i;
    int matching_len = 0, idx = -1;
    char ssexchange[21];
    char stripped_comment[21];
    char multi_call[20];
    bool check_only = false;//FIXME param

    new_mult = -1;

    // --------------------------- arrlss ------------------------------------
    if (CONTEST_IS(ARRL_SS)) {
	g_strlcpy(ssexchange, lan_logline + 54, 21);

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < get_mult_count(); i++) {
	    int len = get_matching_length(ssexchange, i);
	    if (len > matching_len) {
		matching_len = len;
		idx = i;
	    }
	}

	if (idx >= 0) {
	    remember_multi(get_mult(idx), bandinx, MULT_ALL, check_only);
	}
    }

    // --------------------wysiwyg----------------
    if (wysiwyg_once) {
	g_strlcpy(stripped_comment, lan_logline + 54, 15);
	g_strchomp(stripped_comment);

	new_mult = remember_multi(stripped_comment, bandinx, MULT_ALL, check_only);
    }

    if (wysiwyg_multi) {
	g_strlcpy(stripped_comment, lan_logline + 54, 15);
	g_strchomp(stripped_comment);

	new_mult = remember_multi(stripped_comment, bandinx, MULT_BAND, check_only);
    }

    /* -------------- unique call multi -------------- */
    g_strlcpy(multi_call, lan_logline + 68, 10);
    g_strchomp(multi_call);

    if (unique_call_multi == MULT_ALL) {
	new_mult = remember_multi(multi_call, bandinx, MULT_ALL, check_only);
    }

    if (unique_call_multi == MULT_BAND) {
	new_mult = remember_multi(multi_call, bandinx, MULT_BAND, check_only);
    }

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
    return mults_possible != NULL ? mults_possible->len : 0;
}

/* get best matching length of name or aliaslist of mult 'n' in 'str' */
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

/* get mult index for exact match */
int get_exact_mult_index(char *str) {
    if (str == NULL) {
	return -1;
    }
    int len = strlen(str);
    if (len == 0) {
	return -1;
    }
    for (int i = 0; i < get_mult_count(); i++) {
	if (get_matching_length(str, i) == len) {
	    return i;
	}
    }
    return -1;
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

/* parse a mult line and add data to database
 *
 * multiline consists of either
 *   multiplier
 * or
 *   multiplier:followed,by,comma,separated,list,of,aliases
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
    char *s_inputbuffer = NULL;
    size_t s_inputbuffer_len = 0;
    char *mults_location;
    ssize_t read;

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

    while ((read = getline(&s_inputbuffer, &s_inputbuffer_len, cfp)) != -1) {
	if (s_inputbuffer_len > 0) {
	    if (errno == ENOMEM) {
		fprintf(stderr, "Error in: %s:%d", __FILE__, __LINE__);
		perror("RuntimeError: ");
		exit(EXIT_FAILURE);
	    }
	    /* strip leading and trailing whitespace */
	    g_strstrip(s_inputbuffer);

	    /* drop comments starting with '#' and empty lines */
	    if (*s_inputbuffer == '#' || *s_inputbuffer == '\0') {
		continue;
	    }

	    add_mult_line(s_inputbuffer);
	}
    }

    fclose(cfp);
    if (s_inputbuffer > 0)
	free(s_inputbuffer);
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

static pthread_mutex_t mult_mutex = PTHREAD_MUTEX_INITIALIZER;

/** register worked multiplier and check if its new
 *
 * Check if multiplier is already registered. If not make a new entry in
 * multis[] array and increment the total mults count 'nr_multis'.
 * Mark the mult as worked on the actual band. If it is a new band
 * increase the bandspecific 'multscore[band]'.
 *
 * \param multiplier  - the multiplier as a string
 * \param band	      - the bandindex we are on
 * \param mult_mode   - MULT_BAND -> check also if new band
 * \param check_only  - do not record mult, only check it
 * \return	      - index into mults[] array if new mult or on new band
 *			(-1 if multiplier is an empty string or not new)
 */
int remember_multi(char *multiplier, int band, int mult_mode, bool check_only) {
    /* search multbuffer in mults array */
    bool found = false;
    int index = -1;
    if (multiplier == NULL || *multiplier == '\0' || mult_mode == MULT_NONE)
	return -1;      /* ignore if empty string or disabled */

    pthread_mutex_lock(&mult_mutex);

    for (int i = 0; i < nr_multis; i++) {
	/* already in list? */
	if (strcmp(multis[i].name, multiplier) == 0) {
	    found = 1;

	    /* new band? check if mult is per band */
	    if ((multis[i].band & inxes[band]) == 0) {

		if (!check_only) {
		    // update band even if not strictly needed
		    multis[i].band |= inxes[band];
		}

		if (mult_mode == MULT_BAND) {
		    index = i;  // new band
		}
	    }

	    break;
	}
    }

    // found && index < 0: not new mult
    // found && index >= 0: existing mult on a new band
    // !found: new mult

    if (!found) {
	index = nr_multis;  /* not found, add new multi */
    }

    if (index >= 0 && !check_only) {
	if (!found) {   // new mult
	    strcpy(multis[index].name, multiplier);
	    nr_multis++;
	}
	multis[index].band |= inxes[band];
	multscore[band]++;
    }

    pthread_mutex_unlock(&mult_mutex);

    return index;
}
