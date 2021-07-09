/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2011, 2015 Thomas Beierlein <tb@forth-ev.de>
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


#ifndef _BANDMAP_H
#define _BANDMAP_H

#include <stdbool.h>
#include <hamlib/rig.h>
#include "tlf.h"

typedef struct {
    char 	*call;
    int 	freq;	/* freq in Hz */
    char 	mode;
    short 	band;
    char	node;
    unsigned int timeout;/* time (in seconds) left in bandmap */
    char 	dupe;	/* only used internal in bm_show() */
    int 	cqzone;	/* CQ zone */
    int 	ctynr;	/* Country nr */
    char 	*pfx; /* prefix */
} spot;

#define SPOT_NEW	(bm_config.livetime)
#define SPOT_NORMAL	(SPOT_NEW * 95) / 100
#define SPOT_OLD	(SPOT_NEW * 2)  / 3

typedef struct {
    short allband;
    short allmode;
    short showdupes;
    short skipdupes;
    short livetime;
    short onlymults;
} bm_config_t;

extern bm_config_t bm_config;

enum {
    CB_DUPE = 8,
    CB_OLD,
    CB_NORMAL,
    CB_NEW,
    CB_MULTI
};

/* free an allocated spot */
void free_spot(spot *data);

/*
 * write bandmap spots to a file
 */
void bmdata_write_file();

void bm_init();

void bm_add(char *s);

void bm_menu();

/** add a new spot to bandmap data
 * \param call  	the call to add
 * \param freq	 	on which frequency heard
 * \param reason	- new cluster spot
 * 			- local announcement (Ctrl-A)
 * 			- own cluster announcement (Ctrl-B)
 * 			- just worked in S&P
 */
void bandmap_addspot(char *call, freq_t freq, char node);
/*
 * - if call already on that band and mode replace old entry with new one and
 *   set age to 0 otherwise add it to collection
 * - if other call on same frequency (with some tolerance) replace it and set
 *   age to 0
 * - round all frequencies from cluster to 100 Hz, remember all other exactly
 *   but display only rounded to 100 Hz - sort exact
 */

void bandmap_age();
/*
 * - go through all entries
 *   + decrement timeout
 *   + set state to new, normal, aged or dead
 *   + if dead -> drop it from collection
 */

void bandmap_show();
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
 *   		small preceeding letter for repoting station
 *
 * maybe show own frequency as dashline in other color
 * (maybee green highlighted)
 * - highligth actual spot if near its frequency
 *
 * Allow selection of one of the spots (switches to S&P)
 * - Ctrl-G as known
 * - '.' and cursor plus 'Enter'
 *
 * '.' goes into map, shows help line above and supports
 * - cursormovement
 * - 'ESC' leaves mode
 * - 'Enter' selects spot
 * - 'B', 'D', 'M' switches filtering for band, dupes and mode on or off.
 */

spot *bandmap_lookup(char *partialcall);

spot *bandmap_next(bool upwards, freq_t freq);

void get_spot_on_qrg(char *dest, freq_t freq);

#endif
