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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* ------------------------------------------------------------
 *   show prefix map
 *
 *--------------------------------------------------------------*/


#include <glib.h>
#include <string.h>

#include "dxcc.h"
#include "focm.h"
#include "changepars.h"
#include "keystroke_names.h"
#include "setcontest.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "bands.h"


void show_mults(void) {

    extern int countries[MAX_DATALINES];
    extern int bandinx;

    int i, j, k, l, bandmask = 0;
    static char prefix[5];
    static char zonecmp[3] = "";
    int ch;

    int iMax = dxcc_count();

    if (CONTEST_IS(FOCMARATHON)) {
	foc_show_cty();
	return;
    }

    if (CONTEST_IS(CQWW)) {

	mvprintw(12, 29, "E,A,F,N,S,O");

	refreshp();

	ch = key_get();

	while (ch != '\n' && ch != KEY_ENTER) {

	    if (ch == ESCAPE)
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

	    for (l = 1; l < 6; l++)
		mvprintw(l, 0, backgrnd_str);

	    i = 0;

	    for (k = 1; k < 6; k++) {

		for (j = 0; j <= 19; j++) {

		    while ((i < iMax) &&
			    ((strncmp(dxcc_by_index(i) -> continent, zonecmp, 2))
			     != 0)) {
			i++;
		    }
		    if (i == iMax)
			break;

		    bandmask = inxes[bandinx];

		    if ((countries[i] & bandmask) == 0) {
			strncpy(prefix, dxcc_by_index(i)->pfx, 3);
			g_strlcat(prefix, "     ", sizeof(prefix));

			attron(modify_attr(COLOR_PAIR(C_INPUT)));

			mvprintw(k, j * 4, prefix);
			refreshp();
			i++;

		    } else {

			mvprintw(k, j * 4, "    ");
			refreshp();
			i++;

		    }

		}
		if (i == iMax)
		    break;

	    }

	    ch = key_get();

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);


	}			// end while

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
	for (l = 1; l < 6; l++)
	    mvprintw(l, 0, backgrnd_str);
    } else

	multiplierinfo();

}
