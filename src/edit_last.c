/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012           Thomas Beierlein <tb@forth-ev.de>
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

/* ------------------------------------------------------------
 *   edit the 5 latest qsos
 *--------------------------------------------------------------*/


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "background_process.h"
#include "err_utils.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "keystroke_names.h"
#include "logview.h"
#include "readcalls.h"
#include "scroll_log.h"
#include "tlf_curses.h"
#include "ui_utils.h"

#define NR_LINES 5
#define NR_COLS 80

typedef struct {
    int start;
    int end;
    bool tab;
    const char *pattern;
} field_t;

static field_t fields[] = {
    {.start = 0,  .end = 5, .tab = true,    // band+mode
        .pattern = "^[ 1]\\d{2}(CW |SSB|DIG)$"},
    {.start = 7,  .end = 15,                // date
        .pattern = "^\\d{2}-[A-Z]{3}-\\d{2}$"},
    {.start = 17, .end = 21, .tab = true,   // time
        .pattern = "^\\d{2}:\\d{2}$"},
    {.start = 23, .end = 26,                // number
        .pattern = "^\\s*\\d+\\s*$"},
    {.start = 29, .end = 29 + MAX_CALL_LENGTH - 1, .tab = true,   // call
        .pattern = "^\\s*\\S+\\s*$"},
    {.start = 44, .end = 46,                // sent RST
        .pattern = "^\\s*\\d+\\s*$"},
    {.start = 49, .end = 52,                // rcvd RST
        .pattern = "^\\s*\\d+\\s*$"},
    {.start = 54,            .tab = true},  // exchange -- end set at runtime
};

#define FIELD_INDEX_CALL        4
#define FIELD_INDEX_EXCHANGE    (G_N_ELEMENTS(fields) - 1)


static char editbuffer[LOGLINELEN + 1];
static bool changed, needs_rescore;
static int editline;
static int field_index;
static field_t *current_field;  // points to fields[field_index]

/* highlight the edit line and set the cursor */
static void highlight_line(int row, char *line, int column) {

    char ln[NR_COLS + 1];

    g_strlcpy(ln, line, NR_COLS + 1);
    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvaddstr(7 + row, 0, ln);
    move(7 + row, column);
    refreshp();
}

/* reset the highlight state */
static void unhighlight_line(int row, char *line) {

    char ln[NR_COLS + 1];

    g_strlcpy(ln, line, NR_COLS + 1);
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvaddstr(7 + row, 0, ln);
}


/* get a copy of selected QSO into the buffer */
static void get_qso(int nr, char *buffer) {

    assert(nr < nr_qsos);
    strcpy(buffer, qsos[nr]);
    assert(strlen(buffer) == (LOGLINELEN - 1));
}

/* save editbuffer back to log */
static void putback_qso(int nr, char *buffer) {
    FILE *fp;

    assert(strlen(buffer) == (LOGLINELEN - 1));
    assert(nr < nr_qsos);

    if ((fp = fopen(logfile, "r+")) == NULL) {
	TLF_LOG_WARN("Can not open logfile...");
    } else {
	fseek(fp, (long)nr * LOGLINELEN, SEEK_SET);
	fputs(buffer, fp);
	fputs("\n", fp);

	fclose(fp);

	strcpy(qsos[nr], buffer);
    }
}

static bool field_valid() {
    if (!changed || current_field->pattern == NULL) {
        return true;
    }

    // extract current field value
    char value[NR_COLS + 1];
    int width = current_field->end - current_field->start + 1;
    g_strlcpy(value, editbuffer + current_field->start, width + 1);

    return g_regex_match_simple(current_field->pattern, value, G_REGEX_CASELESS, 0);
}


static void check_store_and_get_next_line(int direction) {
    if (!field_valid()) {
        return;     // field is not valid, no action needed
    }
    unhighlight_line(editline, editbuffer);
    if (changed) {
        putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);
        needs_rescore = true;
        changed = false;
    }
    if (direction != 0) {
        editline += direction;
        get_qso(nr_qsos - (NR_LINES - editline), editbuffer);
    }
}


void edit_last(void) {

    int j, b, k;

    if (nr_qsos == 0)
	return;			/* nothing to edit */

    stop_background_process();  // note: this freezes nr_qsos, as network is paused

    const int topline = MAX(NR_LINES - nr_qsos, 0);

    // set current end of exchange field
    fields[FIELD_INDEX_EXCHANGE].end = fields[FIELD_INDEX_EXCHANGE].start + contest->exchange_width - 1;

    field_index = FIELD_INDEX_CALL;
    current_field = &fields[field_index];
    b = current_field->start;

    /* start with last QSO */
    editline = NR_LINES - 1;
    get_qso(nr_qsos - (NR_LINES - editline), editbuffer);
    changed = false;
    needs_rescore = false;

    while (true) {
	highlight_line(editline, editbuffer, b);

	j = key_get();

	// Ctrl-A (^A) or <Home>, beginning of line.
	if (j == CTRL_A || j == KEY_HOME) {
            if (field_valid()) {
                b = 0;
            }

        // Ctrl-E (^E) or <End>, end of exchange field.
	} else if (j == CTRL_E || j == KEY_END) {
            if (field_valid()) {
                b = fields[FIELD_INDEX_EXCHANGE].end;
            }

        // <Tab>, switch to next tab field.
	} else if (j == TAB) {
            if (field_valid()) {
                do {
                    field_index = (field_index + 1) % G_N_ELEMENTS(fields);
                }
                while (!fields[field_index].tab);

                current_field = &fields[field_index];
                b = current_field->start;
            }

        // Up arrow, move to previous line.
	} else if (j == KEY_UP) {
	    if (editline > topline) {
                check_store_and_get_next_line(-1);
	    } else {
                if (field_valid()) {
                    logview();
                    j = ESCAPE;     // signal exit
                }
	    }

        // Down arrow, move to next line.
	} else if (j == KEY_DOWN) {

	    if (editline < NR_LINES - 1) {
                check_store_and_get_next_line(+1);
	    } else {
                j = ESCAPE;     // signal exit
            }

        // Left arrow, move cursor one position left.
	} else if (j == KEY_LEFT) {
	    if (b > current_field->start) {
		b--;
            } else if (field_valid() && field_index > 0) {
                --field_index;
                current_field = &fields[field_index];
                b = current_field->end;
            }

        // Right arrow, move cursor one position right.
	} else if (j == KEY_RIGHT) {
	    if (b < current_field->end) {
		b++;
            } else if (field_valid() && field_index < G_N_ELEMENTS(fields) - 1) {
                ++field_index;
                current_field = &fields[field_index];
                b = current_field->start;
            }

        // <Insert>, insert a space
	} else if (j == KEY_IC) {
	    for (k = current_field->end; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';
	    changed = true;

        // <Delete>, shift rest left
	} else if (j == KEY_DC) {
	    for (k = b; k < current_field->end; k++)
		editbuffer[k] = editbuffer[k + 1];
	    editbuffer[current_field->end] = ' ';
	    changed = true;

	} else {

	    // Promote lower case to upper case.
	    if (j >= 'a' && j <= 'z')
		j = j - 'a' + 'A';

	    // Accept most all printable characters.
	    if (j >= ' ' && j < 'a') {
		editbuffer[b] = j;
                if (b < current_field->end) {
		    b++;
                }
		changed = true;
	    }
	}

        // Check exit
        if (j == ESCAPE || j == '\n' || j == KEY_ENTER) {
            if (field_valid()) {
                break;      // exit loop
            }
        }
    }

    check_store_and_get_next_line(0);

    if (needs_rescore) {
	log_read_n_score();
    }

    scroll_log();

    start_background_process();
}
