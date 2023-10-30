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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <glib.h>
#include <math.h>

#include "dxcc.h"


GPtrArray *dxcc;
GPtrArray *prefix;
GHashTable *hashed_prefix;
int two_char_prefix_index[36 * 36];
bool have_exact_matches;
char cty_dat_version[12];   // VERyyyymmdd

enum {
    TCPI_NONE = -1,
    TCPI_AMB = -2,
};

prefix_data dummy_pfx = {
    "No Prefix",
    0,
    0,
    0,
    INFINITY,
    INFINITY,
    "",
    INFINITY,
    false
};


static void prefix_free(gpointer data) {
    prefix_data *pfx_data = data;

    g_free(pfx_data -> pfx);
    g_free(pfx_data -> continent);
    g_free(pfx_data);
}


void prefix_init(void) {
    if (hashed_prefix) {
	g_hash_table_destroy(hashed_prefix);
    }
    hashed_prefix = g_hash_table_new(g_str_hash, g_str_equal);

    if (prefix) {
	g_ptr_array_free(prefix, TRUE);
    }
    prefix = g_ptr_array_new_with_free_func(prefix_free);
    have_exact_matches = false;
    for (int i = 0; i < 36 * 36; i++) {
	two_char_prefix_index[i] = TCPI_NONE;
    }
}

/* convert char to base36 */
static int to_base36(char c) {
    if (isdigit(c)) {
	return c - '0';
    }
    if (isupper(c)) {
	return 10 + c - 'A';
    }
    return 0;   // rest is mapped to zero
}

/* get hash key for a call/prefix */
static int prefix_hash_key(const char *call) {
    if (call[0] == 0) { // normally call is never empty
	return 0;
    }
    return to_base36(call[0]) + 36 * to_base36(call[1]);
}

/* return number of entries in prefix array */
unsigned int prefix_count(void) {
    return prefix->len;
}

/* give pointer to prefix struct at 'index' */
prefix_data *prefix_by_index(unsigned int index) {
    if (index < 0 || index >= prefix_count())
	return &dummy_pfx;

    return (prefix_data *)g_ptr_array_index(prefix, index);
}

/** lookup key in table of hashed prefixes
 * \return - true, if found in HashTable
 * \param key - part of call to look up
 * \param value - the corresponding prefix index
 */
static gboolean lookup_hashed_prefix(const char *key, void *value) {
    return g_hash_table_lookup_extended(hashed_prefix, key, NULL, value);
}


/* search for a full match of 'call' in the pfx table */
int find_full_match(const char *call) {
    void *value;
    int  w = -1;

    if (lookup_hashed_prefix(call, &value)) {
	w = GPOINTER_TO_INT(value);
    }

    return w;
}


/* search for the best mach of 'call' in pfx table */
int find_best_match(const char *call) {
    void *value;
    int w = -1;

    if (call == NULL)
	return w;

    /* first check if it has a unique 2-char prefix */
    if (strlen(call) >= 2) {
	int key = prefix_hash_key(call);
	w = two_char_prefix_index[key];
	if (w >= 0) {
	    bool ok = true;
	    if (prefix_by_index(w)->exact) {
		ok = (strcmp(prefix_by_index(w)->pfx, call) == 0);
	    }
	    if (ok) {
		return w;
	    }
	}
    }

    /* first try full match */
    if (lookup_hashed_prefix(call, &value)) {
	w = GPOINTER_TO_INT(value);
	return w;
    }

    /* stepwise shorten the call and pick up first one -> maximum length
     * Be careful to not use entries which require an exact match
     */
    char *temp = g_strdup(call);
    for (int len = strlen(call) - 1; len >= 1; len--) {
	temp[len] = 0;  // truncate to len
	if (lookup_hashed_prefix(temp, &value)) {
	    int idx = GPOINTER_TO_INT(value);
	    if (!prefix_by_index(idx)->exact) {
		w = idx;
		break;
	    }
	}
    }
    g_free(temp);

    return w;
}


/* add a new DXCC prefix description */
void prefix_add(char *pfxstr) {

    char *ver = (*pfxstr == '=' ? pfxstr + 1 : pfxstr);
    if (strlen(ver) == 11 && strncmp(ver, "VER", 3) == 0) {
	strcpy(cty_dat_version, ver);    // save it
    }

    gchar *loc;
    gint last_index = dxcc_count() - 1;
    dxcc_data *last_dx = dxcc_by_index(last_index);
    prefix_data *new_prefix = g_new(prefix_data, 1);

    if (*pfxstr == '=') {
	new_prefix -> exact = true;
	have_exact_matches = true;
	pfxstr++;
    } else
	new_prefix -> exact = false;

    loc = strchr(pfxstr, '~');
    if (loc != NULL) {
	new_prefix -> timezone = atof(loc + 1);
	*loc = '\0';
    } else
	new_prefix -> timezone = last_dx->timezone;

    loc = strchr(pfxstr, '{');
    if (loc != NULL) {
	new_prefix -> continent = g_strdup(loc + 1);
	*loc = '\0';
	loc = strchr(new_prefix -> continent, '}');
	if (loc != NULL)
	    *loc = '\0';
    } else
	new_prefix -> continent = g_strdup(last_dx->continent);

    loc = strchr(pfxstr, '<');
    if (loc != NULL) {
	new_prefix -> lat = atof(loc + 1);
	*loc = '\0';
	if ((loc = strchr(loc, '/')) != NULL)
	    new_prefix -> lon = atof(loc + 1);
	else
	    new_prefix -> lon = INFINITY;
    } else {
	new_prefix -> lat = last_dx->lat;
	new_prefix -> lon = last_dx->lon;
    }

    loc = strchr(pfxstr, '[');
    if (loc != NULL) {
	new_prefix -> itu = atoi(loc + 1);
	*loc = '\0';
    } else
	new_prefix -> itu = last_dx -> itu;

    loc = strchr(pfxstr, '(');
    if (loc != NULL) {
	new_prefix -> cq = atoi(loc + 1);
	*loc = '\0';
    } else
	new_prefix -> cq = last_dx -> cq;

    new_prefix -> pfx = g_strdup(pfxstr);
    new_prefix -> dxcc_ctynr = last_index;

    g_ptr_array_add(prefix, new_prefix);
    int index = prefix_count() - 1;
    g_hash_table_insert(hashed_prefix,
			new_prefix->pfx,
			GINT_TO_POINTER(index));

    /* build 2-char prefix hash */
    if (strlen(pfxstr) >= 2) {
	int key = prefix_hash_key(pfxstr);
	if (two_char_prefix_index[key] == TCPI_NONE) {
	    two_char_prefix_index[key] = index;     // first one
	} else {
	    two_char_prefix_index[key] = TCPI_AMB;  // ambiguous
	}
    }
}



static void dxcc_free(gpointer data) {
    dxcc_data *dxcc = data;

    g_free(dxcc -> countryname);
    g_free(dxcc -> continent);
    g_free(dxcc -> pfx);
    g_free(dxcc);
}

void dxcc_init(void) {
    cty_dat_version[0] = 0;
    if (dxcc) {
	g_ptr_array_free(dxcc, TRUE);
    }
    dxcc = g_ptr_array_new_with_free_func(dxcc_free);
}

/* return number of entries in dxcc array */
unsigned int dxcc_count(void) {
    return dxcc->len;
}

/* give pointer to dxcc_data struct at 'index' */
dxcc_data *dxcc_by_index(unsigned int index) {
    if (index >= dxcc_count())
	index = 0;

    return (dxcc_data *)g_ptr_array_index(dxcc, index);
}

void dxcc_add(char *dxcc_line) {
    gchar **split;
    gint item;
    dxcc_data *new_dxcc = g_new(dxcc_data, 1);

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
	new_dxcc -> pfx = g_strdup(split[7] + 1);
	new_dxcc -> starred = true;
    } else {
	new_dxcc -> pfx = g_strdup(split[7]);
	new_dxcc -> starred = false;
    }

    g_strfreev(split);

    g_ptr_array_add(dxcc, new_dxcc);
}

/** load cty database from filename */
int load_ctydata(char *filename) {
    FILE *fd;
    char *buf = NULL;
    size_t buf_len;
    char *loc;
    int read;

    if ((fd = fopen(filename, "r")) == NULL)
	return -1;

    dxcc_init();
    prefix_init();

    // set default for empty country == country nr 0
    dxcc_add("Not Specified        :    --:  --:  --:  -00.00:    00.00:     0.0:     :");

    while ((read = getline(&buf, &buf_len, fd)) != -1) {
	if (buf_len > 0) {
	    if (errno == ENOMEM) {
		fprintf(stderr, "Error in: %s:%d", __FILE__, __LINE__);
		perror("RuntimeError: ");
		exit(EXIT_FAILURE);
	    }
	    g_strchomp(buf); 	/* drop CR and/or NL and */
	    if (*buf == '\0')	/* ignore empty lines */
		continue;

	    if (buf[0] != ' ') {	// data line
		dxcc_add(buf);
	    } else {		// prefix line
		loc = strtok(buf, " ,;");
		while (loc != NULL) {
		    prefix_add(loc);
		    loc = strtok(NULL, " ,;");
		}
	    }
	}
    }

    if (buf != NULL)
	free(buf);

    fclose(fd);
    return 0;
}
