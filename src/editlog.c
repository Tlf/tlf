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
    extern const char backgrnd_str[];
    extern int editor;

    char comstr[40] = "";
    int j;

    if (editor == EDITOR_JOE)
	strcat(comstr, "joe  ");	/*   my favorite editor   */
    else if (editor == EDITOR_VI)
	strcat(comstr, "vi  ");
    else if (editor == EDITOR_MC)
	strcat(comstr, "mcedit  ");
    else
	strcat(comstr, "e3  ");

    stop_background_process();
    strcat(comstr, logfile);
    IGNORE(system(comstr));;
    start_background_process();

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
