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
	 *   show prefix map
	 *
	 *--------------------------------------------------------------*/

#include "showpxmap.h"
#include "dxcc.h"

int show_mults(void)
{
    extern int use_rxvt;
    extern int countries[MAX_DATALINES];
    extern int bandinx;
    extern int cqww;

    int i, j, k, l, bandmask = 0;
    static char prefix[5];
    static char zonecmp[3] = "";
    int ch;

    int iMax = dxcc_count();

    if (cqww == 1) {

	mvprintw(12, 29, "E,A,F,N,S,O");

	refreshp();

	ch = getchar();

	while (ch != '\015') {

	    if (ch == 27)
		break;

	    zonecmp[0] = '\0';
	    if (ch == 'E' || ch == 'e')
		strcat(zonecmp, "EU");
	    else if (ch == 'A' || ch == 'a')
		strcat(zonecmp, "AS");
	    else if (ch == 'F' || ch == 'f')
		strcat(zonecmp, "AF");
	    else if (ch == 'N' || ch == 'n')
		strcat(zonecmp, "NA");
	    else if (ch == 'S' || ch == 's')
		strcat(zonecmp, "SA");
	    else if (ch == 'O' || ch == 'o')
		strcat(zonecmp, "OC");
	    else
		strcat(zonecmp, "EU");

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	    i = 0;

	    for (k = 1; k <= 5; k++) {

		for (j = 0; j <= 19; j++) {

		    while ((i < iMax) && 
			((strncmp(dxcc_by_index(i) -> continent, zonecmp, 2)) 
				!= 0)) {
			i++;
		    }
		    if (i == iMax)
		  	 break;

		    switch (bandinx) {
		    case BANDINDEX_160:{
			    bandmask = BAND160;
			    break;
			}
		    case BANDINDEX_80:{
			    bandmask = BAND80;
			    break;
			}
		    case BANDINDEX_40:{
			    bandmask = BAND40;
			    break;
			}
		    case BANDINDEX_20:{
			    bandmask = BAND20;
			    break;
			}
		    case BANDINDEX_15:{
			    bandmask = BAND15;
			    break;
			}
		    case BANDINDEX_10:{
			    bandmask = BAND10;
			    break;
			}
		    }

		    if ((countries[i] & bandmask) == 0) {
			prefix[0] = '\0';
			strncat(prefix, dxcc_by_index(i)->pfx, 3);

			strncat(prefix, "     ", 4 - strlen(prefix));

			if (use_rxvt == 0)
			    attron(COLOR_PAIR(C_INPUT) | A_BOLD);
			else
			    attron(COLOR_PAIR(C_INPUT));

			mvprintw(k + 1, j * 4, prefix);
			refreshp();
			i++;

		    } else {

			mvprintw(k + 1, j * 4, "    ");
			refreshp();
			i++;

		    }

		}
		if (i == iMax)
		    break;

	    }

	    ch = getchar();

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	    for (l = 1; l <= 6; l++)
		mvprintw(l, 0,
			 "                                                                                ");

	}			// end while
    } else

	multiplierinfo();

    return (0);
}
