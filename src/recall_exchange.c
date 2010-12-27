/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by:q
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
#include "recall_exchange.h"
#include "initial_exchange.h"

/** \brief Recall former exchange or lookup initial exchange file 
 *
 * First search 'hiscall' in already worked stations (callarray). If not found 
 * there lookup 'hiscall' in initial exchange file. If found somewhere copy 
 * the according exchange into the 'comment' field.
 *
 * \return 1 - found, -1 - not found, 0 - call field was empty */
int recall_exchange(void)
{

    extern int callarray_nr;
    extern char callarray[MAX_CALLS][20];
    extern char call_exchange[MAX_CALLS][12];
    extern char hiscall[];
    extern char comment[];
    extern struct ie_list *main_ie_list;

    int i, index, j;
    int found = -1;
    char *loc;
    struct ie_list *current_ie;

    if (strlen(hiscall) == 0)
	return (0);

    for (i = callarray_nr; i >= 0; i--) {

	/* first search call in already worked stations */
	if (strstr(callarray[i], hiscall) != NULL) {
	    found = 1;
	    strcpy(comment, call_exchange[i]);

	    for (j = 0; j < strlen(comment); j++)
		if (comment[j] == ' ') {
		    comment[j] = '\0';
		    break;
		}
	    loc = strchr(comment, '\0');
	    if (loc != NULL) {
		index = (int) (loc - comment);
		if (index <= strlen(comment))
		    comment[index] = '\0';
	    }
	    mvprintw(12, 54, comment);
	    break;
	}
    }

    if (found == -1) {

	/* if no exchange could be recycled search initial exchange list */
	if (strlen(comment) == 0 && main_ie_list != NULL) {

	    current_ie = main_ie_list;

	    while (current_ie) {
		if (strstr(hiscall, current_ie->call) != NULL) {
		    found = 1;
		    strcpy(comment, current_ie->exchange);
		    mvprintw(12, 54, comment);
		    refresh();
		    break;
		} 
		current_ie = current_ie->next;
	    }
	}

    }

    return found;
}
