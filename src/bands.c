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

const unsigned int bandcorner[NBANDS][2] = {
    { 1800000, 2000000 },	// band bottom, band top
    { 3500000, 4000000 },
    { 5250000, 5450000 },       // 5351500-5356500 worldwide
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
    5354000,
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
    5354000,
    7040000,
    10150000,
    14100000,
    18120000,
    21150000,
    24930000,
    28300000,
    0
};

/** Converts bandindex to bandmask */
int inxes[NBANDS] = \
{
    BAND160, BAND80, BAND60, BAND40, BAND30, BAND20, BAND17, BAND15, BAND12, BAND10,
    BANDOOB
};


/** Converts bandnumber to bandindex */
int bandnr2index(int nr) {
    switch (nr) {

	case 160:
	    return BANDINDEX_160;

	case 80:
	    return BANDINDEX_80;

	case 40:
	    return BANDINDEX_40;

	case 20:
	    return BANDINDEX_20;

	case 15:
	    return BANDINDEX_15;

	case 10:
	    return BANDINDEX_10;

	case 12:
	    return BANDINDEX_12;

	case 17:
	    return BANDINDEX_17;

	case 30:
	    return BANDINDEX_30;
	default:
	    return BANDINDEX_OOB;
    }
}


/* converts bandindex to bandnumber */
static int bandnr[NBANDS] =
{ 160, 80, 40, 30, 20, 17, 15, 12, 10, 0 };

int bandindex2nr(int index) {
    return bandnr[index];
}


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
