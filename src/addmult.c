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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define MULTS_POSSIBLE(n) ((char *)g_ptr_array_index(mults_possible, n))


/** Converts bandindex to bandmask */
int inxes[NBANDS] = \
    { BAND160, BAND80, BAND40, BAND30, BAND20, BAND17, BAND15, BAND12, BAND10 };

int addmult(void)
{
    int found = 0;
    int i, j, ismult, multlen = 0;
    char *stripped_comment;

    shownewmult = -1;

    stripped_comment = strdup(comment);
    g_strchomp(stripped_comment);

    // mult for all bands   -------- arrlss --------------
    if (arrlss == 1) {
	ismult = 0;

	/* is it a possible mult? */
	if (mults_possible->len > 0) {
	    for (i = 0; i < mults_possible->len; i++) {
		if ((strstr(ssexchange, MULTS_POSSIBLE(i)) != NULL)
		    && (strlen(MULTS_POSSIBLE(i)) > 1)) {

		    ismult = 1;
		    break;
		}
	    }
	}

	if (ismult != 0) {
	    found = 0;
	    multlen = strlen(MULTS_POSSIBLE(i));

	    /* already worked? */
	    for (j = 0; j < multarray_nr; j++) {
		if (strncmp(mults[j],
			    strstr(ssexchange, MULTS_POSSIBLE(i)),
			    multlen) == 0) {
		    found = 1;
		    break;
		}
	    }

	    if (found == 0) {
		/* not found, add it */
		strcpy(mults[multarray_nr], MULTS_POSSIBLE(i));
		multarray_nr++;
	    }
	}
    }

    // ---------------------------serial + section ---------------------------

    if ((serial_section_mult == 1) || (sectn_mult == 1)) {
	ismult = 0;

	/* is it a possible mult? */
	if (mults_possible->len > 0) {
	    for (i = 0; i < mults_possible->len; i++) {
		// check if valid mult....
		if (strcmp(ssexchange, MULTS_POSSIBLE(i)) == 0) {
		    ismult = 1;
		    break;
		}
	    }
	}

	if (ismult != 0) {
	    shownewmult = remember_multi(MULTS_POSSIBLE(i), bandinx, 1);
	}
    }

    // ------------------------------- section ----------------------------

    if ((dx_arrlsections == 1) &&
	((countrynr == w_cty) || (countrynr == ve_cty))) {
	char *ptr;		// local pointer

	ismult = 0;

	/* is it a possible mult? */
	if (mults_possible->len > 0) {
	    // check if valid mult.
	    for (i = 0; i < mults_possible->len; i++) {
		ptr = strstr(ssexchange, MULTS_POSSIBLE(i));

		if (ptr != NULL) {
		    ismult = 1;
		    multlen = strlen(MULTS_POSSIBLE(i));

		    if (strlen(MULTS_POSSIBLE(i)) == strlen(ptr))
			break;

		}
	    }
	}

	if (ismult != 0) {
	    shownewmult = remember_multi(MULTS_POSSIBLE(i), bandinx, 1);
	}
    }

    // --------------------wysiwyg----------------
    if (wysiwyg_once == 1) {
	shownewmult = remember_multi(stripped_comment, bandinx, 0);
    }

    if (wysiwyg_multi == 1 && strlen(stripped_comment) > 0) {
	shownewmult = remember_multi(stripped_comment, bandinx, 1);
    }

    if (serial_grid4_mult == 1 && strlen(section) > 0) {
	section[4] = '\0';
	shownewmult = remember_multi(section, bandinx, 1);
    }

    free(stripped_comment);

    return (found);
}


/* -------------------------------------------------------------------*/

int addmult2(void)
{
    int n, addarea = 0, found = 0;
    int i, j, ismult, multlen = 0;
    char ssexchange[21];

    shownewmult = -1;

    if (arrlss == 1) {		// mult for all bands
	ismult = 0;
	strncpy(ssexchange, lan_logline + 54, 20);

	if (mults_possible->len > 0) {
	    for (i = 0; i < mults_possible->len; i++) {
		if ((strstr(ssexchange, MULTS_POSSIBLE(i)) != NULL) &&
		    (strlen(MULTS_POSSIBLE(i)) > 1)) {

		    ismult = 1;
		    multlen = strlen(MULTS_POSSIBLE(i));
		    break;
		}
	    }
	}

	if (ismult != 0) {
	    for (j = 0; j < multarray_nr; j++) {
		if (strncmp
		    (mults[j], strstr(ssexchange, MULTS_POSSIBLE(i)),
		     multlen) == 0) {
		    found = 1;
		    break;
		}
	    }

	    if (found == 0) {
		/* not found, add it */
		strncpy(mults[multarray_nr],
			strstr(ssexchange, MULTS_POSSIBLE(i)), multlen);
		multarray_nr++;

		if (strlen(mults[multarray_nr]) == 2)
		    strcat(mults[multarray_nr], " ");
	    }
	}
    }

    if (wysiwyg_once == 1) {
	for (n = 0; n < multarray_nr; n++) {
	    if (strcmp(mults[n], comment) == 0) {
		found = 1;
		break;
	    }
	}

	if (found == 0) {
	    strcpy(mults[multarray_nr], comment);
	    multarray_nr++;
	    addarea = 1;
	    shownewmult = n;
	}
    }

    if ((wysiwyg_multi == 1) && (strlen(comment) > 0)) {
	for (n = 0; n < multarray_nr; n++) {
	    if (strcmp(mults[n], comment) == 0) {
		found = 1;
		break;
	    }
	}

	if (found == 0) {
	    strcpy(mults[multarray_nr], comment);
	    mult_bands[multarray_nr] =
		mult_bands[multarray_nr] | inxes[bandinx];
	    multarray_nr++;
	    addarea = 1;
	    shownewmult = multarray_nr - 1;
	} else if ((found == 1) && ((mult_bands[n] & inxes[bandinx]) == 0)) {
	    mult_bands[n] = mult_bands[n] | inxes[bandinx];
	    addarea = 1;
	    shownewmult = n;
	}
    }

    if (addarea == 1) {
	multscore[bandinx]++;
    }

    return (found);
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
int load_multipliers(void)
{
    extern GPtrArray *mults_possible;
    extern char multsfile[];	// Set by parse_logcfg()

    FILE *cfp;
    char s_inputbuffer[186] = "";
    char mults_location[_POSIX_PATH_MAX * 2];	// 512 chars.  Larger?
    int count = 0;


    if (strlen(multsfile) == 0) {
	mvprintw(9, 0, "No multiplier file specified, exiting.. !!\n");
	refreshp();
	sleep(5);
	exit(1);
    }

    // Check for mults file in working directory first
    if ((cfp = fopen(multsfile, "r")) == NULL) {
	// Check if multsfile is in installation directory
	if ((strlen(PACKAGE_DATA_DIR) + strlen(multsfile) + 1) <= (_POSIX_PATH_MAX * 2)) {
	    sprintf(mults_location, "%s%s%s", PACKAGE_DATA_DIR, "/", multsfile);

	    if ((cfp = fopen(mults_location, "r")) == NULL) {
		mvprintw(9, 0, "Error opening multiplier file %s.\n", multsfile);
		refreshp();
		sleep(5);
	    }
	} else {
	    mvprintw(9, 0, "Multiplier file path length exceeds buffer size of %d.\n", _POSIX_PATH_MAX * 2);
	    refreshp();
	    sleep(5);
	}
    }

    if (cfp) {
	while (fgets(s_inputbuffer, 85, cfp) != NULL) {
	    /* drop comments starting with '#' */
	    if (*s_inputbuffer == '#')
		continue;

	    /* strip leading and trailing whitespace */
	    g_strstrip(s_inputbuffer);
	    s_inputbuffer[9] = '\0';

	    /* drop empty lines */
	    if (*s_inputbuffer == '\0')
		continue;

	    g_ptr_array_add(mults_possible, g_strdup(s_inputbuffer));

	    count++;
	}

	fclose(cfp);
    }

    return (count);
}


/** initialize mults scoring
 *
 * empties mults[] and mult_bands[] arrays and set the number of
 * mults to 0.
 */
void init_mults()
{
    int n;

    for (n = 0; n < MAX_MULTS; n++) {
	mults[n][0] = '\0';
	mult_bands[n] = 0;
    }

    multarray_nr = 0;
}

/** register worked multiplier and check if its new
 *
 * Check if multiplier is already registered. If not make a new entry in
 * mults[] array and increment the total mults count 'multarray_nr'.
 * Mark the mult as worked on the actual band. If it is a new band
 * increase the bandspecific 'multscore[band]'.
 *
 * \param multiplier  - the multiplier as a string
 * \param band	      - the bandindex we are on
 * \param show_new_band -  1 -> check also if new band
 * \return	      - index in mults[] array if new mult or new on band
 *			(-1 if multiplier is an empty string or not new)
 */
int remember_multi(char *multiplier, int band, int show_new_band)
{
    /* search multbuffer in mults arry */
    int found = 0, i, index = -1;

    if (*multiplier == '\0')
	return -1;			/* ignore empty string */

    for (i = 0; i < multarray_nr; i++) {
	/* already in list? */
	if (strcmp(mults[i], multiplier) == 0) {
	    found = 1;

	    /* new band? */
	    if ((mult_bands[i] & inxes[band]) == 0) {
		mult_bands[i] |= inxes[band];
		multscore[band]++;

		/* if wanted, show it as new band */
		if (show_new_band)
		    index = i;
	    }

	    break;
	}
    }

    /* add new multi */
    if (found == 0) {
	index = multarray_nr;		/* return index of new mult */

	strcpy(mults[multarray_nr], multiplier);
	mult_bands[multarray_nr] |= inxes[band];
	multscore[band]++;
	multarray_nr++;
    }

    return index;
}
