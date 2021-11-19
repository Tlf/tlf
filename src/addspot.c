/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2010-2016      Thomas Beierlein <tb@forth-ev.de>
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
#include "globalvars.h"
#include "lancode.h"
#include "splitscreen.h"
#include "ui_utils.h"


/** add call to list of spots
 *
 * format a fake DX spot from call and frequency add it to the spot list
 * and send it to other stations in the LAN
 */
void add_to_spots(char *call, freq_t freq) {

    char spotline[160];
    char spottime[6];

    sprintf(spotline, "DX de TLF-%c:     %9.3f  %s", thisnode, freq / 1000.0, call);
    strcat(spotline, spaces(43));

    format_time(spottime, sizeof(spottime), "%H%MZ");
    strcpy(spotline + 70, spottime);
    strcat(spotline, "\n\n");

    send_lan_message(TLFSPOT, spotline);
    lanspotflg = true;
    addtext(spotline);
    lanspotflg = false;
}


void addspot(void) {

    char frequency[8];

    if (strlen(hiscall) < 3) {
	return;
    }

    if (!trx_control) {

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	mvaddstr(13, 20, "freq.: ");
	echo();
	getnstr(frequency, 7);
	noecho();
	freq = atof(frequency) * 1000.0;
    }

    add_to_spots(hiscall, freq);

    hiscall[0] = '\0';

}
