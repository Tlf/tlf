/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2018 Thomas Beierlein <tb@forth-ev.de>
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

/* Definitions and functions related to manipulation of frequencies and
 * ham radio bands
 */

#include "bands.h"

/** Converts bandindex to bandmask */
int inxes[NBANDS] = \
    { BAND160, BAND80, BAND40, BAND30, BAND20, BAND17, BAND15, BAND12, BAND10,
        BANDOOB };

extern int bandinx;

void next_band(int direction) {
    bandinx += direction;

    if (bandinx < 0) {
        bandinx = BANDINDEX_OOB - 1;
    }

    if (bandinx >= BANDINDEX_OOB) {
        bandinx = 0;
    }
}



