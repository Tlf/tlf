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
    L_NONE,
    L_ERR,
    L_WARN,
    L_INFO,
    L_DEBUG
};

void handle_logging(enum log_lvl lvl, char *fmt, ...);


/* show Error, Warning or similar to user, but write into debuglog if enabled
 */
#define TLF_LOG_ERR(fmt, ...)  do { \
	debug_log(TLF_DBG_ERR, fmt, ##__VA_ARGS__); \
	handle_logging(L_ERR, fmt, ##__VA_ARGS__); \
	exit(EXIT_FAILURE); \
    } while(0)
#define TLF_LOG_WARN(fmt, ...) do {\
	debug_log(TLF_DBG_WARN, fmt, ##__VA_ARGS__); \
	handle_logging(L_WARN, fmt, ##__VA_ARGS__); \
    }while(0)
#define TLF_LOG_INFO(fmt, ...) do {\
	debug_log(TLF_DBG_INFO, fmt, ##__VA_ARGS__); \
	handle_logging(L_INFO, fmt, ##__VA_ARGS__); \
    }while(0)
#define TLF_LOG_DEBUG(fmt, ...) do {\
	debug_log(TLF_DBG_DEBUG, fmt, ##__VA_ARGS__); \
	handle_logging(L_DEBUG, fmt, ##__VA_ARGS__); \
    }while(0)


#endif /* ERR_UTILS_H */
