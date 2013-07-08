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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
	/* ------------------------------------------------------------
	 *        Set Cw sidetone
	 *
	 *--------------------------------------------------------------*/

#include "set_tone.h"
#include "tlf.h"
#include "cwkeyer.h"
#include "clear_display.h"
#include "write_tone.h"
#include "netkeyer.h"

int set_tone(void)
{

    extern char tonestr[];
    extern int trxmode;

    if (trxmode != CWMODE)
	return (1);

    nicebox(4, 40, 1, 6, "Tone");
    attron(COLOR_PAIR(7) | A_STANDOUT);
    mvprintw(5, 41, "      ");
    mvprintw(5, 42, "");
    echo();
    getnstr(tonestr, 3);
    noecho();
    tonestr[3] = '\0';

    write_tone();

    return (0);
}

void write_tone(void)
{

    extern int trxmode;
    extern char tonestr[];
    extern char sc_volume[];

    if (netkeyer(K_TONE, tonestr) < 0) {
	mvprintw(24, 0, "keyer not active; switching to SSB");
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
