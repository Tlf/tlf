/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * Copyright (C) 201-2011       Thomas Beierlein <tb@forth-ev.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by:q
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


#include <string.h>

#include "globalvars.h"
#include "initial_exchange.h"
#include "tlf.h"
#include "tlf_curses.h"


/** \brief Recall former exchange or lookup initial exchange file
 *
 * First search 'hiscall' in already worked stations (callarray). If not found
 * there lookup 'hiscall' in initial exchange file. If found somewhere copy
 * the according exchange into the 'comment' field.
 *
 * \return 1 - found, -1 - not found, 0 - call field was empty */
int recall_exchange(void) {

    int i, l;
    int found = -1;
    char *loc, *loc2;
    struct ie_list *current_ie;

    if (strlen(hiscall) == 0)
	return 0;

    /* respect content which is already in comment field */
    if (strlen(comment) != 0)
	return 0;

    l = strlen(hiscall);

    /* search backwards through list of worked stations */
    for (i = nr_worked - 1; i >= 0; i--) {

	/* first search call in already worked stations */
	/* call has to be exact -> la/dl1jbe/p must be the same again */
	if ((strstr(worked[i].call, hiscall) == worked[i].call) &&
		(*(worked[i].call + l) == '\0' || *(worked[i].call + l) == ' ')) {
	    found = 1;
	    strcpy(comment, worked[i].exchange);
	    break;
	}
    }

    if (found == -1) {

	/* if no exchange could be recycled and no comment available
	 * search initial exchange list (if available) */
	if (strlen(comment) == 0 && main_ie_list != NULL) {

	    current_ie = main_ie_list;

	    while (current_ie) {
		/* call from IE_List has to be a substring of hiscall
		 * but must be delimited on both sides by '/' or eos */
		if ((loc = strstr(hiscall, current_ie->call)) != NULL) {

		    loc2 = loc + strlen(current_ie->call);
		    if (((loc == hiscall) || (*(loc - 1) == '/')) &&
			    ((*loc2 == '\0') || (*loc2 == '/'))) {

			found = 1;
			strcpy(comment, current_ie->exchange);
			break;
		    }
		}
		current_ie = current_ie->next;
	    }
	}

    }

    if (found) {
	mvprintw(12, 54, comment);
	refreshp();
    }

    return found;
}
