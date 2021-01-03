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
 *   Cabrillo utils file
 *
 *--------------------------------------------------------------*/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cabrillo_utils.h"
#include "nicebox.h"
#include "parse_logcfg.h"
#include "showscore.h"


/* conversion table between tag name in format file and internal tag */
static const struct tag_conv tag_tbl[] = {
    { "FREQ",	FREQ 	},
    { "MODE",	MODE 	},
    { "DATE",	DATE 	},
    { "TIME", 	TIME 	},
    { "MYCALL", MYCALL 	},
    { "HISCALL", HISCALL },
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
enum tag_t translate_item_name(char *name) {
    int i;

    /* lookup name in tag list */
    for (i = 0; i < G_N_ELEMENTS(tag_tbl); i++) {
	if (strcmp(tag_tbl[i].item_name, name) == 0) {
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
	for (i = 0; i < desc->item_array->len; i++) {
	    g_free(g_ptr_array_index(desc->item_array, i));
	}

	g_ptr_array_free(desc->item_array, TRUE);
    }

    if (desc->qtc_item_array) {
	for (i = 0; i < desc->qtc_item_array->len; i++) {
	    g_free(g_ptr_array_index(desc->qtc_item_array, i));
	}

	g_ptr_array_free(desc->qtc_item_array, TRUE);
    }

    FREE_DYNAMIC_STRING(desc->name);
    FREE_DYNAMIC_STRING(desc->exchange_separator);
    g_free(desc);
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

    item = g_malloc(sizeof(struct line_item));
    parts = g_strsplit(line_entry, ",", 2);

    if (g_strv_length(parts) == 2) {
	tag = translate_item_name(parts[0]);

	item->tag = tag;
	item->len = atoi(parts[1]);
    } else {
	/* type is NO_ITEM */
	item->tag = NO_ITEM;
    }

    g_strfreev(parts);

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
struct cabrillo_desc *read_cabrillo_format(char *filename, char *format) {
    GKeyFile *keyfile;
    GError *error = NULL;
    gchar **list;
    gsize nrstrings;
    struct cabrillo_desc *cabdesc;
    int i;

    keyfile = g_key_file_new();

    if (!g_key_file_load_from_file(keyfile, filename,
				   G_KEY_FILE_NONE, &error)) {
	g_error_free(error);

	/* file does not exist or is wrongly formatted */
	g_key_file_free(keyfile);
	return NULL;
    }

    /* check if 'format' defined in file */
    if (g_key_file_has_group(keyfile, format) == FALSE) {

	/* group not found */
	g_key_file_free(keyfile);
	return NULL;
    }

    /* read needed keys */
    list = g_key_file_get_string_list(keyfile, format,
				      "QSO", &nrstrings, &error);

    if (error && error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {

	/* if not found -> stop processing as that key is mandatory */
	g_error_free(error);
	g_key_file_free(keyfile);
	return NULL;
    }

    /* construct new format descriptor and fill it in */
    cabdesc = g_new(struct cabrillo_desc, 1);
    cabdesc->name = g_strdup(format);
    cabdesc->item_array = g_ptr_array_new();
    cabdesc->item_count = nrstrings;
    cabdesc->qtc_item_array = NULL;
    cabdesc->qtc_item_count = 0;

    for (i = 0; i < nrstrings; i++) {
	struct line_item *item;

	item = parse_line_entry(list[i]);
	if (item) {
	    /* add only well formatted entries */
	    g_ptr_array_add(cabdesc->item_array, item);
	}
    }

    if (cabdesc->item_array->len != nrstrings) {
	/* not all entries are ok -> stop processing */
	free_cabfmt(cabdesc);
	g_strfreev(list);
	g_key_file_free(keyfile);
	return NULL;
    }

    g_strfreev(list);

    /* read needed QTC keys */
    list = g_key_file_get_string_list(keyfile, format,
				      "QTC", &nrstrings, &error);

    if (error && error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {

	/* if not found -> stop processing as that key is optional */
	g_error_free(error);
	error = NULL;

    } else {

	/* construct new format descriptor and fill it in */
	cabdesc->qtc_item_array = g_ptr_array_new();
	cabdesc->qtc_item_count = nrstrings;

	for (i = 0; i < nrstrings; i++) {
	    struct line_item *item;

	    item = parse_line_entry(list[i]);
	    if (item) {
		/* add only well formatted entries */
		g_ptr_array_add(cabdesc->qtc_item_array, item);
	    }
	}

	if (cabdesc->qtc_item_array->len != nrstrings) {
	    /* not all entries are ok -> stop processing */
	    free_cabfmt(cabdesc);
	    g_strfreev(list);
	    g_key_file_free(keyfile);
	    return NULL;
	}
    }

    g_strfreev(list);

    cabdesc->exchange_separator = g_key_file_get_string(keyfile, format,
				  "EXCHANGE-SEPARATOR", &error);
    if (error) {
	g_error_free(error);    // clear error
	error = NULL;
    }

    /* possible further entries in format specification may contain information
     * about allowed items for different categories:
     * CONTEST, CATEGORY-OPERATOR, CATEGORY_TRANSMITTER, CATEGORY-POWER,
     * CATEGORY-ASSISTED, CATEGORY-BAND, CATEGORY-MODE, C-STATION, C-TIME.
     * C-OVERLAY
     */

    g_key_file_free(keyfile);

    /* return parsed cabrillo format description */
    return cabdesc;
}

cbr_field_t cabrillo_fields[] = {
    // internals
    {
	.name = CBR_EXCHANGE, .text = "Your exchange",
	.hint = "(e.g. State, province, age etc... (# if serial number))",
	.internal = true
    },
    {
	.name = CBR_QSO_FORMAT,
	.internal = true
    },
    {
	.name = CBR_TEMPLATE,
	.internal = true
    },
    //
    {
	.name = "CONTEST", .text = "Contest",
	.hint = "(CQ-WW-CW/SSB, CQ-WPX-CW/SSB, ARRL-DX-CW/SSB)",
    },
    {
	.name = CBR_CALLSIGN,   // filled internally
    },
    {
	.name = "CATEGORY-ASSISTED", .text = "Category-Assisted",
	.hint = "(ASSISTED, NON-ASSISTED)",
    },
    {
	.name = "CATEGORY-BAND", .text = "Bands",
	.hint = "(ALL,160M,80M,40M,20M,15M,10M)",
    },
    {
	.name = "CATEGORY-MODE", .text = "Mode",
	.hint = "(CW,SSB,RTTY,MIXED)",
    },
    {
	.name = "CATEGORY-OPERATOR", .text = "Category-Operator",
	.hint = "(SINGLE-OP, MULTI-OP, CHECKLOG)",
    },
    {
	.name = "CATEGORY-POWER", .text = "Power",
	.hint = "(HIGH,LOW,QRP)",
    },
    {
	.name = "CATEGORY-STATION", .text = "Category-Station",
	.hint = "(FIXED, MOBILE, PORTABLE, ROVER, EXPEDITION, HQ, SCHOOL)",
	.skip_empty = true
    },
    {
	.name = "CATEGORY-TIME", .text = "Category-Time",
	.hint = "(6-HOURS, 12-HOURS, 24-HOURS)",
	.skip_empty = true
    },
    {
	.name = "CATEGORY-TRANSMITTER", .text = "Transmitter",
	.hint = "(ONE, TWO, LIMITED, UNLIMITED, SWL)",
	.skip_empty = true
    },
    {
	.name = "CATEGORY-OVERLAY", .text = "Category-Overlay",
	.hint = "(ROOKIE, TB-WIRES, NOVICE-TECH, OVER-50)",
	.skip_empty = true
    },
    {
	.name = "CERTIFICATE", .text = "Certificate",
	.hint = "(YES [default], NO)",
	.disabled = true
    },
    {
	.name = CBR_SCORE,      // filled internally
    },
    {
	.name = "CLUB", .text = "Club",
	.skip_empty = true
    },
    {
	.name = "LOCATION", .text = "Location",
	.hint = "(section, IOTA name, RDA, State/Province, ...)",
	.skip_empty = true
    },
    {
	.name = "GRID-LOCATOR", .text = "Locator:",
	.disabled = true
    },
    {
	.name = "NAME", .text = "Operator name",
    },
    {
	.name = "EMAIL", .text = "E-mail",
	.disabled = true
    },
    {
	.name = "ADDRESS", .text = "Address",
    },
    {
	.name = "ADDRESS(2)", .text = "Address(2)",
	.disabled = true
    },
    {
	.name = "ADDRESS(3)", .text = "Address(3)",
	.disabled = true
    },
    {
	.name = "ADDRESS-CITY", .text = "Address-City",
	.disabled = true
    },
    {
	.name = "ADDRESS-STATE-PROVINCE", .text = "Address-State/Province",
	.disabled = true
    },
    {
	.name = "ADDRESS-POSTALCODE", .text = "Address-PostalCode",
	.disabled = true
    },
    {
	.name = "ADDRESS-COUNTRY", .text = "Address-Country",
	.disabled = true
    },
    {
	.name = "OPERATORS", .text = "List of Operators",
	.hint = "(space delimited)",
    },
    {
	.name = "OFFTIME", .text = "Offtime",
	.hint = "(yyyy-mm-dd hhmm yyyy-mm-dd hhmm)",
	.skip_empty = true
    },
    {
	.name = "SOAPBOX", .text = "Soapbox",
	.hint = "(use any text editor to include more lines)",
	.skip_empty = true
    },
    {
	.name = "SOAPBOX(2)", .text = "Soapbox(2)",
	.disabled = true
    },
    {
	.name = "SOAPBOX(3)", .text = "Soapbox(3)",
	.disabled = true
    },
};

cbr_field_t *find_cabrillo_field(const char *name) {
    for (int i = 0; i < G_N_ELEMENTS(cabrillo_fields); ++i) {
	if (strcmp(cabrillo_fields[i].name, name) == 0) {
	    return &cabrillo_fields[i];
	}
    }
    return NULL;
}

int get_cabrillo_field_value(const cbr_field_t *field, char *buffer, int size) {
    if (field == NULL || field->disabled) {
	return -1;      // no such field or disabled
    }
    if (field->value != NULL && !field->value_is_hint) {
	g_strlcpy(buffer, field->value, size);    // use specified value
	return 0;
    }

    if (field->text == NULL) {
	return -1;      // missing value, no text, don't ask
    }

    // ask if no value was provided
    const char *hint;
    if (field->value != NULL) {
	hint = field->value;
    } else if (field->hint != NULL) {
	hint = field->hint;
    } else {
	hint = "";
    }

    char *prompt = g_strdup_printf("%s: %s", field->text, hint);
    if (strlen(prompt) > 76) {
	prompt[76] = 0;
	prompt[75] = '.';
	prompt[74] = '.';
	prompt[73] = '.';
    }

    char input[80];
    ask(input, prompt);
    g_free(prompt);

    g_strstrip(input);
    g_strlcpy(buffer, input, size);

    if (strlen(buffer) == 0 && field->skip_empty) {
	return -1;       // skip empty value
    }

    return 0;
}


void write_cabrillo_header(FILE *fp) {
    char buffer[80];

    // set CALLSIGN
    cbr_field_t *call = find_cabrillo_field(CBR_CALLSIGN);
    strcpy(buffer, my.call);
    g_strstrip(buffer);     // remove trailing NL
    FREE_DYNAMIC_STRING(call->value);
    call->value = g_strdup(buffer);
    call->disabled = false; // always enabled

    // set CLAIMED-SCORE unless disabled
    cbr_field_t *score = find_cabrillo_field(CBR_SCORE);
    if (!score->disabled) {
	FREE_DYNAMIC_STRING(score->value);
	score->value = g_strdup_printf("%d", get_total_score());
    }

    // set GRID-LOCATOR if enabled and my.qra is not empty
    cbr_field_t *locator = find_cabrillo_field(CBR_LOCATOR);
    if (!locator->disabled && locator->value == NULL && my.qra[0] != 0) {
	locator->value = g_strdup(my.qra);
    }

    fprintf(fp, "START-OF-LOG: 3.0\n");
    fprintf(fp, "CREATED-BY: %s\n", argp_program_version);
    format_time(buffer, sizeof(buffer), CREATED_DATE_TIME_FORMAT);
    fprintf(fp, "X-CREATED-ON: %s\n", buffer);

    for (int i = 0; i < G_N_ELEMENTS(cabrillo_fields); ++i) {
	if (cabrillo_fields[i].internal) {
	    continue;       // internal use only
	}

	if (get_cabrillo_field_value(&cabrillo_fields[i], buffer, 80) != 0) {
	    continue;       // has no value
	}

	// cut index from name, e.g. ADDRESS(2)
	char tag[40];
	strcpy(tag, cabrillo_fields[i].name);
	char *p = strchr(tag, '(');
	if (p != NULL) {
	    *p = 0;
	}
	fprintf(fp, "%s: %s\n", tag, buffer);
    }
}

static int process_cabrillo_template_file(const char *file_name);

// called from parse_logcfg.c and process_cabrillo_template_file()
int add_cabrillo_field(const char *name, const char *value) {
    cbr_field_t *field = find_cabrillo_field(name);
    if (field == NULL) {
	return PARSE_NO_MATCH;
    }

    // special cases:
    //      1) QSO-FORMAT: set cabrillo variable
    if (strcmp(name, CBR_QSO_FORMAT) == 0) {
	FREE_DYNAMIC_STRING(cabrillo);
	cabrillo = g_strdup(value);
	return PARSE_OK;
    }
    //      2) TEMPLATE: read fields from Cabrillo file
    if (strcmp(name, CBR_TEMPLATE) == 0) {
	return process_cabrillo_template_file(value);
    }

    field->disabled = false;
    FREE_DYNAMIC_STRING(field->value);

    if (value == NULL) {
	return PARSE_OK;    // missing value just enables field
    }

    int len = strlen(value);
    field->value_is_hint = (len > 1 && value[0] == '(' && value[len - 1] == ')');
    field->disabled = (len == 1 && value[0] == '-');
    if (!field->disabled) {
	field->value = g_strdup(value);
    }

    return PARSE_OK;
}

static bool skip_template_line(const char *line) {
    static const char *skips[] = {
	"^$", "^#", "^X-",      // empty line or comment
	"^(START|END)-OF-LOG:",
	"^CREATED-BY:",
	"^CALLSIGN:",
	"^QSO:",
	"^" CBR_TEMPLATE ":"    // no recursion
    };

    for (int i = 0; i < G_N_ELEMENTS(skips); ++i) {
	if (g_regex_match_simple(skips[i], line, 0, 0)) {
	    return true;
	}
    }

    return false;
}

static int process_cabrillo_template_file(const char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
	error_details = g_strdup_printf("can't open '%s'", file_name);
	return PARSE_WRONG_PARAMETER;
    }

    char logline[MAX_CABRILLO_LEN];

    while (fgets(logline, MAX_CABRILLO_LEN, fp) != NULL) {
	g_strstrip(logline);
	if (skip_template_line(logline)) {
	    continue;   // skip it
	}
	char **fields = g_strsplit(logline, ":", 2);
	g_strstrip(fields[0]);
	if (g_strv_length(fields) == 2) {
	    g_strstrip(fields[1]);
	}

	int rc = add_cabrillo_field(fields[0], fields[1]);
	g_strfreev(fields);

	if (rc != PARSE_OK) {
	    fclose(fp);
	    error_details = g_strdup_printf("unknown tag '%s'", fields[0]);
	    return PARSE_ERROR;
	}
    }

    fclose(fp);

    return PARSE_OK;
}
