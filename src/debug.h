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

#ifndef DEBUG_H
#define DEBUG_H

#include "stdarg.h"
#include "stdbool.h"

#define DEBUG_LOG "debug.txt"

enum debuglevel {
    TLF_DBG_NONE,
    TLF_DBG_ERR,
    TLF_DBG_WARN,
    TLF_DBG_INFO,
    TLF_DBG_DEBUG
};

/* Log Error, Warning, Info or Debug message to debuglog file if enabled */
#define TLF_LOG_ERR(fmt, ...)  do { \
	debug_log(TLF_DBG_ERR, fmt, ##__VA_ARGS__); \
    } while(0)
#define TLF_LOG_WARN(fmt, ...) do {\
	debug_log(TLF_DBG_WARN, fmt, ##__VA_ARGS__); \
    }while(0)
#define TLF_LOG_INFO(fmt, ...) do {\
	debug_log(TLF_DBG_INFO, fmt, ##__VA_ARGS__); \
    }while(0)
#define TLF_LOG_DEBUG(fmt, ...) do {\
	debug_log(TLF_DBG_DEBUG, fmt, ##__VA_ARGS__); \
    }while(0)

bool debug_is_active();
bool debug_init();
void debug_log (enum debuglevel lvl,
	const char *fmt,
	...);

#endif /* DEBUG_H */
