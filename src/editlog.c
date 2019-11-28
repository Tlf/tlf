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
#include "err_utils.h"
#include "ignore_unused.h"
#include "readqtccalls.h"
#include "readcalls.h"
#include "scroll_log.h"
#include "tlf.h"
#include "tlf_curses.h"

void edit(char *filename) {
    extern char *editor_cmd;
    char *cmdstr;
    const char *editor = editor_cmd;
    int retval;

    if (editor_cmd == NULL) {
	editor = g_getenv("EDITOR");
    }

    if (editor == NULL) {
	return;
    }

    cmdstr = g_strdup_printf("%s %s", editor, filename);
    retval = (system(cmdstr));;
    if (WEXITSTATUS(retval) == 127) {
	TLF_LOG_WARN("Can not start editor, check EDITOR= command");
    }
    g_free(cmdstr);
}


int logedit(void) {

    extern int total;
    extern char logfile[];
    extern const char backgrnd_str[];
    extern int qtcdirection;

    int j;

    stop_background_process();
    edit(logfile);
    checklogfile();

    total = 0;
    readcalls();
    if (qtcdirection > 0) {
        readqtccalls();
    }

    start_background_process();

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    erase();
    refreshp();
    scroll_log();
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j < LINES - 1; j++) {
	mvprintw(j, 0, backgrnd_str);
    }
    refreshp();

    return (0);
}
