/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2011 Thomas Beierlein <tb@forth-ev.de>
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

#include "foc.h"
#include <glib.h>

extern int foc;
extern int contest;
extern int showscore_flag;
extern int searchflg;

int got_g4foc; 		/* did we got Gx4FOC on the air? */

/** Initialize settings for FOC contest */
void foc_init(void) {
    foc = 1;
    got_g4foc = 0;
    contest = 1;
    showscore_flag = 1;
    searchflg = 1;
}


/** calculate score for last QSO
 *
 * Calculate the point score for the last qso. Each qso counts one
 * point on every band. Only exception are qsos with Gx4FOC/xxx which
 * counts two points.
 * \param call 	call of the other station
 * \return 	number of points given
 */
int foc_score(char *call) {

    if (g_regex_match_simple("^G(|[A-Z])4FOC(|/.*)", call,
	    G_REGEX_CASELESS, 0)) {
	got_g4foc = 1;
	return 2;
    }
    else {
	return 1;
    }
}
