/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 -2004 Rein Couperus <pa0r@amsat.org>
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

	/* ------------------------------------------------------------------------------
 	*      initial exchange.c
 	*
	*  makes list of calls and exchanges from comma separated file
	*  and retrieves them
	*
 	*-------------------------------------------------------------------------------*/

#include "initial_exchange.h"

struct ie_list *make_ie_list (void) {

extern char exchange_list[];

FILE *fp;
char inputbuffer[91];
char *loc;

struct ie_list *ie_listhead;
struct ie_list *ie_current;
struct ie_list *new;

 	if  ( (fp = fopen(exchange_list,"r"))  == NULL){
		showmsg("Cannot find initial exchange file");
		return(NULL);
	}else
		showstring ("Using initial exchange file", exchange_list);

	ie_listhead = malloc(sizeof (struct ie_list));

	if (ie_listhead == NULL) {
		showmsg("Out of memory");
		return(NULL);
	}

	fgets(inputbuffer, 90, fp);

	loc = strchr(inputbuffer, ',');

	if (loc != NULL) {
		inputbuffer[loc - inputbuffer] = '\0';
		strcpy (ie_listhead->call,inputbuffer);
		strcpy (ie_listhead->exchange, loc+1);
		ie_listhead->exchange[strlen(ie_listhead->exchange) - 1] = '\0';
		ie_listhead->next = NULL;
		ie_current = ie_listhead;
	}else {
		showmsg ("Wrong format, no comma found");
		return(NULL);
	}

	while(!feof(fp)) {

		fgets (inputbuffer, 90,  fp);
		if (strlen(inputbuffer) == 0) break;

		loc = strchr(inputbuffer, ',');

		if (loc != NULL) {	// comma found

			new = malloc(sizeof (struct ie_list));

			if (new == NULL) {
				showmsg("Out of memory");
				return(NULL);
			}

			inputbuffer[loc - inputbuffer] = '\0';		// split the string into call and exchange
			strncpy (new->call,inputbuffer, MAX_CALL_LENGTH);
			strncpy (new->exchange, loc+1, MAX_IE_LENGTH);
			new->exchange[strlen(new->exchange) - 1] = '\0';
			new->next = NULL;
			ie_current->next = new;
			ie_current = new;

			inputbuffer[0] = '\0';

		}else {
			showmsg ("Wrong format, no comma found");
			break;
//			return(NULL);
		}

	}

return(ie_listhead);
}

int  test_ie_list (struct ie_list *example_ie_list) {

	if (example_ie_list == NULL)
		return(-1);

	while (1) {
		showmsg(example_ie_list->call);
		showmsg(example_ie_list->exchange);
		if(example_ie_list->next != NULL)
			example_ie_list = example_ie_list->next;
		else
			break;
	}
return(0);
}
