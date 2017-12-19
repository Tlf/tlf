/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012-2013           Thomas Beierlein <tb@forth-ev.de>
 *               2017                Ervin Hegedus <airween@gmail.com>
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
	 *   cabrillo utils file
	 *
	 *--------------------------------------------------------------*/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cabrillo_utils.h"


/* conversion table between tag name in format file and internal tag */
struct tag_conv tag_tbl[] = {
    { "FREQ",	FREQ 	},
    { "MODE",	MODE 	},
    { "DATE",	DATE 	},
    { "TIME", 	TIME 	},
    { "MYCALL", MYCALL 	},
    { "HISCALL",HISCALL },
    { "RST_S",  RST_S 	},
    { "RST_R",  RST_R 	},
    { "EXC_S",  EXC_S 	},
    { "EXCH",   EXCH 	},
    { "EXC1",	EXC1	},
    { "EXC2",	EXC2	},
    { "EXC3",	EXC3	},
    { "EXC4",	EXC4	},
    { "TX",     TX 	},
    { "QTCRCALL", QTCRCALL },
    { "QTCHEAD",  QTCHEAD  },
    { "QTCSCALL", QTCSCALL },
    { "QTC",	QTC }
};


/* translate item name into a tag */
enum tag_t translate_item_name( char *name ) {
    int i;

    /* lookup name in tag list */
    for ( i = 0; i < sizeof(tag_tbl)/sizeof(struct tag_conv); i++) {
    	if (strcmp( tag_tbl[i].item_name, name ) == 0 ) {
	    /* and return corresponding tab */
	    return tag_tbl[i].tag;
	}
    }

    /* if not found return NO_ITEM tag */
    return NO_ITEM;
}

/** free cabrillo format description */
void free_cabfmt(struct cabrillo_desc *desc) {
    int i;

    if (desc == NULL)
	return;

    if (desc->item_array) {
	for ( i = 0; i < desc->item_array->len; i++ ) {
	    g_free( g_ptr_array_index(desc->item_array, i) );
	}

	g_ptr_array_free( desc->item_array, TRUE );
    }

    if (desc->qtc_item_array) {
	for ( i = 0; i < desc->qtc_item_array->len; i++ ) {
	    g_free( g_ptr_array_index(desc->qtc_item_array, i) );
	}

	g_ptr_array_free( desc->qtc_item_array, TRUE );
    }

    if (desc->name) g_free( desc->name );
    g_free( desc );
}


/* parse item describing one entry
 *
 * has to be in following format: item,length
 *   - item to print (date, time, call, ...)
 *   - max length
 */
struct line_item *parse_line_entry(char *line_entry) {
    struct line_item *item;
    gchar **parts;
    enum tag_t tag;

    item = g_malloc( sizeof(struct line_item) );
    parts = g_strsplit(line_entry, ",", 2);

    if ( g_strv_length(parts) == 2 ) {
	tag = translate_item_name( parts[0] );

	item->tag = tag;
	item->len = atoi( parts[1] );
    }
    else {
	/* type is NO_ITEM */
	item->tag = NO_ITEM;
    }

    g_strfreev( parts );

    return item;
}

/** read cabrillo format description
 *
 * Try to read cabrillo format description for given format from
 * file and return a describing structure.
 *
 * \param filename	File to read description from
 * \param format	Name of the format to read
 * \return 		Pointer to a structure describing the format
 * 			(NULL if file or format not found or not readable)
 */
struct cabrillo_desc *read_cabrillo_format (char *filename, char *format)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gchar **list;
    gsize nrstrings;
    struct cabrillo_desc *cabdesc;
    int i;

    keyfile = g_key_file_new();

    if (!g_key_file_load_from_file( keyfile, filename,
		G_KEY_FILE_NONE, &error)) {
	g_error_free( error );

	/* file does not exist or is wrongly formatted */
	g_key_file_free(keyfile);
	return NULL;
    }

    /* check if 'format' defined in file */
    if( g_key_file_has_group( keyfile, format ) == FALSE ) {

	/* group not found */
	g_key_file_free( keyfile );
	return NULL;
    }

    /* read needed keys */
    list = g_key_file_get_string_list( keyfile, format,
			"QSO", &nrstrings, &error );

    if ( error && error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {

	/* if not found -> stop processing as that key is mandatory */
	g_error_free( error );
	g_key_file_free( keyfile );
	return NULL;
    }

    /* construct new format descriptor and fill it in */
    cabdesc = g_new( struct cabrillo_desc, 1 );
    cabdesc->name = g_strdup(format);
    cabdesc->item_array = g_ptr_array_new();
    cabdesc->item_count = nrstrings;
    cabdesc->qtc_item_array = NULL;
    cabdesc->qtc_item_count = 0;

    for (i = 0; i < nrstrings; i++) {
	struct line_item *item;

	item = parse_line_entry( list[i] );
	if ( item ) {
	    /* add only well formatted entries */
	    g_ptr_array_add( cabdesc->item_array, item );
	}
    }

    if ( cabdesc->item_array->len != nrstrings ) {
	/* not all entries are ok -> stop processing */
	free_cabfmt( cabdesc );
	g_strfreev( list );
	g_key_file_free( keyfile );
	return NULL;
    }

    g_strfreev( list );

    /* read needed QTC keys */
    list = g_key_file_get_string_list( keyfile, format,
			"QTC", &nrstrings, &error );

    if ( error && error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {

	/* if not found -> stop processing as that key is optional */
	g_error_free( error );

    }
    else {

	/* construct new format descriptor and fill it in */
	cabdesc->qtc_item_array = g_ptr_array_new();
	cabdesc->qtc_item_count = nrstrings;

	for (i = 0; i < nrstrings; i++) {
	    struct line_item *item;

	    item = parse_line_entry( list[i] );
	    if ( item ) {
		/* add only well formatted entries */
		g_ptr_array_add( cabdesc->qtc_item_array, item );
	    }
	}

	if ( cabdesc->qtc_item_array->len != nrstrings ) {
	    /* not all entries are ok -> stop processing */
	    free_cabfmt( cabdesc );
	    g_strfreev( list );
	    g_key_file_free( keyfile );
	    return NULL;
	}
    }

    g_strfreev( list );

    /* possible further entries in format specification may contain information
     * about allowed items for different categories:
     * CONTEST, CATEGORY-OPERATOR, CATEGORY_TRANSMITTER, CATEGORY-POWER,
     * CATEGORY-ASSISTED, CATEGORY-BAND, CATEGORY-MODE, C-STATION, C-TIME.
     * C-OVERLAY
     */

    g_key_file_free( keyfile );

    /* return parsed cabrillo format description */
    return cabdesc;
}
