/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014 Ervin Hegedüs - HA2OS <airween@gmail.com>
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
	 *        QTC utils functions
	 *
	 *--------------------------------------------------------------*/

#include "qtcutil.h"
#include <stdio.h>
#include <stdlib.h>

#define NBANDS 10

typedef unsigned char t_qtc_bands[NBANDS];

extern GHashTable* qtc_store; // = NULL;
extern int qtcdirection;
extern struct t_qtc_store_obj *qtc_empty_obj;

int qtc_inc(char callsign[15], int direction) {
    struct t_qtc_store_obj *qtc_obj;

    qtc_obj = g_hash_table_lookup(qtc_store, callsign);
    if (qtc_obj == NULL) {
	qtc_obj = g_malloc0(sizeof (struct t_qtc_store_obj));
	qtc_obj->total = 0;
	qtc_obj->received = 0;
	qtc_obj->sent = 0;
	g_hash_table_insert(qtc_store, strdup(callsign), qtc_obj);
    }

    qtc_obj->total++;
    if (direction == RECV) {
	qtc_obj->received++;
    }
    if (direction == SEND) {
	qtc_obj->sent++;
    }

    return 0;
}

int qtc_dec(char callsign[15], int direction) {
    struct t_qtc_store_obj *qtc_obj;

    qtc_obj = g_hash_table_lookup(qtc_store, callsign);
    if (qtc_obj != NULL) {
	qtc_obj->total--;
        if (direction == RECV) {
	    qtc_obj->received--;
	}
	if (direction == SEND) {
	    qtc_obj->sent--;
	}
    }

    return 0;
}

struct t_qtc_store_obj * qtc_get(char callsign[15]) {
    struct t_qtc_store_obj *qtc_obj;

    if (qtc_store == NULL) {
	return qtc_empty_obj;
    }

    qtc_obj = g_hash_table_lookup(qtc_store, callsign);
    if (qtc_obj == NULL) {
	return qtc_empty_obj;
    }
    return qtc_obj;

}

int parse_qtcline(char * logline, char callsign[15], int direction) {

    int i = 0;

    if (direction == RECV) {
	strncpy(callsign, logline+29, 15);
    }
    if (direction == SEND) {
	strncpy(callsign, logline+34, 15);
    }
    while(callsign[i] != ' ') {
	i++;
    }
    callsign[i] = '\0';

    return 0;
}

