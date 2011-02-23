/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 		 2011 Thomas Beierlein <tb@forth-ev.de>
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
	 *      search  call array
	 *
	 *--------------------------------------------------------------*/

#include "searchcallarray.h"

/**	\brief lookup 'hiscall' in callarray
 *
 * 	See if 'hiscall' was already worked by looking it up in callarray
 * 	\param hiscall 	callsign to lookup
 *      \return index in callarray where hiscall was found (-1 if not found)
 */
int searchcallarray(char *hiscall)
{
    extern int callarray_nr;
    extern char callarray[MAX_CALLS][20];

    int found = -1;
    int i;
    static char cbuffer[40] = "";

    cbuffer[0] = '\0';
    strcat(cbuffer, hiscall);
    strcat(cbuffer, "           ");
    cbuffer[10] = '\0';

    for (i = 0; i <= callarray_nr; i++) {

	if (strcmp(callarray[i], cbuffer) == 0) {
	    found = i;
	    break;
	}

    }

    return (found);
}
