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


#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include "bandmap.h"
#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "searchcallarray.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "getctydata.h"
#include "dxcc.h"
#include "initial_exchange.h"
#include "bands.h"

#define TOLERANCE 100 		/* spots with a QRG +/-TOLERANCE
				   will be counted as the same QRG */

#define SPOT_COLUMN_WIDTH 22
#define SPOT_FREQ_WIDTH 7
#define SPOT_CALL_WIDTH SPOT_COLUMN_WIDTH-SPOT_FREQ_WIDTH-4
				/* 3 space before and 1 after call */


const unsigned int bandcorner[NBANDS][2] = {
    { 1800000, 2000000 },	// band bottom, band top
    { 3500000, 4000000 },
    { 7000000, 7300000 },
    { 10100000, 10150000 },
    { 14000000, 14350000 },
    { 18068000, 18168000 },
    { 21000000, 21450000 },
    { 24890000, 24990000 },
    { 28000000, 29700000 },
    {        0,        0 }
};

const unsigned int cwcorner[NBANDS] = {
    1838000,
    3580000,
    7040000,
    10140000,
    14070000,
    18095000,
    21070000,
    24915000,
    28070000,
    0
};

const unsigned int ssbcorner[NBANDS] = {
    1840000,
    3600000,
    7040000,
    10150000,
    14100000,
    18120000,
    21150000,
    24930000,
    28300000,
    0
};

pthread_mutex_t bm_mutex = PTHREAD_MUTEX_INITIALIZER;

/** \brief sorted list of all recent DX spots
 */
GList *allspots = NULL;

/** \brief sorted list of filtered spots
 */
GPtrArray *spots;


bm_config_t bm_config = {
    1,	/* show all bands */
    1,  /* show all mode */
    1,  /* show dupes */
    1,	/* skip dupes during grab */
    900,/* default livetime */
    0  /* DO NOT show ONLY multipliers */
};
short	bm_initialized = 0;

extern float freq;
extern int trx_control;
extern int bandinx;
extern int trxmode;
extern char thisnode;
extern struct worked_t worked[];
extern int contest;
extern char whichcontest[];

char *qtc_format(char *call);

gint cmp_freq(spot *a, spot *b);

/*
 * write bandmap spots to a file
 */
void bmdata_write_file() {

    FILE *fp;
    spot *sp;
    GList *found;
    struct timeval tv;

    if ((fp = fopen(".bmdata.dat", "w")) == NULL) {
	attron(modify_attr(COLOR_PAIR(CB_DUPE) | A_BOLD));
	mvprintw(13, 29, "can't open bandmap data file!");
	refreshp();
	return;
    }

    gettimeofday(&tv, NULL);

    pthread_mutex_lock(&bm_mutex);

    found = allspots;
    fprintf(fp, "%d\n", (int)tv.tv_sec);
    while (found != NULL) {
	sp = found->data;
	fprintf(fp, "%s;%d;%d;%d;%c;%u;%d;%d;%d;%s\n",
		sp->call, sp->freq, sp->mode, sp->band,
		sp->node, (int)sp->timeout, sp->dupe, sp->cqzone,
		sp->ctynr, g_strchomp(sp->pfx));
	found = found->next;
    }

    pthread_mutex_unlock(&bm_mutex);

    fclose(fp);
}

/*
 * read bandmap spots from file, put them to allspots list
 */
void bmdata_read_file() {
    FILE *fp;
    struct timeval tv;
    int timediff, last_bm_save_time, fc;
    char line[50], *token;
    static int bmdata_parsed = 0;

    if ((fp = fopen(".bmdata.dat", "r")) != NULL && bmdata_parsed == 0) {
	bmdata_parsed = 1;
	timediff = 0;
	if (fgets(line, 50, fp)) {
	    sscanf(line, "%d", &last_bm_save_time);
	    gettimeofday(&tv, NULL);
	    timediff = (int)tv.tv_sec - last_bm_save_time;
	    if (timediff < 0)
		timediff = 0;

	    while (fgets(line, 50, fp)) {
		spot *entry = g_new0(spot, 1);
		fc = 0;
		token = strtok(line, ";");
		while (token != NULL) {
		    switch (fc) {
			case 0:		entry -> call = g_strdup(token);
			    break;
			case 1:		sscanf(token, "%d", &entry->freq);
			    break;
			case 2:		sscanf(token, "%hhd", &entry->mode);
			    break;
			case 3:		sscanf(token, "%hd", &entry->band);
			    break;
			case 4:		sscanf(token, "%c", &entry->node);
			    break;
			case 5:		sscanf(token, "%u", &entry->timeout);
			    break;
			case 6:		sscanf(token, "%hhd", &entry->dupe);
			    break;
			case 7:		sscanf(token, "%u", &entry->cqzone);
			    break;
			case 8:		sscanf(token, "%u", &entry->ctynr);
			    break;
			case 9:		entry->pfx = g_strdup(token);
			    break;
		    }
		    fc++;
		    token = strtok(NULL, ";");
		}
		if (entry->timeout > timediff) {
		    entry->timeout -= timediff;	/* remaining time */
		    allspots = g_list_insert_sorted(allspots, entry, (GCompareFunc)cmp_freq);
		} else {
		    g_free(entry);
		}
	    }
	}
	fclose(fp);
    }
}

/** \brief initialize bandmap
 *
 * initalize colors and data structures for bandmap operation
 */
void bm_init() {

    pthread_mutex_lock(&bm_mutex);

    init_pair(CB_NEW, COLOR_CYAN, COLOR_WHITE);
    init_pair(CB_NORMAL, COLOR_BLUE, COLOR_WHITE);
    init_pair(CB_DUPE, COLOR_BLACK, COLOR_WHITE);
    init_pair(CB_OLD, COLOR_YELLOW, COLOR_WHITE);
    init_pair(CB_MULTI, COLOR_WHITE, COLOR_BLUE);

    spots = g_ptr_array_sized_new(128);

    bmdata_read_file();

    pthread_mutex_unlock(&bm_mutex);
}


/** \brief convert frequency in Hz to bandindex
 *
 * \return	bandindex or BANDINDEX_OOB if not in any band
 */
int freq2band(unsigned int freq) {
    int i;

    for (i = 0; i < NBANDS; i++) {
	if (freq >= bandcorner[i][0] &&
		freq <= bandcorner[i][1])
	    return i;	/* in actual band */
    }

    return BANDINDEX_OOB;   /* not in any band (out of band) */
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

    bandmap_addspot(strtok(line + 26, " \t"),
		    (unsigned int)(atof(line + 16) * 1000), node);
    g_free(line);
}


/* compare functions to search in list */
gint	cmp_call(spot *ldata, char *call) {

    return g_strcmp0(ldata->call, call);
}

gint	cmp_freq(spot *a, spot *b) {
    unsigned int af = a->freq;
    unsigned int bf = b->freq;

    if (af < bf)  return -1;
    if (af > bf)  return  1;
    return 0;
}

/** add a new spot to bandmap data
 * \param call  	the call to add
 * \param freq 		on which frequency heard
 * \param node		reporting node
 */
void bandmap_addspot(char *call, unsigned int freq, char node) {
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
    dxcc_data *dxccdata;
    int dxccindex;
    int wi;
    char *lastexch;
    extern struct ie_list *main_ie_list;
    struct ie_list *current_ie;

    /* add only HF spots */
    if (freq > 30000000)
	return;

    band = freq2band(freq);
    if (band == BANDINDEX_OOB)  /* no ham band */
	return;

    mode = freq2mode(freq, band);

    /* acquire bandmap mutex */
    pthread_mutex_lock(&bm_mutex);

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
	((spot *)found->data)->timeout = SPOT_NEW;
	((spot *)found->data)->node = node;
	if (abs(((spot *)found->data)->freq - freq) > TOLERANCE) {
	    ((spot *)found->data)->freq = freq;
	    allspots = g_list_sort(allspots, (GCompareFunc)cmp_freq);
	}
    } else {
	/* if not in list already -> prepare new entry and
	 * insert in list at correct freq */
	spot *entry = g_new(spot, 1);
	entry -> call = g_strdup(call);
	entry -> freq = freq;
	entry -> mode = mode;
	entry -> band = band;
	entry -> node = node;
	entry -> timeout = SPOT_NEW;
	entry -> dupe = 0;	/* Dupe will be determined later. */

	lastexch = NULL;
	dxccindex = getctynr(entry->call);
	if (cqww == 1) {
	    // check if the callsign exists in worked list
	    wi = searchcallarray(call);
	    if (wi >= 0) {
		lastexch = g_strdup(worked[wi].exchange);
	    }

	    if (lastexch == NULL && main_ie_list != NULL) {
		current_ie = main_ie_list;

		while (current_ie) {
		    if (strcmp(call, current_ie->call) == 0) {
			lastexch = g_strdup(current_ie->exchange);
			break;
		    }
		    current_ie = current_ie->next;
		}
	    }
	}
	if (dxccindex > 0) {
	    dxccdata = dxcc_by_index(dxccindex);
	    entry -> cqzone = dxccdata->cq;
	    if (lastexch != NULL) {
		entry -> cqzone = atoi(lastexch);
		g_free(lastexch);
	    }
	    entry -> ctynr = dxccindex;
	    entry -> pfx = g_strdup(dxccdata->pfx);
	} else {
	    entry -> cqzone = 0;
	    entry -> ctynr = 0;
	    entry -> pfx = g_strdup("");
	}
	allspots = g_list_insert_sorted(allspots, entry, (GCompareFunc)cmp_freq);
	/* lookup where it is */
	found = g_list_find(allspots, entry);
    }

    /* check that spot is unique on freq +/- TOLERANCE Hz,
     * drop other entries if needed */
    if (found->prev &&
	    (abs(((spot *)(found->prev)->data)->freq - freq) < TOLERANCE)) {
	spot *olddata;
	olddata = found->prev->data;
	allspots = g_list_remove_link(allspots, found->prev);
	g_free(olddata->call);
	g_free(olddata->pfx);
	g_free(olddata);
    }
    if (found->next &&
	    (abs(((spot *)(found->next)->data)->freq - freq) < TOLERANCE)) {
	spot *olddata;
	olddata = found->next->data;
	allspots = g_list_remove_link(allspots, found->next);
	g_free(olddata->call);
	g_free(olddata->pfx);
	g_free(olddata);
    }


    pthread_mutex_unlock(&bm_mutex);
}


void bandmap_age() {
    /*
     * go through all entries
     *   + decrement timeout
     *   + set state to new, normal, aged or dead
     *   + if dead -> drop it from collection
     */

    pthread_mutex_lock(&bm_mutex);

    GList *list = allspots;

    while (list) {
	spot *data = list->data;
	GList *temp = list;
	list = list->next;
	if (data->timeout) {
	    data->timeout--;
	}
	if (data->timeout == 0) {
	    allspots = g_list_remove_link(allspots, temp);
	    g_free(data->call);
	    g_free(data->pfx);
	    g_free(data);
	}
    }

    pthread_mutex_unlock(&bm_mutex);
}


int bm_ismulti(char *call, spot *data, int band) {

    if (data != NULL && data->cqzone > 0 && data->ctynr > 0) {
	if (cqww == 1) {
	    if ((zones[data->cqzone] & inxes[band]) == 0
		    || (countries[data->ctynr] & inxes[band]) == 0) {
		return 1;
	    }
	}
    }
    return 0;
}


/** \todo should check band AND mode if already worked.... */

int bm_isdupe(char *call, int band) {
    int found = -1;
    struct t_qtc_store_obj *qtc_obj;

    /* spot for warc bands are never dupes */
    if (IsWarcIndex(band))
	return 0;

    found = searchcallarray(call);

    if (found == -1)		/* new call */
	return 0;

    if (qtcdirection > 0) {
	qtc_obj = qtc_get(call);
	if (qtc_obj->total > 0 && qtc_obj->total < 10) {
	    return 0;
	}
	if (qtc_obj->total == 0 && (qtc_obj->capable > 0)) {
	    return 0;
	}
    }
    if (worked[found].band & inxes[band]) {
	return 1;
    } else {
	return 0;
    }
}


void bm_show_info() {

    int curx, cury;

    getyx(stdscr, cury, curx);		/* remember cursor */

    /* show info field on the right */
    attrset(COLOR_PAIR(CB_DUPE) | A_BOLD);
    move(14, 66);
    vline(ACS_VLINE, 10);

    mvprintw(18, 68, "bands: %s", bm_config.allband ? "all" : "own");
    mvprintw(19, 68, "modes: %s", bm_config.allmode ? "all" : "own");
    mvprintw(20, 68, "dupes: %s", bm_config.showdupes ? "yes" : "no");
    mvprintw(21, 68, "onl.ml: %s", bm_config.onlymults ? "yes" : "no");

    attrset(COLOR_PAIR(CB_NEW) | A_STANDOUT);
    mvprintw(22, 69, "MULTI");

    attrset(COLOR_PAIR(CB_NEW) | A_BOLD);
    printw(" NEW");

    attrset(COLOR_PAIR(CB_NORMAL));
    mvprintw(23, 67, "SPOT");

    attrset(COLOR_PAIR(CB_OLD));
    printw(" OLD");

    attrset(COLOR_PAIR(CB_DUPE) | A_BOLD);
    printw(" dupe");

    attroff(A_BOLD | A_STANDOUT);

    move(cury, curx);			/* reset cursor */
}


/* helper function for bandmap display
 * mark entries according to age, source and worked state. Mark new multis
 * - new 	brigth blue
 * - normal	blue
 * - aged	brown
 * - worked	small caps */
void colorize_spot(spot *data) {
    if (data -> timeout > SPOT_NORMAL)
	attrset(COLOR_PAIR(CB_NEW) | A_BOLD);

    else if (data -> timeout > SPOT_OLD)
	attrset(COLOR_PAIR(CB_NORMAL));

    else
	attrset(COLOR_PAIR(CB_OLD));

    if (bm_ismulti(NULL, data, data->band)) {
	attrset(COLOR_PAIR(CB_NEW) | A_STANDOUT);
	attron(A_STANDOUT);
    }

    else if (data->dupe && bm_config.showdupes) {
	attrset(COLOR_PAIR(CB_DUPE) | A_BOLD);
	attroff(A_STANDOUT);
    }
}

/* helper function for bandmap display
 * convert dupes to lower case
 * add QTC flags for WAE contest
 */
char *format_spot(spot *data) {
    char *temp;
    char *temp2;

    if (qtcdirection > 0) {
	temp = qtc_format(data->call);
    } else
	temp = g_strdup(data->call);

    if (data->dupe && bm_config.showdupes) {
	temp2 = temp;
	temp = g_ascii_strdown(temp2, -1);
	g_free(temp2);
    }
    return temp;
}


/* helper function for bandmap display
 * shows formatted spot on actual cursor position
 */
void show_spot(spot *data) {
    attrset(COLOR_PAIR(CB_DUPE) | A_BOLD);
    printw("%7.1f %c ", (float)(data->freq / 1000.),
	   (data->node == thisnode ? '*' : data->node));

    char *temp = format_spot(data);
    colorize_spot(data);
    printw("%-12s", temp);
    g_free(temp);
}


/* helper function for bandmap display
 * shows spot on actual working frequency
 */
void show_spot_on_qrg(spot *data) {

    printw("%7.1f %c ", (data->freq / 1000.),
	   (data->node == thisnode ? '*' : data->node));

    char *temp = format_spot(data);
    printw("%-12s", temp);
    g_free(temp);
}


/* helper function for bandmap display
 * advance to next spot position
 */
void next_spot_position(int *y, int *x) {
    *y += 1;
    if (*y == 24) {
	*y = 14;
	*x += SPOT_COLUMN_WIDTH;
    }
}

/* helper function for bandmap display
 * provide center frequency for display
 *
 * If we have a rig online, read the frequency from it.
 * Otherwise calculate center frequency from band and mode
 * as middle value of the band/mode corners.
 */
float bm_get_center(int band, int mode) {
    float centerfrequency;

    if (trx_control)
	return freq;		/* return freq from rig */

    /* calculate center frequency for current band and mode */
    if (CWMODE == mode) {
	centerfrequency = (bandcorner[band][0] + cwcorner[band]) / 2.;
    } else if (SSBMODE == mode) {
	centerfrequency = (ssbcorner[band] + bandcorner[band][1]) / 2.;
    } else {
	centerfrequency = (cwcorner[band] + ssbcorner[band]) / 2.;
    }
    return centerfrequency / 1000.;
}


void bandmap_show() {
    /*
     * display depending on filter state
     * - all bands on/off
     * - all mode  on/off
     * - dupes     on/off
     *
     * If more entries to show than room in window, show around current frequency
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
     * - '.' and cursor plus 'Enter' \Todo
     * - Test mouseclick..           \Todo
     *
     * '.' goes into map, shows help line above and supports
     * - cursormovement
     * - 'ESC' leaves mode
     * - 'Enter' selects spot
     * - 'B', 'D', 'M', 'O' switches filtering for band, dupes, mode and multiPlier on or off.
     */

    GList *list;
    spot *data;
    int curx, cury;
    int bm_x, bm_y;
    int i, j;
    short dupe, multi;
    float centerfrequency;

    if (!bm_initialized) {
	bm_init();
	bm_initialized = 1;
    }

    /* acquire mutex
     * do not add new spots to allspots during
     * - aging and
     * - filtering
     * furthermore do not allow call lookup as long as
     * filtered spot array is build anew */

    pthread_mutex_lock(&bm_mutex);

    /* make array of spots to display
     * filter spotlist according to settings */

    if (spots)
	g_ptr_array_free(spots, TRUE);		/* free array */

    spots = g_ptr_array_sized_new(128);	/* allocate new one */

    list = allspots;

    while (list) {
	data = list->data;

	/* if spot is allband or allmode is set or band or mode matches
	 * actual one than add it to the filtered 'spot' array
	 * drop spots on WARC bands if in contest mode
	 */
	multi = bm_ismulti(NULL, data, data->band);
	dupe = bm_isdupe(data->call, data->band);

	if ((!contest || !IsWarcIndex(data->band))         &&
		(bm_config.allband || (data->band == bandinx)) &&
		(bm_config.allmode || (data->mode == trxmode)) &&
		(bm_config.showdupes || !dupe) &&
		(! bm_config.onlymults || multi)) {

	    data -> dupe = dupe;
	    g_ptr_array_add(spots, data);
	}

	list = list->next;
    }

    pthread_mutex_unlock(&bm_mutex);


    /* afterwards display filtered list around own QRG +/- some offest
     * (offset gets reset if we change frequency */

    getyx(stdscr, cury, curx);		/* remember cursor */

    /* start in line 14, column 0 */
    bm_y = 14;
    bm_x = 0;

    /* clear space for bandmap */
    attrset(COLOR_PAIR(CB_DUPE) | A_BOLD);

    move(bm_y, 0);			/* do not overwrite # frequency */
    for (j = 0; j < 67; j++)
	addch(' ');

    for (i = bm_y + 1; i < bm_y + 10; i++) {
	move(i, 0);
	for (j = 0; j < 80; j++)
	    addch(' ');
    }

    /* show info text */
    bm_show_info();

    /* split bandmap into two parts below and above current QRG.
     * Give both both parts equal size.
     * If there are less spots then reserved in the half
     * give the remaining room to the other half.
     *
     * These results in maximized usage of the bandmap display while
     * trying to keep the actual frequency in the center.
     */
    unsigned int below_qrg = 0;
    unsigned int on_qrg = 0;
    unsigned int startindex, stopindex;

    centerfrequency = bm_get_center(bandinx, trxmode);

    /* calc number of spots below your current QRG */
    for (i = 0; i < spots->len; i++) {
	data = g_ptr_array_index(spots, i);

	if (data->freq <= (centerfrequency * 1000 - TOLERANCE))
	    below_qrg++;
	else
	    break;
    }

    /* check if current QRG is on a spot */
    if (below_qrg < spots->len) {
	data = g_ptr_array_index(spots, below_qrg);

	if (!(data->freq > centerfrequency * 1000 + TOLERANCE))
	    on_qrg = 1;
    }

    /* calc the index into the spot array of the first spot to show */
    {
	unsigned int max_below;
	unsigned int above_qrg = spots->len - below_qrg - on_qrg;

	if (above_qrg < 14) {
	    max_below = 30 - above_qrg - 1;
	} else
	    max_below = 15;

	startindex = (below_qrg < max_below) ? 0 : (below_qrg - max_below);
    }

    /* calculate the index+1 of the last spot to show */
    stopindex  = (spots->len < startindex + 30 - (1 - on_qrg))
		 ? spots->len
		 : (startindex + 30 - (1 - on_qrg));

    /* correct calculations if we have no rig frequency to show */
    if (trx_control == 0) {
	if (on_qrg) {
	    on_qrg = 0;
	} else {
	    stopindex += 1;
	}
	if (spots->len < stopindex)
	    stopindex = spots->len;
    }

    /* show spots below QRG */
    for (i = startindex; i < below_qrg; i++) {
	move(bm_y, bm_x);
	show_spot(g_ptr_array_index(spots, i));
	next_spot_position(&bm_y, &bm_x);
    }

    /* show highlighted frequency marker or spot on QRG if rig control
     * is active */
    if (trx_control != 0) {
	move(bm_y, bm_x);
	attrset(COLOR_PAIR(C_HEADER) | A_STANDOUT);
	if (!on_qrg) {
	    printw("%7.1f   %s", centerfrequency,  "============");
	} else {
	    show_spot_on_qrg(g_ptr_array_index(spots, below_qrg));
	}
	next_spot_position(&bm_y, &bm_x);
    }

    /* show spots above QRG */
    for (i = below_qrg + on_qrg; i < stopindex; i++) {
	move(bm_y, bm_x);
	show_spot(g_ptr_array_index(spots, i));
	next_spot_position(&bm_y, &bm_x);
    }

    attroff(A_BOLD);
    move(cury, curx);			/* reset cursor */

    refreshp();
}


/** allow control of bandmap features
 */
void bm_menu() {
    int curx, cury;
    char c = -1;
    int j;

    getyx(stdscr, cury, curx);		/* remember cursor */

    attrset(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvprintw(13, 0, "  Toggle <B>and, <M>ode, <D>upes or <O>nly multi filter");
    printw(" | any other - leave");

    c = toupper(key_get());
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

	case 'O':
	    bm_config.onlymults = 1 - bm_config.onlymults;
	    break;
    }
    bandmap_show();		/* refresh display */

    move(13, 0);
    for (j = 0; j < 80; j++)
	addch(' ');

    move(cury, curx);
    refreshp();
}

spot *copy_spot(spot *data) {
    spot *result = NULL;

    result = g_new(spot, 1);
    result -> call = g_strdup(data -> call);
    result -> freq = data -> freq;
    result -> mode = data -> mode;
    result -> band = data -> band;
    result -> node = data -> node;
    result -> timeout = data -> timeout;
    result -> dupe = data -> dupe;
    result -> cqzone = data -> cqzone;
    result -> ctynr = data -> ctynr;
    result -> pfx = g_strdup(data -> pfx);

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
spot *bandmap_lookup(char *partialcall) {
    spot *result = NULL;

    if ((*partialcall != '\0') && (spots->len > 0)) {
	int i;

	pthread_mutex_lock(&bm_mutex);

	for (i = 0; i < spots->len; i++) {
	    spot *data;
	    data = g_ptr_array_index(spots, i);

	    if (strstr(data->call, partialcall) != NULL) {

		/* copy data into a new Spot structure */
		result = copy_spot(data);

		break;
	    }
	}

	pthread_mutex_unlock(&bm_mutex);

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

spot *bandmap_next(unsigned int upwards, unsigned int freq) {
    spot *result = NULL;

    if (spots->len > 0) {
	int i;

	pthread_mutex_lock(&bm_mutex);

	if (upwards) {

	    for (i = 0; i < spots->len; i++) {
		spot *data;
		data = g_ptr_array_index(spots, i);

		if ((data->freq > freq + TOLERANCE / 2) &&
			(!bm_config.skipdupes || data->dupe == 0)) {
		    /* copy data into a new Spot structure */
		    result = copy_spot(data);

		    break;
		}
	    }
	} else {
	    for (i = spots->len - 1; i >= 0; i--) {
		spot *data;
		data = g_ptr_array_index(spots, i);

		if ((data->freq < freq - TOLERANCE / 2) &&
			(!bm_config.skipdupes || data->dupe == 0)) {
		    /* copy data into a new Spot structure */
		    result = copy_spot(data);

		    break;
		}
	    }
	}
	pthread_mutex_unlock(&bm_mutex);

    }
    return result;
}

/*
 * copy string to buffer but truncate it to n characters
 * If truncated show it by replacing last two chars by '..'
 * The buffer has to be at least n+1 chars long.
 */
void str_truncate(char *buffer, char *string, int n) {
    if (strlen(string) > n) {
	g_strlcpy(buffer, string, n - 1);   	/* truncate to n-2 chars */
	strcat(buffer, "..");
    } else {
	g_strlcpy(buffer, string, n + 1); 	/* copy up to n chars */
    }
}

/*
 * format bandmap call output for WAE
 * - prepare and return a temporary string from call and number of QTC's
 *   (if any)
 */
char *qtc_format(char *call) {
    char tcall[15];
    char qtcflag;
    struct t_qtc_store_obj *qtc_temp_ptr;

    qtc_temp_ptr = qtc_get(call);
    qtcflag = qtc_get_value(qtc_temp_ptr);

    if (qtc_temp_ptr->total <= 0 && qtcflag == '\0') {
	str_truncate(tcall, call, SPOT_CALL_WIDTH);
    } else {
	str_truncate(tcall, call, SPOT_CALL_WIDTH - 2);
	sprintf(tcall + strlen(tcall), " %c", qtcflag);
    }
    return g_strdup(tcall);
}


/** Search filtered bandmap for a spot near the given frequency
 *
 * Return the call found at that frequency or NULL if no spot found
 *
 * \param 	dest - place to put the call in
 * \param 	freq - the frequency where to look for a spot
 */
void get_spot_on_qrg(char *dest, float freq) {

    *dest = '\0';

    if (spots->len > 0) {
	int i;

	pthread_mutex_lock(&bm_mutex);

	for (i = 0; i < spots->len; i++) {
	    spot *data;
	    data = g_ptr_array_index(spots, i);

	    if ((fabs(data->freq - freq * 1000) < TOLERANCE) &&
		    (!bm_config.skipdupes || data->dupe == 0)) {
		strcpy(dest, data->call);
		break;
	    }
	}

	pthread_mutex_unlock(&bm_mutex);

    }
}
