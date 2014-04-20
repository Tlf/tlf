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

#include <syslog.h>
#define NBANDS 10

typedef unsigned char t_qtc_rec_bands[NBANDS];

extern GHashTable* qtc_rec_store; // = NULL;
extern char qtcreccalls[MAX_CALLS][15];

/*int create_store() {
    qtc_rec_store = g_hash_table_new(g_str_hash, g_str_equal);
    return 0;
}*/

int qtc_inc(char callsign[15], int band) {

    t_qtc_rec_bands *qtc_rec_bands;
    int i, gi;


    qtc_rec_bands = g_hash_table_lookup(qtc_rec_store, callsign);
    if (qtc_rec_bands == NULL) {
	qtc_rec_bands = g_malloc0(sizeof *qtc_rec_bands);
	for(i=0; i<NBANDS; i++) {
	    (*qtc_rec_bands)[i] = 0;
	}
	gi = g_hash_table_size(qtc_rec_store);
	strncpy(qtcreccalls[gi], callsign, strlen(callsign));
	g_hash_table_insert(qtc_rec_store, qtcreccalls[gi], qtc_rec_bands);
    }
    else {
	qtc_rec_bands = g_hash_table_lookup(qtc_rec_store, callsign);
    }
    (*qtc_rec_bands)[band]++;

    return 0;
}

int qtc_get(char callsign[15], int band) {
    t_qtc_rec_bands *qtc_rec_bands;

    if (qtc_rec_store == NULL) {
	return -1;
    }

    qtc_rec_bands = g_hash_table_lookup(qtc_rec_store, callsign);
    if (qtc_rec_bands == NULL) {
        return -1;
    }
    else {
      return (*qtc_rec_bands)[band];
    }
}

int parse_qtcline(char * logline, char callsign[15], int * bandidx) {

    int i = 0;

    if (logline[0] == '1') {
	*bandidx = BANDINDEX_160;
    }
    else {
	switch(logline[1]) {
	    case '8':	*bandidx = BANDINDEX_80;
			break;
	    case '4':	*bandidx = BANDINDEX_40;
			break;
	    case '2':	*bandidx = BANDINDEX_20;
			break;
	    case '1':
			switch(logline[2]) {
			    case '7':	*bandidx = BANDINDEX_17;
					break;
			    case '5':	*bandidx = BANDINDEX_15;
					break;
			    case '2':	*bandidx = BANDINDEX_12;
					break;
			    case '0':	*bandidx = BANDINDEX_10;
					break;
			}
	}
    }

    strncpy(callsign, logline+29, 15);
    while(callsign[i] != ' ') {
	i++;
    }
    callsign[i] = '\0';

    return 0;
}

