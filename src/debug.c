/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2025 Thomas Beierlein <tb@forth-ev.de>
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

#include "debug.h"
#include "get_time.h"
#include "globalvars.h"
#include "pthread.h"
#include "stdbool.h"

/* check if any debug level is active */
bool debug_is_active() {
    return (debuglevel > 0);
}

bool debug_init() {
    char debugbuffer[80];

    if (debug_is_active()) {
	/* write timestamp as start entry into debug log */
	FILE *fp = fopen(DEBUG_LOG, "a");
	if (fp == NULL) {
	    return FALSE;
	}

	format_time(debugbuffer, sizeof(debugbuffer),
		    "\nStarted %d/%m/%Y %H:%M\n");
	fputs(debugbuffer, fp);
	fclose(fp);
    }
    return TRUE;
}

/* check if message needs to be logged */
static bool needs_logging(enum debuglevel lvl) {
    return (lvl <= debuglevel);
}

void debug_log (enum debuglevel lvl,
	const char *fmt,
	...) {

    static pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER;
    char debugbuffer[160];
    va_list args;


    if (!needs_logging(lvl)) {
	return;
    }

    pthread_mutex_lock(&debug_mutex);

    FILE *fp = fopen(DEBUG_LOG, "a");
    if (fp == NULL) {	    /* ignore failure to write debug log */
	    pthread_mutex_unlock(&debug_mutex);
	    return;	    /* to not disturb logging activity */
    }

    format_time(debugbuffer, sizeof(debugbuffer), "%H:%M:%S ");
    va_start(args, fmt);
    char *msg = g_strdup_vprintf(fmt, args);
    va_end(args);

    fputs(debugbuffer, fp);
    switch (lvl) {
	case TLF_DBG_ERR: fputs("ERR ", fp);
	    break;
	case TLF_DBG_WARN: fputs("WRN ", fp);
	    break;
	case TLF_DBG_INFO: fputs("INF ", fp);
	    break;
	case TLF_DBG_DEBUG: fputs("DBG ", fp);
	    break;
	default:
	    break;
    }
    fputs(msg, fp);

    g_free(msg);
    fclose(fp);

    pthread_mutex_unlock(&debug_mutex);
}

