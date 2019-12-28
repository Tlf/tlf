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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "err_utils.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include <glib.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

void handle_logging(enum log_lvl lvl, ...) {
    char *fmt;
    char *str;
    va_list args;

    va_start(args, lvl);
    fmt = va_arg(args, char *);
    str = g_strdup_vprintf(fmt, args);
    va_end(args);

    mvprintw(LINES - 1, 0, backgrnd_str);
    mvprintw(LINES - 1, 0, str);
    refreshp();

    g_free(str);

    switch (lvl) {
	case L_INFO:
	    sleep(1);
	    break;
	case L_WARN:
	    sleep(3);
	    break;
	case L_ERR:
	    sleep(3);
	    break;
	default:
	    break;
    }
}


