/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2019 Thomas Beierlein <dl1jbe@darc.de>
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
/* ------------------------------------------------------------------------
*    util functions to work with lines from log
*
---------------------------------------------------------------------------*/
#include <ctype.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bands.h"

/* for the following code we assume that we have well formatted log lines,
 * which has to be checked separately if needed */

/** check if logline is only a comment */
bool log_is_comment(const char *buffer) {

    if (buffer[0] != ';')
	return 0;
    else
	return 1;
}


/** read bandindex from logline */
int log_get_band(const char *logline) {

    int nr = 0;
    char band[4];

    g_strlcpy(band, logline, 4);

    nr = atoi(band);

    return bandnr2index(nr);
}


/** read mode from logline
 * -1 if no recognized mode */
int log_get_mode(const char *logline) {
    if (strncasecmp("CW ", logline + 3, 3) == 0) {
	return CWMODE;
    }
    if (strncasecmp("SSB", logline + 3, 3) == 0) {
	return SSBMODE;
    }
    if (strncasecmp("DIG", logline + 3, 3) == 0) {
	return DIGIMODE;
    }
    return -1;
}

/** read points from logline */
int log_get_points(const char *logline) {
    char tmpbuf[3];

    g_strlcpy(tmpbuf, logline + 76, 3);
    return atoi(tmpbuf);
}

