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
#include "tlf_curses.h"

#define MULTS_POSSIBLE(n) ((char *)g_ptr_array_index(mults_possible, n))

enum { ALL_BAND, PER_BAND };

/** Converts bandindex to bandmask */
int inxes[NBANDS] = \
{
    BAND160, BAND80, BAND40, BAND30, BAND20, BAND17, BAND15, BAND12, BAND10,
    BANDOOB
};

int addmult(void) {
    int found = 0;
    int i;
    int matching_len = 0, idx = -1;
    char *stripped_comment;

    shownewmult = -1;

    stripped_comment = strdup(comment);
    g_strchomp(stripped_comment);

    // --------------------------- arrlss ------------------------------------
    if (arrlss == 1) {

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < mults_possible->len; i++) {
	    if ((strstr(ssexchange, MULTS_POSSIBLE(i)) != NULL)
		    && (strlen(MULTS_POSSIBLE(i)) > 1)) {

		if (strlen(MULTS_POSSIBLE(i)) > matching_len) {
		    matching_len = strlen(MULTS_POSSIBLE(i));
		    idx = i;
		}
	    }
	}

	if (idx >= 0) {
	    remember_multi(MULTS_POSSIBLE(idx), bandinx, ALL_BAND);
	}
    }

    // ---------------------------serial + section ---------------------------
    if ((serial_section_mult == 1) || (sectn_mult == 1)) {

	/* is it a possible mult? */
	for (i = 0; i < mults_possible->len; i++) {
	    // check if valid mult....
	    if (strcmp(ssexchange, MULTS_POSSIBLE(i)) == 0) {
		idx = i;
		break;
	    }
	}

	if (idx >= 0) {
	    shownewmult =
		remember_multi(MULTS_POSSIBLE(idx), bandinx, PER_BAND);
	}
    }

    // ------------------------------- section ----------------------------
    if ((dx_arrlsections == 1) &&
	    ((countrynr == w_cty) || (countrynr == ve_cty))) {

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < mults_possible->len; i++) {
	    if (strstr(ssexchange, MULTS_POSSIBLE(i)) != NULL) {

		if (strlen(MULTS_POSSIBLE(i)) > matching_len) {
		    matching_len = strlen(MULTS_POSSIBLE(i));
		    idx = i;
		}
	    }
	}

	if (idx >= 0) {
	    shownewmult =
		remember_multi(MULTS_POSSIBLE(idx), bandinx, PER_BAND);
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
    if (arrlss == 1) {
	g_strlcpy(ssexchange, lan_logline + 54, 21);

	/* check all possible mults for match and remember the longest one */
	for (i = 0; i < mults_possible->len; i++) {
	    if ((strstr(ssexchange, MULTS_POSSIBLE(i)) != NULL)
		    && (strlen(MULTS_POSSIBLE(i)) > 1)) {

		if (strlen(MULTS_POSSIBLE(i)) > matching_len) {
		    matching_len = strlen(MULTS_POSSIBLE(i));
		    idx = i;
		}
	    }
	}

	if (idx >= 0) {
	    remember_multi(MULTS_POSSIBLE(idx), bandinx, ALL_BAND);
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


/* compare functions to sort multi by aphabetic order  */
gint	cmp_size(char **a, char **b) {

    return g_strcmp0(*a, *b);
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
    extern GPtrArray *mults_possible;
    extern char multsfile[];	// Set by parse_logcfg()

    FILE *cfp;
    char s_inputbuffer[186] = "";
    char mults_location[_POSIX_PATH_MAX * 2];	// 512 chars.  Larger?
    int count = 0;

    if (mults_possible) {
	/* free old array if exists */
	g_ptr_array_free(mults_possible, TRUE);
    }
    mults_possible = g_ptr_array_new_with_free_func(g_free);


    if (strlen(multsfile) == 0) {
	return 0;
    }

    // Check for mults file in working directory first
    if ((cfp = fopen(multsfile, "r")) == NULL) {
	// Check if multsfile is in installation directory
	if ((strlen(PACKAGE_DATA_DIR) + strlen(multsfile) + 1) <=
		(_POSIX_PATH_MAX * 2)) {
	    sprintf(mults_location, "%s%s%s", PACKAGE_DATA_DIR, "/", multsfile);

	    if ((cfp = fopen(mults_location, "r")) == NULL) {
		mvprintw(9, 0, "Error opening multiplier file %s.\n", multsfile);
		refreshp();
		sleep(5);
	    }
	} else {
	    mvprintw(9, 0, "Multiplier file path length exceeds buffer size of %d.\n",
		     _POSIX_PATH_MAX * 2);
	    refreshp();
	    sleep(5);
	}
    }

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

	s_inputbuffer[9] = '\0';

	g_ptr_array_add(mults_possible, g_strdup(s_inputbuffer));

	count++;
    }

    fclose(cfp);

    /* do not rely on the order in the mult file but sort it here */
    g_ptr_array_sort(mults_possible, (GCompareFunc)cmp_size);

    return count;
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
