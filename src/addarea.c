/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
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
	 *      add call area to list for one band
	 *
	 *--------------------------------------------------------------*/


#include <string.h>


int add_callarea(void)
{
    extern int callareas[];
    extern char hiscall[];
    extern int manise80;
    extern int bandinx;
    extern int total;
    extern int multscore[];

    int addarea = 0;
    int found = 0;

    if (manise80 == 1) {
	if (strncmp(hiscall, "EA1", 3) == 0) {
	    if (callareas[1] == 0) {
		callareas[1] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA2", 3) == 0) {
	    if (callareas[2] == 0) {
		callareas[2] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA3", 3) == 0) {
	    if (callareas[3] == 0) {
		callareas[3] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA4", 3) == 0) {
	    if (callareas[4] == 0) {
		callareas[4] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA5", 3) == 0) {
	    if (callareas[5] == 0) {
		callareas[5] = 1;
		addarea = 1;
	    }
	    if (strncmp(hiscall, "EA5URW", 6) == 0)
		total = total + 9;
	}
	if (strncmp(hiscall, "EA6", 3) == 0) {
	    if (callareas[6] == 0) {
		callareas[6] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA7", 3) == 0) {
	    if (callareas[7] == 0) {
		callareas[7] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA8", 3) == 0) {
	    if (callareas[8] == 0) {
		callareas[8] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "EA9", 3) == 0) {
	    if (callareas[9] == 0) {
		callareas[9] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "CT", 2) == 0) {
	    if (callareas[0] == 0) {
		callareas[0] = 1;
		addarea = 1;
	    }
	}
	if (strncmp(hiscall, "C3", 2) == 0) {
	    if (callareas[10] == 0) {
		callareas[10] = 1;
		addarea = 1;
	    }
	}
    }

    if (addarea == 1) {

	addarea = 0;

	multscore[bandinx]++;

    }

    return (found);
}
