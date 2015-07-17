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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

	/* ------------------------------------------------------------------------------
	 *      initial exchange.c
	 *
	 *  makes list of calls and exchanges from comma separated file
	 *  and retrieves them
	 *
	 *-------------------------------------------------------------------------------*/

#include "initial_exchange.h"
#include "startmsg.h"
#include "tlf.h"

struct ie_list *make_ie_list(char *file)
{

    FILE *fp;
    char inputbuffer[91];
    char *loc;

    struct ie_list *ie_listhead = NULL;
    struct ie_list *new;
    char *token;
    int linectr = 0;

    if ((fp = fopen(file, "r")) == NULL) {
	showmsg("Cannot find initial exchange file");
	return (NULL);
    } else
	showstring("Using initial exchange file", file);

    while (fgets(inputbuffer, 90, fp) != NULL) {

	linectr++;

	/* allow empty and comment lines */
	if ((inputbuffer[0] == '#') ||
	    strspn(inputbuffer, " \t") == strlen(inputbuffer)-1)
	    continue;			

	/* strip trailing newline */ 
	if ((loc = strchr(inputbuffer, '\n')) != NULL)
	    *loc = '\0';

	if (strlen(inputbuffer) > 80) {
	    /* line to long */
	    char msg[80];
	    free_ie_list(ie_listhead);
	    fclose(fp);
	    sprintf( msg, "Line %d: too long", linectr);
	    showmsg(msg);
	    return NULL;
	}

	loc = strchr(inputbuffer, ',');

	if (loc != NULL) {	// comma found

	    new = malloc(sizeof(struct ie_list));

	    if (new == NULL) {
		free_ie_list(ie_listhead);
		fclose(fp);
		showmsg("Out of memory");
		return (NULL);
	    }

	    *loc = '\0';	/* split the string into call and exchange */

	    token = strtok(inputbuffer, " \t"); 	/* callsign is first
							   token delimited by
							   whitespace */
	    if ((token == NULL) || strtok(NULL, " \t")) {
		/* 0 or >1 token before comma */
		char msg[80];
		free(new);
		free_ie_list(ie_listhead);
		fclose(fp);
		sprintf( msg, "Line %d: 0 or more than one token before comma",
			linectr);
		showmsg(msg);
		return (NULL);
	    }

	    strncpy(new->call, token, MAX_CALL_LENGTH);
	    new->call[MAX_CALL_LENGTH] = '\0';		/* proper termination */

	    loc += strspn(loc+1, " \t");		/* skip leading space */
	    strncpy(new->exchange, loc + 1, MAX_IE_LENGTH);
	    new->exchange[MAX_IE_LENGTH] = '\0';	/* proper termination */

	    /* prepend new entry to existing list */
	    new->next = ie_listhead;
	    ie_listhead = new;

	    inputbuffer[0] = '\0';

	} else {
	    /* no comma found */
	    char msg[80];
	    free_ie_list(ie_listhead);
	    fclose(fp);
	    sprintf( msg, "Line %d: no comma found", linectr);
	    showmsg(msg);
	    return NULL;			
	}
    }

    fclose(fp);

    return (ie_listhead);
}

void free_ie_list(struct ie_list *head)
{
    struct ie_list *next;

    while (head) {
	next = head->next;
	free(head);
	head = next;
    }
}

int test_ie_list(struct ie_list *example_ie_list)
{

    if (example_ie_list == NULL)
	return (-1);

    while (1) {
	showmsg(example_ie_list->call);
	showmsg(example_ie_list->exchange);
	if (example_ie_list->next != NULL)
	    example_ie_list = example_ie_list->next;
	else
	    break;
    }
    return (0);
}
