/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2010-2011      Thomas Beierlein <tb@forth-ev.de>
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
	 *
	 *              Add spot to bandmap
	 *
	 *
	 *--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "get_time.h"
#include "lancode.h"
#include "splitscreen.h"
#include "tlf.h"
#include "tlf_curses.h"


int addspot(void)
{
    extern float freq;
    extern char hiscall[];
    extern int trx_control;
    extern int lanspotflg;
    extern struct tm *time_ptr;
    extern char thisnode;

    char spotline[160];
    char frequency[8];
    char spottime[6];

    if (strlen(hiscall) < 3)
	return(0);

    if (trx_control == 0) {

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	mvprintw(13, 20, "freq.: ");
	echo();
	getnstr(frequency, 7);
	noecho();
	freq = atof(frequency);
    }

    sprintf(spotline, "DX de TLF-%c:     %9.3f  %s", thisnode, freq, hiscall);

    strcat(spotline, "                                           ");

    get_time();

    strftime(spottime, sizeof(spottime), "%H%MZ", time_ptr);
    strcpy(spotline + 70, spottime);
    strcat(spotline, "\n\n");

    send_lan_message(TLFSPOT, spotline);
    lanspotflg = 1;
    addtext(spotline);
    lanspotflg = 0;
    spotline[0] = '\0';
    hiscall[0] = '\0';

    return (0);
}
