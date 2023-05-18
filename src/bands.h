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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* Definitions and functions related to manipulation of frequencies and
 * ham radio bands
 */

#ifndef BANDS_H
#define BANDS_H

#include "tlf.h"

#define IsWarcIndex(index) ((index == BANDINDEX_12) || \
			(index == BANDINDEX_17) || (index == BANDINDEX_30) || \
			(index == BANDINDEX_60))

/* Direction for switch to next band */
#define BAND_UP      +1
#define BAND_DOWN    -1

extern int inxes[NBANDS];  /**< conversion from BANDINDEX to BAND-mask */
extern unsigned int bandcorner[NBANDS][2];
extern unsigned int cwcorner[NBANDS];
extern unsigned int ssbcorner[NBANDS];

/** \brief converts bandnumber to bandindex
 *
 * \parameter   bandnumber
 * \return	bandindex or BANDINDEX_OOB if no ham radio band
 */
int bandnr2index(int nr);


/** \brief converts bandindex to bandnumber */
int bandindex2nr(int index);


/** Switch to next band
 *
 * Switch to next ham radio band up or down. Wrap around if lowest or highest
 * band reached.
 *
 * \param direction  BAND_UP or BAND_DOWN to choose next band up or down
 */
void next_band(int direction);

/** \brief convert frequency in Hz to bandindex
 *
 * \return	bandindex or BANDINDEX_OOB if not in any band
 */
int freq2bandindex(unsigned int freq);

/** \brief convert band to frequency in Hz
 *
 * \return	lowest frequency of the band or 0 if band not found
 */
int band2freq(int band);

#endif /* BANDS_H */
