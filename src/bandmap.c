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

#include "bandmap.h"
#include "tlf.h"
#include "searchcallarray.h"
#include "getctydata.h"
#include "showinfo.h"
#include "searchlog.h"
#include "onechar.h"

#include <math.h>
#include <glib.h>
#include <ncurses.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif

#define TOLERANCE 50

unsigned int bandcorner[NBANDS][2] =
{{ 1800000, 2000000 },
 { 3500000, 4000000 },
 { 7000000, 7300000 },
 { 10100000, 10150000 },
 { 14000000, 14350000 },
 { 18068000, 18168000 },
 { 21000000, 21450000 },
 { 24890000, 24990000 },
 { 28000000, 29700000 }};

unsigned int cwcorner[NBANDS] =
{ 1838000,
  3580000,
  7040000,
  10140000,
  14070000,
  18095000,
  21070000,
  24915000,
  28070000};

unsigned int ssbcorner[NBANDS] =
{ 1840000,
  3600000,
  7040000,
 10150000,
 14100000,
 18120000,
 21150000,
 24930000,
 28300000 };

pthread_mutex_t bm_mutex = PTHREAD_MUTEX_INITIALIZER;

/** \brief sorted list of DX all recent spots
 *
 * a simple sorted linked list should do for a first start */
GList *allspots = NULL;

/** \brief sorted list of filtered spots
 */
GPtrArray *spots;


bm_config_t bm_config = {
    1,	/* show all bands */
    1,  /* show all mode */
    1,  /* show dupes */
    1,	/* skip dupes during grab */
    900	/* default livetime */
};
short	bm_initialized = 0;

extern int bandinx;
extern int trxmode;
extern int thisnode;

extern int call_band[];		/** \todo should not be public */


/** \brief initialize bandmap
 *
 * initalize colors and data structures for bandmap operation
 */
void bm_init() {

    pthread_mutex_lock( &bm_mutex );

    init_pair (CB_NEW, COLOR_CYAN, COLOR_WHITE);
    init_pair (CB_NORMAL, COLOR_BLUE, COLOR_WHITE);
    init_pair (CB_DUPE, COLOR_BLACK, COLOR_WHITE);
    init_pair (CB_OLD, COLOR_YELLOW, COLOR_WHITE);
    init_pair (CB_MULTI, COLOR_WHITE, COLOR_BLUE);

    spots = g_ptr_array_sized_new( 128 );

    pthread_mutex_unlock( &bm_mutex );
}


/** \brief convert frequency to bandnumber 
 *
 * \return	bandnumber or -1 if not in any band
 */
int freq2band(unsigned int freq) {
    int i;

    for (i = 0; i < NBANDS; i++) {
	if (freq >= (unsigned int)bandcorner[i][0] && 
		    freq <= (unsigned int)bandcorner[i][1])
	    return i;	/* in actual band */
    }

    return -1;		/* not in any band */
}

/** \brief guess mode based on frequency
 *
 * \return CWMODE, DIGIMODE or SSBMODE
 */
int freq2mode(int freq, int band) {
    if (freq <= cwcorner[band])
	return CWMODE;
    else if (freq < ssbcorner[band])
    	return DIGIMODE;
    else
	return SSBMODE;
}



/** \brief add DX spot message to bandmap
 *
 * check if cluster message is a dx spot,
 * if so split it into pieces and insert in spot list */
void bm_add(char *s) {
    char *line;
    char node = ' ';

    line = g_strdup(s);
    if (strncmp(line, "DX de ", 6) != 0) {
	g_free(line);
	return;
    }

    if (strncmp(line + 6, "TLF-", 4) == 0)
	node = line[10];		/* get sending node id */

    bandmap_addspot (strtok(line+26, " \t") , (unsigned int)(atof(line+16)*1000) , node);
    g_free (line);
}


/* compare functions to search in list */
gint	cmp_call (spot* ldata, char *call) {

    return g_strcmp0(ldata->call, call);
}

gint	cmp_freq(spot *a, spot *b) {
    unsigned int af = a->freq;
    unsigned int bf = b->freq;

    if (af < bf)    return -1;
    if (af > bf)  return  1;
    return 0;
}

/** add a new spot to bandmap data 
 * \param call  	the call to add
 * \param freq 		on which frequency heard
 * \param node		reporting node
 */
void bandmap_addspot( char *call, unsigned int freq, char node) {
/* - if a spot on that band and mode is already in list replace old entry 
 *   with new one and set timeout to SPOT_NEW,
 *   otherwise add it to the list as new
 * - if other call on same frequency (with some TOLERANCE) replace it and set
 *   timeout to SPOT_NEW
 * - all frequencies from cluster are rounded to 100 Hz, 
 *   remember all other frequencies exactly
 *   but display only rounded to 100 Hz - sort exact
 */
    GList *found;
    int band;
    char mode;

    /* add only HF spots */
    if (freq > 30000000)
	return;

    band = freq2band(freq);
    if (band < 0)	/* no ham band */
	return;

    mode = freq2mode(freq, band);

    /* acquire bandmap mutex */
    pthread_mutex_lock( &bm_mutex );

    /* look if call is already on list in that mode and band */
    /* each call is allowed in every combination of band and mode
     * but only once */
    found = g_list_find_custom(allspots, call, (GCompareFunc)cmp_call);

    while (found != NULL) {

	/* if same band and mode -> found spot already in list */
	if (((spot *)found->data)->band == band &&
		((spot *)found->data)->mode == mode)
    	    break;

	found = g_list_find_custom(found->next, call, (GCompareFunc)cmp_call);
    }

    /* if already in list on that band and mode
     * 		-> set timeout to SPOT_NEW, and set new freq and reporting node
     *   		if freq has changed enough sort list anew by freq
     */
    if (found) {
    	((spot*)found->data)->timeout = SPOT_NEW;
	((spot*)found->data)->node = node;
	if (abs(((spot*)found->data)->freq - freq) > TOLERANCE) {
	    ((spot*)found->data)->freq = freq;
	    allspots = g_list_sort(allspots, (GCompareFunc)cmp_freq);
	}
    } 
    else {
    /* if not in list already -> prepare new entry and 
     * insert in list at correct freq */
	spot *entry = g_new(spot, 1);
	entry -> call = g_strdup(call);
	entry -> freq = freq;
	entry -> mode = mode;
	entry -> band = band;
	entry -> node = node;
	entry -> timeout = SPOT_NEW;

	allspots = g_list_insert_sorted( allspots, entry, (GCompareFunc)cmp_freq);
	/* lookup where it is */
	found = g_list_find(allspots, entry);
    }

    /* check that spot is unique on freq +/- TOLERANCE Hz, 
     * drop other entries if needed */
    if (found->prev && 
	(abs(((spot*)(found->prev)->data)->freq - freq) < TOLERANCE)) {
	spot *olddata;
	olddata = found->prev->data;
	allspots = g_list_remove_link(allspots, found->prev);
	g_free (olddata->call);
	g_free (olddata);
    }
    if (found->next && 
	(abs(((spot*)(found->next)->data)->freq - freq) < TOLERANCE)) {
	spot *olddata;
	olddata = found->next->data;
	allspots = g_list_remove_link(allspots, found->next);
	g_free (olddata->call);
	g_free (olddata);
    }

    pthread_mutex_unlock( &bm_mutex );
}


void bandmap_age() {
/*
 * go through all entries
 *   + decrement timeout 
 *   + set state to new, normal, aged or dead
 *   + if dead -> drop it from collection
 */

    GList *list = allspots;

    while (list) {
	spot *data = list->data;
	GList *temp = list;
	list = list->next;
	if (data->timeout)
	    data->timeout--;
	    if (data->timeout == 0) {
		allspots = g_list_remove_link( allspots, temp);
		g_free (data->call);
		g_free (data);
	    }
    }
}

int bm_ismulti( char * call) {
    return 0;
}


/** \todo should check band AND mode if already worked.... */

int bm_isdupe( char *call, int band ) {
    int found = -1;
    
    /* spot for warc bands are never dupes */
    if (!inxes[band])
	return 0;

    found = searchcallarray(call);
    
    if (found == -1)		/* new call */
	return 0;
 
    if (call_band[found] & inxes[band])
	return 1;
    else
	return 0;
}


void bm_show_info() {

    int curx, cury;

    getyx( stdscr, cury, curx);		/* remember cursor */

    /* show info field on the right */
    attrset(COLOR_PAIR(CB_DUPE)|A_BOLD);
    move(14,66);
    vline(ACS_VLINE,10);
    mvprintw( 17, 68, "Spots: %3d", g_list_length(allspots));

    mvprintw (19, 68, "bands: %s", bm_config.allband ? "all" : "own");
    mvprintw (20,68, "modes: %s", bm_config.allmode ? "all" : "own");
    mvprintw (21,68, "dupes: %s", bm_config.showdupes ? "yes" : "no");
    
    attrset(COLOR_PAIR(CB_NEW)|A_STANDOUT);
    mvprintw( 22 ,69, "MULTI");

    attrset(COLOR_PAIR(CB_NEW)|A_BOLD);
    printw( " NEW");

    attrset(COLOR_PAIR(CB_NORMAL));
    mvprintw( 23,67, "SPOT");

    attrset(COLOR_PAIR(CB_OLD));
    printw( " OLD");

    attrset(COLOR_PAIR(CB_DUPE)|A_BOLD);
    printw( " dupe");

    attroff (A_BOLD|A_STANDOUT);

    move(cury, curx);			/* reset cursor */
}


void bandmap_show() {
/*
 * display depending on filter state
 * - all bands on/off
 * - all mode  on/off
 * - dupes     on/off
 *
 * If more entries to show than place in window, show around current frequency
 *
 * mark entries according to age, source and worked state. Mark new multis
 * - new 	brigth blue
 * - normal	blue
 * - aged	black
 * - worked	small caps
 * - new multi	underlined
 * - self announced stations
 *   		small preceeding letter for reporting station
 *
 * maybe show own frequency as dashline in other color 
 * (maybee green highlighted)
 * - highligth actual spot if near its frequency 
 *
 * Allow selection of one of the spots (switches to S&P)
 * - Ctrl-G as known
 * - '.' and cursor plus 'Enter'
 * - Test mouseclick...
 *
 * '.' goes into map, shows help line above and supports
 * - cursormovement
 * - 'ESC' leaves mode
 * - 'Enter' selects spot
 * - 'B', 'D', 'M' switches filtering for band, dupes and mode on or off.
 */

    GList *list;
    spot *data;
    int cols = 0;
    int curx, cury;
    int bm_x, bm_y;
    int i,j;
    short dupe;

    if (!bm_initialized) {
	bm_init();
	bm_initialized = 1;
    }

    /* acquire mutex 
     * do not add new spots to allspots during
     * - aging and
     * - filtering
     * furthermore do not allow call lookup as long as
     * filter array is build anew */
    pthread_mutex_lock( &bm_mutex );

    bandmap_age();			/* age entries in bandmap */

    /* make array of spots to display
     * filter spotlist according to settings */

    if (spots) 
	g_ptr_array_free( spots, TRUE);		/* free array */

    spots = g_ptr_array_sized_new( 128 );	/* allocate new one */

    list = allspots;

    while (list) {
	data = list->data;

	/* if spot is allband or allmode is set or band or mode matches
	 * actual one than add it to the filtered 'spot' array
	 */

	dupe = bm_isdupe(data->call, data->band);

	if ((bm_config.allband || (data->band == bandinx)) && 
		(bm_config.allmode || (data->mode == trxmode)) &&
		(bm_config.showdupes || !dupe)) {
	
	    data -> dupe = dupe;
	    g_ptr_array_add( spots, data );
	}

	list = list->next;
    }

    pthread_mutex_unlock( &bm_mutex );


    /* afterwards display filtered list around own QRG +/- some offest 
     * (offset gets resest if we change frequency */

    /** \todo Auswahl des Display Bereiches */
    getyx( stdscr, cury, curx);		/* remember cursor */

    /* start in line 14, column 0 */
    bm_y = 14;
    bm_x = 0;

    /* clear space for bandmap */
    attrset(COLOR_PAIR(CB_DUPE)|A_BOLD);

    move(bm_y,0);			/* do not overwrite # frequency */
    for (j = 0; j < 67; j++)
	addch(' ');

    for (i = bm_y + 1; i < bm_y + 10; i++) {
	move (i,0);
	for (j = 0; j < 80; j++)
	    addch (' ');
    }

    bm_show_info();
    /** \fixme Darstellung des # Speichers */

    for (i = 0; i < spots->len; i++) 
    {
	data = g_ptr_array_index( spots, i );

	attrset(COLOR_PAIR(CB_DUPE)|A_BOLD);
	mvprintw (bm_y, bm_x, "%7.1f %c ", (float)(data->freq/1000.), 
		(data->node == thisnode ? '*' : data->node));

	if (data -> timeout > SPOT_NORMAL) 
	    attrset(COLOR_PAIR(CB_NEW)|A_BOLD);

	else if (data -> timeout > SPOT_OLD)
	    attrset(COLOR_PAIR(CB_NORMAL));

	else
	    attrset(COLOR_PAIR(CB_OLD));

	if (bm_ismulti(data->call))
	    attron(A_STANDOUT);

       if (data->dupe) {
	   if (bm_config.showdupes) {
	       attrset(COLOR_PAIR(CB_DUPE)|A_BOLD);
	       attroff(A_STANDOUT);
	       printw ("%-12s", g_ascii_strdown(data->call, -1));
	   }
	}
	else {
	    printw ("%-12s", data->call);
	}

	attroff (A_BOLD);

	bm_y++;
	if (bm_y == 24) {
	    bm_y = 14;
	    bm_x += 22;
	    cols++;
	    if (cols > 2)
		break;
	}
    }
    
    move(cury, curx);			/* reset cursor */

    refreshp();
}

/** allow control of bandmap features 
 */
void bm_menu()
{
    int curx, cury;
    char c = -1;
    int j;

    getyx( stdscr, cury, curx);		/* remember cursor */

    attrset( COLOR_PAIR(C_LOG) | A_STANDOUT );
    mvprintw( 13, 0, "  Toggle <B>and, <M>ode or <D>upes filter");
    printw(" | any other - leave");

    c = toupper( onechar());
    switch (c) {
	case 'B':	
	    bm_config.allband = 1 - bm_config.allband;
	    break;

	case 'M':
	    bm_config.allmode = 1 - bm_config.allmode;
	    break;

	case 'D':
	    bm_config.showdupes = 1 - bm_config.showdupes;
	    break;
    }
    bandmap_show();		/* refresh display */
    
    move (13,0);
    for (j = 0; j < 80; j++)
	addch (' ');

    move (cury, curx);
    refreshp();
}

spot *copy_spot(spot *data)
{
    spot *result = NULL;

    result = g_new(spot, 1);
    result -> call = g_strdup(data -> call);
    result -> freq = data -> freq;
    result -> mode = data -> mode;
    result -> band = data -> band;
    result -> node = data -> node;
    result -> timeout = data -> timeout;
    result -> dupe = data -> dupe;

    return result;
}

/** Search partialcall in filtered bandmap
 *
 * Lookup given partial call in the list of filtered bandmap spots.
 * Return a copy of the first entry found (means with teh lowest frequency).
 *
 * \param 	partialcall - part of call to look up
 * \return 	spot * structure with a copy of the found spot
 * 		or NULL if not found (You have to free the structure 
 * 		after use).
 */
spot *bandmap_lookup(char *partialcall)
{
    spot *result = NULL;

    if ((*partialcall != '\0') && (spots->len > 0))
    {
	int i;

	pthread_mutex_lock( &bm_mutex );

	for (i = 0; i < spots->len; i++) {
	    spot *data;
	    data = g_ptr_array_index( spots, i );

	    if (strstr(data->call, partialcall) != NULL) {

		/* copy data into a new Spot structure */
		result = copy_spot(data);

		break;
	    }
	}

	pthread_mutex_unlock( &bm_mutex );

    }
    return result;
}

/** Lookup next call in filtered spotlist
 *
 * Starting at given frequency lookup the array of filtered spots for
 * the next call up- or downwards.
 * Apply some headroom for frequency comparison (see problem with ORION rig
 * (Dec2011).
 * Returns a copy of the spot data or NULL if no such entry.
 *
 * \param 	upwards - lookup upwards if not 0
 * \param 	freq - frequency to start from
 *
 * \return 	spot * structure with a copy of the found spot
 * 		or NULL if not found (You have to free the structure 
 * 		after use).
 */

spot *bandmap_next(unsigned int upwards, unsigned int freq)
{
    spot *result = NULL;

    if (spots->len > 0) {
	int i;

	pthread_mutex_lock( &bm_mutex );

	if (upwards) {

	    for (i = 0; i < spots->len; i++) {
		spot *data;
		data = g_ptr_array_index( spots, i );

		if ((data->freq > freq + TOLERANCE/2) && 
			(!bm_config.skipdupes || data->dupe == 0)) {
		    /* copy data into a new Spot structure */
		    result = copy_spot(data);

		    break;
		}
	    }
	} else {
	    for (i = spots->len-1; i >= 0; i--) {
		spot *data;
		data = g_ptr_array_index( spots, i );

		if ((data->freq < freq - TOLERANCE/2) &&
			(!bm_config.skipdupes || data->dupe == 0)) {
		    /* copy data into a new Spot structure */
		    result = copy_spot(data);

		    break;
		}
	    }
	}
	pthread_mutex_unlock( &bm_mutex );

    }
    return result;
}

