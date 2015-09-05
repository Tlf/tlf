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
#include <glib.h>
#include "string.h"
#include "tlf.h"
#include "qtcvars.h"

GHashTable* qtc_store = NULL; 	/* stores number of QTC's per callsign */
struct t_qtc_store_obj *qtc_empty_obj = NULL;

void qtc_init() {
    if (qtc_store != NULL) {
	g_hash_table_destroy(qtc_store);
    }
    qtc_store = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    if (qtc_empty_obj != NULL) {
	g_free(qtc_empty_obj);
    }
    qtc_empty_obj = g_malloc(sizeof (struct t_qtc_store_obj));
    qtc_empty_obj->total = 0;
    qtc_empty_obj->received = 0;
    qtc_empty_obj->sent = 0;
    qtc_empty_obj->capable = 0;
}

void qtc_meta_write() {
    struct t_qtc_store_obj *qtc_obj;
    GList * qtc_key_list;
    char logline[20];
    FILE * fp;

    qtc_key_list = g_hash_table_get_keys(qtc_store);
    if ((fp = fopen(QTC_META_LOG, "w")) == NULL) {
	mvprintw(5, 0, "Error opening QTC meta logfile.\n");
	refreshp();
	sleep(2);
    }
    while(qtc_key_list != NULL) {
	    qtc_obj = g_hash_table_lookup(qtc_store, qtc_key_list->data);
	    if (qtc_obj->capable == 2) {
		sprintf(logline, "%s;L\n", (char *)qtc_key_list->data);
		fputs(logline, fp);
	    }
	    if (qtc_obj->capable == -1) {
		sprintf(logline, "%s;N\n", (char *)qtc_key_list->data);
		fputs(logline, fp);
	    }
	    qtc_key_list = qtc_key_list->next;
    }
    fclose(fp);
    g_list_free(qtc_key_list);
}

void qtc_inc(char callsign[15], int direction) {
    struct t_qtc_store_obj *qtc_obj;

    qtc_obj = g_hash_table_lookup(qtc_store, callsign);
    if (qtc_obj == NULL) {
	qtc_obj = g_malloc0(sizeof (struct t_qtc_store_obj));
	qtc_obj->total = 0;
	qtc_obj->received = 0;
	qtc_obj->sent = 0;
	g_hash_table_insert(qtc_store, strdup(callsign), qtc_obj);
    }

    if (direction == RECV || direction == SEND) {
	qtc_obj->total++;
	if (direction == RECV) {
		qtc_obj->received++;
	}
	if (direction == SEND) {
		qtc_obj->sent++;
	}
    }
    if (direction == QTC_CAP) {
	qtc_obj->capable = 1;
    }
    if (direction == QTC_LATER) {
	qtc_obj->capable = 2;
    }
    if (direction == QTC_NO) {
	qtc_obj->capable = -1;
    }
    if (direction == QTC_LATER || direction == QTC_NO) {
	qtc_meta_write();
    }
}

void qtc_dec(char callsign[15], int direction) {
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

void parse_qtcline(char * logline, char callsign[15], int direction) {

    int i = 0;

    if (direction == RECV) {
	strncpy(callsign, logline+30, 15);
    }
    if (direction == SEND) {
	strncpy(callsign, logline+35, 15);
    }
    while(callsign[i] != ' ') {
	i++;
    }
    callsign[i] = '\0';
}

char qtc_get_value(struct t_qtc_store_obj * qtc_obj) {

    if (qtc_obj->total > 0) {
	if (qtc_obj->total == 10) {
	    return 'Q';
	}
	else {
	    return qtc_obj->total+48;
	}
    }
    else {
	if (qtc_obj->capable == 1) {
	    return 'P';
	}
	if (qtc_obj->capable == 2) {
	    return 'L';
	}
	if (qtc_obj->capable == -1) {
	    return 'N';
	}
    }
    return '\0';
}

int parse_qtc_flagstr(char * lineptr, char * callsign, char * flag) {
    char * tmp;

    tmp = strtok(lineptr, ";");
    if (tmp != NULL) {
	strcpy(callsign, tmp);
	tmp = strtok(NULL, ";");
	if (tmp != NULL) {
	  strncpy(flag, tmp, 1);
	  return 0;
	}
    }
    return 1;
}

void parse_qtc_flagline(char * lineptr) {
    int rc;
    char callsign[15], flag[2], msg[18];

    rc = parse_qtc_flagstr(lineptr, callsign, flag);
    if (rc == 0 && (flag[0] == 'N')) {
	qtc_inc(callsign, QTC_NO);
    }
    if (rc == 0 && (flag[0] == 'L')) {
	qtc_inc(callsign, QTC_LATER);
    }
    sprintf(msg, "%s;%c", callsign, flag[0]);
}
