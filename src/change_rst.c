/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2020 Thomas Beierlein <dl1jbe@darc.de>
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


#include <glib.h>
#include <string.h>
#include <stdbool.h>
#include "change_rst.h"
#include "globalvars.h"

bool change_rst = false;

struct rst {
    GPtrArray *array;
    int index;
};

static struct rst rst_sent = {NULL, 0};
static struct rst rst_recv = {NULL, 0};

/* sorter for RST values */
static int cmp_rst (char **a, char **b) {
    return g_ascii_strcasecmp(*a, *b);
}

/* Create a new GPtrarray holding allowed RS(T) values,
 * parses initstr and add elements to new GPtrArray,
 * finally sort elements */
static void init_array(struct rst *rst, const char *initstr) {
    gchar **list;

    if (rst->array != NULL)
	g_ptr_array_free(rst->array, TRUE);
    rst->array = g_ptr_array_new_full(25, g_free);

    list = g_strsplit(initstr, ",", 0);
    for (int i = 0; list[i] != NULL; i++) {
	char *tmp = g_strdup(list[i]);
	g_ptr_array_add(rst->array, g_strdup(g_strstrip(tmp)));
	g_free(tmp);
    }
    g_strfreev(list);
    g_ptr_array_sort(rst->array, (GCompareFunc)cmp_rst);
}

/* Initialize RS(T) tables for sent and received rapports,
 * sent rapports can be specified by init_string */
void rst_init(char *init_string) {
    char *default_rst = "33, 34, 35, 36, 37, 38, 39, \
		      43, 44, 45, 46, 47, 48, 49, \
		      53, 54, 55, 56, 57, 58, 59";

    init_array(&rst_recv, default_rst);
    init_array(&rst_sent, (init_string != NULL) ? init_string : default_rst);

    rst_reset();
}

/* reset RS(T) values for start of new QSO to highest available value */
void rst_reset(void) {
    rst_sent.index = rst_sent.array->len - 1;
    memcpy(sent_rst, g_ptr_array_index(rst_sent.array, rst_sent.index), 2);

    rst_recv.index = rst_recv.array->len - 1;
    memcpy(recvd_rst, g_ptr_array_index(rst_recv.array, rst_recv.index), 2);
}


/* initialize 'my_rst' and 'his_rst' */
void rst_set_strings() {
    memcpy(recvd_rst, g_ptr_array_index(rst_recv.array, rst_recv.index), 2);
    memcpy(sent_rst, g_ptr_array_index(rst_sent.array, rst_sent.index), 2);
   if (trxmode != SSBMODE) {
        recvd_rst[2] = '9';
        sent_rst[2] = '9';
    } else {
        recvd_rst[2] = ' ';
        sent_rst[2] = ' ';
    }
}


void rst_recv_up() {
    if (rst_recv.index < rst_recv.array->len - 1) {
	rst_recv.index++;
	rst_set_strings();
    }
}

void rst_recv_down() {
    if (rst_recv.index > 0) {
	rst_recv.index--;
	rst_set_strings();
    }
}

void rst_sent_up() {
    if (rst_sent.index < rst_sent.array->len - 1) {
	rst_sent.index++;
	rst_set_strings();
    }
}

void rst_sent_down() {
    if (rst_sent.index > 0) {
	rst_sent.index--;
	rst_set_strings();
    }
}
