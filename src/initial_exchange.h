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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------------------------
*      initial exchange.h
*-------------------------------------------------------------------------------*/


#ifndef INITIAL_EXCHANGE_H
#define INITIAL_EXCHANGE_H

#include "tlf.h"

#define MAX_IE_LENGTH 30

/** Dataelement for one initial entry item */
struct ie_list {
    struct ie_list *next;		/**< pointer to next element */
    char call[MAX_CALL_LENGTH + 1];	/**< call of the station */
    char exchange [MAX_IE_LENGTH + 1];	/**< initial exchange field */
};

/**
*	Make linked list from initial exchange file.
*	File must be in CALL,EXCHANGE format.
*	Returns pointer to the top of the list.
*/
struct ie_list *make_ie_list(char *file);
void free_ie_list(struct ie_list *head);

#endif /* INITIAL_EXCHANGE_H */
