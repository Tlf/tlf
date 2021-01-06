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


#ifndef GET_TIME_H
#define GET_TIME_H

#include <time.h>

#define DATE_FORMAT         "%d-%b-%y"  // 23-Apr-20
#define TIME_FORMAT         "%H:%M"
#define DATE_TIME_FORMAT    DATE_FORMAT " " TIME_FORMAT

extern long timecorr;

time_t get_time();
int get_minutes();

time_t format_time(char *buffer, size_t size, const char *format);
time_t format_time_with_offset(char *buffer, size_t size, const char *format,
			       double offset);

time_t parse_time(const char *buffer, const char *format);

#endif /* GET_TIME_H */
