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
 	*      initial exchange.h
 	*-------------------------------------------------------------------------------*/


#include "startmsg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "tlf.h"
#include "startmsg.h"


#define MAX_CALL_LENGTH 13
#define MAX_IE_LENGTH 30

struct ie_list {
	struct ie_list *next;
	char call[MAX_CALL_LENGTH];
	char exchange [MAX_IE_LENGTH];
};

/*------------------------------------------------------------------------------
*
*	Make linked list from initial exchange  file.
*	File must be in CALL,EXCHANGE format.
*	Returns pointer to the top of the list.
*
--------------------------------------------------------------------------------*/
struct ie_list *make_ie_list (void);

/*------------------------------------------------------------------------------
*
*	Print linked list from initial exchange  file.
*	File must be in CALL,EXCHANGE format.
*	Returns 0 if o.k.
*
--------------------------------------------------------------------------------*/

int test_ie_list (struct ie_list *example_ie_list);


