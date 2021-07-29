/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Thomas Beierlein <tb@forth-ev.de>
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
/*
 * ------------------------------------------------------------ *
 * Show help file *
 * *--------------------------------------------------------------
 */


#include <stdlib.h>
#include <unistd.h>

#include <glib/gstdio.h>

#include "clear_display.h"
#include "globalvars.h"
#include "ignore_unused.h"
#include "tlf_curses.h"
#include "utils.h"


/** \brief Show help file
 *
 * Read 'help.txt' and display it via 'less'.
 * Local 'help.txt' in actual directory overrides default file
 * in PKG_DATA_DIR
 */
int show_help(void) {
    char *helpfile;
    char *cmdstr;

    helpfile = find_available("help.txt");

    cmdstr = g_strdup_printf("less %s", helpfile);

    endwin();
    IGNORE(system("clear"));;
    IGNORE(system(cmdstr));;
    IGNORE(system("clear"));;

    g_free(helpfile);
    g_free(cmdstr);

    set_term(mainscreen);
    clear_display();
    return 0;
}

