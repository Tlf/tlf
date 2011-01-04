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
	 *     add prefix
	 *
	 *--------------------------------------------------------------*/

#include "addpfx.h"

char prefixes_worked[MAX_CALLS][6];
int nr_of_px = 0;

int add_pfx(char *pxstr)
{


    int q = 0, found = 0;

    prefixes_worked[nr_of_px][0] = '\0';

    for (q = 0; q <= nr_of_px; q++) {

	if (strcmp(pxstr, prefixes_worked[q]) == 0) {
	    found = 1;
	    break;

	}
    }
    if (found != 1) {
	strcpy(prefixes_worked[nr_of_px], pxstr);
	nr_of_px++;
    }

    return (found);
}

	/*--------------------addpx for LAN qso's--------------------------------------*/
