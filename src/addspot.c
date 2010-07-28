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
	 *
	 *              Add spot to bandmap
	 *
	 *
	 *--------------------------------------------------------------*/
#include "addspot.h"

int addspot(void)
{
    extern float freq;
    extern char hiscall[];
    extern int trx_control;
    extern int lanspotflg;
    extern struct tm *time_ptr;

    char spotline[160];
    char frequency[8];
    char spottime[6];

    if (trx_control == 0) {

	attron(COLOR_PAIR(7) | A_STANDOUT);

	mvprintw(13, 20, "freq.: ");
	echo();
	getnstr(frequency, 7);
	noecho();
	freq = atof(frequency);
    }
    spotline[0] = '\0';

    strcat(spotline, "DX de TLF   :        ");	/* todo: change call here.... */

    if (freq >= 10000.0)
	sprintf(spotline + 17, "%5.1f", freq);
    else
	sprintf(spotline + 18, "%5.1f", freq);

    strcat(spotline, "  ");
    strcat(spotline, hiscall);
    strcat(spotline, "                                           ");

    get_time();
//              strftime(spottime, 80, "%H%MZ", time_ptr);      ### bug fix
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
