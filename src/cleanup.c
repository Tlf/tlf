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
 *        Cleanup call input field
 *
 *--------------------------------------------------------------*/


#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "write_keyer.h"

void cleanup_qso(void) {
    extern char hiscall[];
    extern char comment[];
    extern char my_rst[];
    extern char his_rst[];

    hiscall[0] = '\0';	    /* reset hiscall and comment */
    comment[0] = '\0';
    his_rst[1] = '9';	    /* reset to 599 */
    my_rst[1] = '9';
}

int cleanup(void) {
    extern int defer_store;

    int k = 0;

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvprintw(12, 29, "            ");
    mvprintw(12, 54, "                        ");
    mvprintw(12, 29, "");

    attron(COLOR_PAIR(C_LOG | A_STANDOUT));

    for (k = 1; k <= 5; k++) {
	mvprintw(k, 0, "%s", "                                        ");
    }

    refreshp();
    cleanup_qso();
    defer_store = 0;
    keyer_flush();

    return (0);
}
