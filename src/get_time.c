/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
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
*  First get the local time from the kernel, apply corrections
*  - TIME_OFFSET setting from logcfg.dat and
*  - time synchronistation from LAN.
*
*  All functions operate on UTC dates/times.
---------------------------------------------------------------------------*/

#include <time.h>
#include <string.h>

/*
 * returns current UTC as seconds since 1970-01-01
 */
time_t get_time() {
    extern int timeoffset;
    extern long timecorr;

    return time(NULL) + (timeoffset * 3600L) + timecorr;
}

int get_minutes() {
    time_t now = get_time();
    struct tm time_tm;
    gmtime_r(&now, &time_tm);
    return 60 * time_tm.tm_hour + time_tm.tm_min;
}


/*
 * formats now-offset using format into buffer
 * returns the formatted time (now-offset) as UTC seconds
 * note: timeoffset and offset are in hours
 */
time_t format_time_with_offset(char *buffer, size_t size, const char *format,
			       double offset) {

    time_t t = get_time() - offset * 3600L;

    struct tm time_tm;
    memset(&time_tm, 0, sizeof(struct tm));
    gmtime_r(&t, &time_tm);

    strftime(buffer, size, format, &time_tm);

    return t;
}

time_t format_time(char *buffer, size_t size, const char *format) {
    return format_time_with_offset(buffer, size, format, 0);
}

/*
 * parses buffer according to format
 * returns the parsed time as UTC seconds
 */
time_t parse_time(const char *buffer, const char *format) {
    struct tm time_tm;
    memset(&time_tm, 0, sizeof(time_tm));
    strptime(buffer, format, &time_tm);
    return timegm(&time_tm);
}

