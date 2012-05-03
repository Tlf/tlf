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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
    mvprintw(5, 42, "");
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    echo();
    getnstr(tonestr, 3);
    noecho();
    tonestr[4] = '\0';

    write_tone();

    return (0);
}

void write_tone(void)
{

    extern int trxmode;
    extern char tonestr[];
    extern int keyerport;
    extern int cfd;

    int mute = 1;


    if (netkeyer(K_TONE, tonestr) < 0) {
	mvprintw(24, 0, "keyer not active; switching to SSB");
	trxmode = SSBMODE;
    }

}
