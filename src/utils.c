/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2021           Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

#include "getpx.h"

/* \brief find named file in actual directory or in share
 *
 * \returns filename of actual available file or NULL if not found
 *	    returned pointer has to be freed
 */
char *find_available(char *filename) {
    char *path;

    if (g_access(filename, R_OK) == 0) {
	path = g_strdup(filename);
    } else {
	path = g_strconcat(PACKAGE_DATA_DIR, G_DIR_SEPARATOR_S,
			   filename, NULL);
	if (g_access(path, R_OK) != 0) {
	    g_free(path);
	    path = g_strdup("");
	}
    }
    return path;
}

/* \brief get a substring from corrected call to repeat it
 *
 * \returns a substring based on the sent and corrected callsign
 */
void get_partial_callsign(char *call1, char *call2, char *partial) {

    size_t len1 = strlen(call1), len2 = strlen(call2);
    unsigned int len = (len1 < len2) ? len1 : len2;
    unsigned int i;
    unsigned int plen, plen1, plen2;
    int min = -1, max = -1;
    char tpartial[20];


    tpartial[0] = '\0';

    char *pfx1 = get_wpx_pfx(call1);
    char *pfx2 = get_wpx_pfx(call2);

    plen1 = strlen(pfx1);
    plen2 = strlen(pfx2);

    plen = (plen1 > plen2) ? plen1 : plen2;

    for (i = 0; i < len; i++) {
	if (call1[i] != call2[i]) {
	    if (min < 0) {
		min = i;
		max = i;
	    }
	    if (max < i) {
		max = i;
	    }
	}
    }

    // if all existing chars are the same
    // AB1CD / AB1CDE -> CDE
    // AB1CDE / AB1CD -> 1CD
    if (min == -1 && max == -1) {
	if (len2 < len1) {  // if the new call is shorter
	    plen--;         // include the last char of suffix
	}
	strncpy(tpartial, call2 + plen, len2 - plen + 1); // the full suffix
	tpartial[len2 - plen + 1] = '\0';
    } else {
	// if there is only 1 diff, and it's at the end
	// AB1CD / AB1CE -> CE
	if (min == max && max == len2 - 1) {
	    min--; // add the previous char too
	}
	if (len1 == len2) {
	    // if the mismatch is in the prefix
	    // AB1CD / AB2CD -> AB2
	    if (max <= plen - 1) {
		strncpy(tpartial, call2, plen);
		tpartial[plen] = '\0';
	    } else {
		strncpy(tpartial, call2 + min, len2 - min + 1);
		tpartial[len2 - min + 1] = '\0';
	    }
	} else {
	    strncpy(tpartial, call2 + min, len2 - min + 1);
	    tpartial[len2 - min + 1] = '\0';
	}
    }

    strcpy(partial, tpartial);
    free(pfx1);
    free(pfx2);

    return;
}
