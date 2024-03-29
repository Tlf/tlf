/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
 *               2015           Thomas Beierlein <tb@forth-ev.de>
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
 *     add prefix
 *
 *--------------------------------------------------------------*/


#include <string.h>
#include <glib.h>
#include "tlf.h"
#include "bands.h"

#define MAX_PFX_LEN 5

unsigned int nr_of_px = 0;
unsigned int nr_of_px_ab = 0;


struct {
    char pfx[MAX_PFX_LEN + 1];
    int bands;
} prefixes_worked[MAX_CALLS];

unsigned int pfxs_per_band[NBANDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/** lookup worked prefix and return index in prefixes_worked[] or
 * -1 if not found
 */
static int find_worked_pfx(char *prefix) {
    int found = -1;

    for (int i = 0; i < nr_of_px; i++) {
	if (strcmp(prefix, prefixes_worked[i].pfx) == 0) {
	    found = i;
	    break;
	}
    }

    return found;
}

bool pfx_is_new(char *prefix) {
    return (find_worked_pfx(prefix) == -1);
}


bool pfx_is_new_on(char *prefix, int bandindex) {
    int index;
    int worked_bands;

    index = find_worked_pfx(prefix);

    if (index == -1)
	return true;

    worked_bands = prefixes_worked[index].bands;
    if ((worked_bands & inxes[bandindex]) == 0)
	return true;

    return false;
}


int add_pfx(char *pxstr, unsigned int bandindex) {
    extern bool pfxmultab;
    int q = 0, found = 0, bandfound = 0;

    prefixes_worked[nr_of_px].pfx[0] = '\0';
    prefixes_worked[nr_of_px].bands = 0;

    for (q = 0; q <= nr_of_px; q++) {

	if (strcmp(pxstr, prefixes_worked[q].pfx) == 0) {
	    /* pfx already worked */
	    found = 1;
	    if (prefixes_worked[q].bands & inxes[bandindex]) {
		bandfound = 1;
	    } else {
		/* pfx new on band */
		prefixes_worked[q].bands |= inxes[bandindex];
		nr_of_px_ab++;
		pfxs_per_band[bandindex]++;
	    }
	    break;
	}
    }

    if (found != 1) {
	/* new pfx */
	g_strlcpy(prefixes_worked[nr_of_px].pfx, pxstr, MAX_PFX_LEN + 1);
	prefixes_worked[nr_of_px].bands |= inxes[bandindex];
	nr_of_px++;
	nr_of_px_ab++;
	pfxs_per_band[bandindex]++;
    }

    if (pfxmultab != 1) {
	return (found);
    } else {
	return (bandfound);
    }
}


unsigned int GetNrOfPfx_once() {
    return nr_of_px;
}


unsigned int GetNrOfPfx_multiband() {
    return nr_of_px_ab;
}


unsigned int GetNrOfPfx_OnBand(unsigned int bandindex) {
    if (bandindex < NBANDS)
	return pfxs_per_band[bandindex];
    else
	return 0;
}


void InitPfx() {
    int i;

    nr_of_px = 0;
    nr_of_px_ab = 0;

    for (i = 0; i < NBANDS; i++) {
	pfxs_per_band[i] = 0;
    }
}

