/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012           Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/* ------------------------------------------------------------
 *        Set CW sidetone
 *
 *--------------------------------------------------------------*/


#include <stdlib.h>

#include "err_utils.h"
#include "globalvars.h"
#include "netkeyer.h"
#include "nicebox.h"	// Includes curses.h
#include "set_tone.h"
#include "tlf.h"

char tonestr[5] = "600";

void set_tone(void) {

    if (trxmode != CWMODE)
	return;

    nicebox(4, 40, 1, 6, "Tone");
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvprintw(5, 41, "      ");
    move(5, 42);
    echo();
    getnstr(tonestr, 3);
    noecho();
    tonestr[3] = '\0';

    write_tone();
}

void write_tone(void) {

    if (netkeyer(K_TONE, tonestr) < 0) {
	TLF_LOG_INFO("keyer not active; switching to SSB");
	trxmode = SSBMODE;
    }

    if (atoi(tonestr) != 0) {
	/* work around bugs in cwdaemon:
	 * cwdaemon < 0.9.6 always set volume to 70% at change of tone freq
	 * cwdaemon >=0.9.6 do not set volume at all after change of freq,
	 * resulting in no tone output if you have a freq=0 in between
	 * So... to be sure we set the volume back to our chosen value
	 * or to 70% (like cwdaemon) if no volume got specified
	 */
	if (*sc_volume != '\0')     // set soundcard volume
	    netkeyer(K_STVOLUME, sc_volume);
	else
	    netkeyer(K_STVOLUME, "70");
    }

}
