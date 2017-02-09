/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2011, 2013 Thomas Beierlein <tb@forth-ev.de>
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


#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "dxcc.h"


GPtrArray *dxcc;
GPtrArray *prefix;

prefix_data dummy_pfx = {
    "No Prefix",
    0,
    0,
    0
};


void prefix_free(gpointer data) {
	prefix_data *pfx_data = data;

	g_free(pfx_data -> pfx);
	g_free(pfx_data);
}


void prefix_init(void)
{
	if (prefix) {
		g_ptr_array_free(prefix, TRUE);
	}
	prefix = g_ptr_array_new_with_free_func(prefix_free);
}

/* return number of entries in prefix array */
unsigned int prefix_count(void)
{
	return prefix->len;
}

/* give pointer to prefix struct at 'index' */
prefix_data *prefix_by_index(unsigned int index)
{
	if (index >= prefix_count())
	    return &dummy_pfx;

	return (prefix_data *)g_ptr_array_index(prefix, index);
}

/* add a new prefix description */
void prefix_add (char *pfxstr)
{
	gchar *loc;
	gint last_index = dxcc_count() - 1;
	dxcc_data *last_dx = dxcc_by_index(last_index);
	prefix_data *new_prefix = g_new (prefix_data, 1);

	loc = strchr(pfxstr, '[');
	if (loc != NULL) {
	    new_prefix -> itu = atoi(loc + 1);
	    *loc = '\0';
	}
	else
	    new_prefix -> itu = last_dx -> itu;

	loc = strchr(pfxstr, '(');
	if (loc != NULL) {
	    new_prefix -> cq = atoi(loc + 1);
	    *loc = '\0';
	}
	else
	    new_prefix -> cq = last_dx -> cq;

	new_prefix -> pfx = g_strdup(pfxstr);
	new_prefix -> dxcc_index = last_index;

	g_ptr_array_add (prefix, new_prefix);
}



void dxcc_free(gpointer data) {
	dxcc_data *dxcc = data;

	g_free(dxcc -> countryname);
	g_free(dxcc -> continent);
	g_free(dxcc -> pfx);
	g_free(dxcc);
}

void dxcc_init(void)
{
	if (dxcc) {
		g_ptr_array_free(dxcc, TRUE);
	}
	dxcc = g_ptr_array_new_with_free_func(dxcc_free);
}

/* return number of entries in dxcc array */
unsigned int dxcc_count(void)
{
	return dxcc->len;
}

/* give pointer to dxcc_data struct at 'index' */
dxcc_data *dxcc_by_index(unsigned int index)
{
	if (index >= dxcc_count())
	    index = 0;

	return (dxcc_data *)g_ptr_array_index(dxcc, index);
}

void dxcc_add (char * dxcc_line)
{
	gchar **split;
	gint item;
	dxcc_data *new_dxcc = g_new (dxcc_data, 1);

	/* split up the line */
	split = g_strsplit(dxcc_line, ":", 9);

	if (g_strv_length(split) < 8) {	/* wrong syntax, ignore line */
	    g_strfreev(split);
	    g_free(new_dxcc);
	    return;
	}

	for (item = 0; item < 8; item++)
	    g_strstrip(split[item]);

	new_dxcc -> countryname = g_strdup(split[0]);
	new_dxcc -> cq = atoi(split[1]);
	new_dxcc -> itu = atoi(split[2]);
	new_dxcc -> continent = g_strdup(split[3]);
	new_dxcc -> lat = atof(split[4]);
	new_dxcc -> lon = atof(split[5]);
	new_dxcc -> timezone = atof(split[6]);
	if (*split[7] == '*') {
	    new_dxcc -> pfx = g_strdup(split[7]+1);
	    new_dxcc -> starred = 1;
	} else {
	    new_dxcc -> pfx = g_strdup(split[7]);
	    new_dxcc -> starred = 0;
	}

	g_strfreev (split);

	g_ptr_array_add (dxcc, new_dxcc);
}
