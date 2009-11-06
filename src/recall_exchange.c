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
#include "recall_exchange.h"
#include "initial_exchange.h"

int recall_exchange(void) {

extern int callarray_nr;
extern char callarray[MAX_CALLS][20];
extern char call_exchange[MAX_CALLS][12];
extern char hiscall[];
extern char comment[];
extern struct ie_list *main_ie_list;

int i,index, j;
char *loc;
struct ie_list *current_ie;

if (strlen(hiscall) == 0) return (0);



for (i = callarray_nr; i >= 0 ; i--) {

 	if(strstr(callarray[i], hiscall) != NULL){
 	 	strcpy(comment, call_exchange[i]);

		 for (j = 0; j < strlen(comment) ; j++)
			if (comment[j] == ' ')   {
				comment[j] = '\0';
				break;
			}
		loc = strchr(comment, '\0');
 	 	if (loc != NULL) {
 	 		index = (int) (loc - comment);
 	 		if (index <= strlen(comment))
 	 			comment[index] = '\0';
 	 	}
 	 	mvprintw(12, 54,  comment);
 	 	break;
 	}

if (strlen(comment) == 0 && main_ie_list != NULL) {

	current_ie = main_ie_list;

	while(1) {
		if(strstr(hiscall, current_ie->call) != NULL) {
			strcpy(comment, current_ie->exchange);
			mvprintw(12, 54,  comment);
			refresh();
			break;
		}
		else {
			if (current_ie->next != NULL )
				current_ie = current_ie->next;
			else
				break;
		}
	}
}

}

 return(i);
}

