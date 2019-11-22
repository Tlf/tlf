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

/* ------------------------------------------------------------
 *        Edit Log
 *
 *--------------------------------------------------------------*/


#include <fcntl.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "background_process.h"
#include "checklogfile.h"
#include "clear_display.h"
#include "ignore_unused.h"
#include "scroll_log.h"
#include "tlf.h"
#include "tlf_curses.h"


int logedit(void) {

    extern char logfile[];
    extern char backgrnd_str[];
    extern char *editor_name;

    char *comstr;
    int j;

    comstr = g_strdup_printf("%s %s", editor_name, logfile);

    stop_background_process();
    IGNORE(system(comstr));;
    start_background_process();

    g_free(comstr);

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    erase();
    refreshp();
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }

    stop_background_process();
    checklogfile();
    start_background_process();

    scroll_log();
    refreshp();

    return (0);
}
