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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

	/* ------------------------------------------------------------
	 *     add prefix
	 *
	 *--------------------------------------------------------------*/


#include <string.h>

#include "tlf.h"


int nr_of_px = 0;
int nr_of_px_ab = 0;

struct {
    char pfx[6];
    int bands;
} prefixes_worked[MAX_CALLS];

int pfxs_per_band[NBANDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

int add_pfx(char *pxstr)
{
    extern int pfxmultab;
    extern int bandinx;
    int q = 0, found = 0, bandfound = 0;

    prefixes_worked[nr_of_px].pfx[0] = '\0';
    prefixes_worked[nr_of_px].bands = 0;

    for (q = 0; q <= nr_of_px; q++) {

	if (strcmp(pxstr, prefixes_worked[q].pfx) == 0) {
	    /* pfx already worked */
	    found = 1;
	    if (prefixes_worked[q].bands & inxes[bandinx]) {
		bandfound = 1;
	    }
	    else {
		/* pfx new on band */
		prefixes_worked[q].bands |= inxes[bandinx];
		nr_of_px_ab++;
		pfxs_per_band[bandinx]++;
	    }
	    break;
	}
    }

    if (found != 1) {
	/* new pfx */
	strcpy(prefixes_worked[nr_of_px].pfx, pxstr);
	prefixes_worked[nr_of_px].bands |= inxes[bandinx];
	nr_of_px++;
	nr_of_px_ab++;
	pfxs_per_band[bandinx]++;
    }

    if (pfxmultab != 1) {
	return (found);
    }
    else {
	return (bandfound);
    }
}


int GetNrOfPfx_once() {
    return nr_of_px;
}


int GetNrOfPfx_multiband() {
    return nr_of_px_ab;
}


int GetNrOfPfx_OnBand(int bandindex) {
    if (bandindex < NBANDS)
	return pfxs_per_band[bandindex];
    else
	return 0;
}


void InitPfx() {
    int i;

    nr_of_px = 0;
    nr_of_px_ab = 0;

    for(i = 0; i < NBANDS; i++) {
        pfxs_per_band[i] = 0;
    }
}

