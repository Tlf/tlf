/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 			   2011 Thomas Beierlein <tb@forth-ev.de>
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
	 *      add call area to list for one band
	 *
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "addmult.h"
#include <glib.h>

#define MULTS_POSSIBLE(n) ((char *)g_ptr_array_index(mults_possible, n))

int inxes[NBANDS] =
    { BAND160, BAND80, BAND40, 0, BAND20, 0, BAND15, 0, BAND10 };

int addmult(void)
{
    int n, found = 0;
    int i, j, addarea = 0, ismult, multlen = 0;
    char *stripped_comment;

    shownewmult = -1;

    stripped_comment = strdup(comment);
    g_strchomp(stripped_comment);

    if (arrlss == 1) {	// mult for all bands   -------- arrlss --------------

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
		if (strncmp (mults[j], 
			    strstr(ssexchange, MULTS_POSSIBLE(i)), 
			    multlen) == 0) {
		    found = 1;
		    break;
		}
	    }

	    if (found == 0) {	/* not -> add it */
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

	    found = 0;

	    /* already worked? */
	    for (n = 0; n < multarray_nr; n++) {	// did we work it somewhere?

		if (strcmp(mults[n], MULTS_POSSIBLE(i)) == 0) {
		    found = 1;
		    break;
		}
	    }

	    if (found == 0) {	/* not -> add it */

		strcpy(mults[multarray_nr], MULTS_POSSIBLE(i));
		mult_bands[multarray_nr] =
		    mult_bands[multarray_nr] | inxes[bandinx];
		multarray_nr++;
		addarea = 1;
		shownewmult = multarray_nr - 1;

	    } else if ((found == 1) && ((mult_bands[n] & inxes[bandinx]) == 0))
	    {	// new on this band -> mark it...
		mult_bands[n] = mult_bands[n] | inxes[bandinx];
		addarea = 1;
		shownewmult = n;
	    }
	}
    }

    // ------------------------------- section ----------------------------

    if ((dx_arrlsections == 1)
	&& ((countrynr == w_cty) || (countrynr == ve_cty))) {

	char *ptr;		// local pointer

	ismult = 0;

	/* is it a possible mult? */
	if (mults_possible->len > 0) {
	    for (i = 0; i < mults_possible->len; i++) {	// check if valid mult.

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

	    found = 0;

	    /* already worked? */
	    for (n = 0; n < multarray_nr; n++) {	// did we work it somewhere?

		if (strcmp(mults[n], MULTS_POSSIBLE(i)) == 0) {
		    found = 1;
		    break;
		}
	    }

	    if (found == 0) {

		// no, store it.
		strcpy(mults[multarray_nr], MULTS_POSSIBLE(i));
		mult_bands[multarray_nr] =
		    mult_bands[multarray_nr] | inxes[bandinx];
		multarray_nr++;
		addarea = 1;
		shownewmult = multarray_nr - 1;

	    } else if ((found == 1) && ((mult_bands[n] & inxes[bandinx]) == 0)) {	// yes, mark it...
		mult_bands[n] = mult_bands[n] | inxes[bandinx];
		addarea = 1;
		shownewmult = n;
	    }
	}
    }

    if (wysiwyg_once == 1) {	// --------------------wysiwyg----------------

	found = 0;

	/* already worked? */
	for (n = 0; n < multarray_nr; n++) {
	    if (strcmp(mults[n], stripped_comment) == 0) {
		found = 1;
		break;
	    }
	}
	if (found == 0) {

	    strcpy(mults[multarray_nr], stripped_comment);
	    multarray_nr++;
	    addarea = 1;
	    shownewmult = n;
	}
    }

    if (wysiwyg_multi == 1 && strlen(stripped_comment) > 0) {

	found = 0;

	/* already worked? */
	for (n = 0; n < multarray_nr; n++) {
	    if (strcmp(mults[n], stripped_comment) == 0)  {
		found = 1;
		break;
	    }
	}

	if (found == 0) {
	    strcpy(mults[multarray_nr], stripped_comment);
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
    
    if (serial_grid4_mult == 1 && strlen(section) > 0) {

	found = 0;
	section[4] = '\0';

	/* already worked? */
	for (n = 0; n < multarray_nr; n++) { /** \todo check loop boundary */
	    if (strcmp(mults[n], section) == 0) {
		found = 1;
		break;
	    }
	}

	if (found == 0) {
	    strcpy(mults[multarray_nr], section);
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

		if ((strstr(ssexchange, MULTS_POSSIBLE(i)) != NULL)
		    && (strlen(MULTS_POSSIBLE(i)) > 1)) {

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
		multarray_nr++;
		strncpy(mults[multarray_nr],
			strstr(ssexchange, MULTS_POSSIBLE(i)), multlen);
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
    extern char multsfile[];

    FILE *cfp;
    char s_inputbuffer[186] = "";
    int count = 0;


    if (strlen(multsfile) == 0) {
	mvprintw(9, 0, "No multiplier file specified, exiting.. !!\n");
	refresh();
	sleep(5);
	exit(1);
    }

    if ((cfp = fopen(multsfile, "r")) == NULL) {
	mvprintw(9, 0, "Error opening multiplier file %s.\n", multsfile);
	refresh();
	sleep(2);
    } else {

	count = 0;

	while ( fgets(s_inputbuffer, 85, cfp) != NULL ) {

	    /* drop comments starting with '#' */
	    if (*s_inputbuffer == '#')
		continue;

	    /* strip leading and trailing whitespace */
	    g_strstrip( s_inputbuffer );
	    s_inputbuffer[9] = '\0';

	    /* drop empty lines */
	    if (s_inputbuffer == '\0')
		continue;

	    g_ptr_array_add(mults_possible, g_strdup(s_inputbuffer));

	    count++;
	}

	fclose(cfp);

    }

    return (count);
}

