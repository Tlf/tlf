/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2019 Thomas Beierlein <tb@forth-ev.de>, <dl1jbe@darc.de>
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

#ifndef ERR_UTILS_H
#define ERR_UTILS_H

enum log_lvl {
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERR
};

void handle_logging(enum log_lvl lvl, ...);

#define TLF_LOG_DEBUG(...) ( handle_logging(L_DEBUG, __VA_ARGS__) )
#define TLF_LOG_INFO(...) ( handle_logging(L_INFO, __VA_ARGS__) )
#define TLF_LOG_WARN(...) ( handle_logging(L_WARN, __VA_ARGS__) )
#define TLF_LOG_ERR(...)  do { \
	handle_logging(L_ERR, __VA_ARGS__); \
	exit(EXIT_FAILURE); \
    } while(0)


#endif /* ERR_UTILS_H */
