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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------------------
*    util functions to work with lines from log
*
---------------------------------------------------------------------------*/
#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <stdbool.h>

bool log_is_comment(const char *buffer);
int log_get_band(const char *logline);
int log_get_mode(const char *logline);
int log_get_points(const char *logline);
struct qso_t *parse_qso(char * buffer);
void free_qso(struct qso_t *ptr);

#endif
