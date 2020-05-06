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
#include "change_rst.h"
#include "globalvars.h"

GPtrArray *rst_sent = NULL;
GPtrArray *rst_recv = NULL;
int sent_index, recv_index;

static int cmp_rst (char **a, char **b) {
    return g_ascii_strcasecmp(*a, *b);
}

void rst_init(char *init) {

    char *default_rst = "33, 34, 35, 36, 37, 38, 39, \
		      43, 44, 45, 46, 47, 48, 49, \
		      53, 54, 55, 56, 57, 58, 59";

    gchar **list;

    if (rst_sent != NULL)
	g_ptr_array_free(rst_sent, TRUE);
    if (rst_recv != NULL)
	g_ptr_array_free(rst_recv,TRUE);

    rst_sent = g_ptr_array_new_full(25, g_free);
    rst_recv = g_ptr_array_new_full(25, g_free);

    list = g_strsplit(default_rst, ",", 0);
    for (int i = 0; list[i] != NULL; i++) {
	char *tmp = g_strdup(list[i]);
	g_ptr_array_add(rst_recv, g_strdup(g_strstrip(tmp)));
	g_free(tmp);
    }
    g_strfreev(list);
    g_ptr_array_sort(rst_recv, (GCompareFunc)cmp_rst);

    if (init != NULL) {
	list = g_strsplit(init, ",", 0);
    } else {
	list = g_strsplit(default_rst, ",", 0);
    }
    for (int i = 0; list[i] != NULL; i++) {
	char *tmp = g_strdup(list[i]);
	g_ptr_array_add(rst_sent, g_strdup(g_strstrip(tmp)));
	g_free(tmp);
    }
    g_strfreev(list);
    g_ptr_array_sort(rst_sent, (GCompareFunc)cmp_rst);

    rst_reset();
}

void rst_reset(void) {
    sent_index = rst_sent->len - 1;
    memcpy(his_rst, g_ptr_array_index(rst_sent, sent_index), 2);

    recv_index = rst_recv->len - 1;
    memcpy(my_rst, g_ptr_array_index(rst_recv, recv_index), 2);
}

void rst_r_up() {
    if (recv_index < rst_recv->len - 1) {
	recv_index++;
	memcpy(my_rst, g_ptr_array_index(rst_recv, recv_index), 2);
    }
}

void rst_r_down() {
    if (recv_index > 0) {
	recv_index--;
	memcpy(my_rst, g_ptr_array_index(rst_recv, recv_index), 2);
    }
}

void rst_s_up() {
    if (sent_index < rst_sent->len - 1) {
	sent_index++;
	memcpy(his_rst, g_ptr_array_index(rst_sent, sent_index), 2);
    }
}

void rst_s_down() {
    if (sent_index > 0) {
	sent_index--;
	memcpy(his_rst, g_ptr_array_index(rst_sent, sent_index), 2);
    }
}
