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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>

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


