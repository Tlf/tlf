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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------
 *        Cleanup call input field
 *
 *--------------------------------------------------------------*/


#include "globalvars.h"
#include "change_rst.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "write_keyer.h"

/* reset comment */
void cleanup_comment(void) {
    current_qso.comment[0] = '\0';
    current_qso.normalized_comment[0] = '\0';
}

/* reset hiscall */
void cleanup_hiscall(void) {
    current_qso.call[0] = '\0';	    /* reset current call and comment */
    proposed_exchange[0] = '\0';
    countrynr = 0;
}


void cleanup_qso(void) {
    cleanup_hiscall();
    cleanup_comment();
    rst_reset();;	    /* reset to 599 */
}

void cleanup(void) {
    extern int defer_store;

    cleanup_qso();
    defer_store = 0;
    keyer_flush();
}
