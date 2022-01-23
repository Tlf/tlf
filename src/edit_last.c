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

#define SOTIME 17	    /* start of time field */
#define SOCALL 29	    /* start of call field */
#define SOEXCH 54	    /* start of exchange field */
#define EOEXCH (54 + contest->exchange_width) /* end of last field */

typedef struct {
    int start;
    int end;
    bool tab;
} field_t;

static field_t fields[] = {
    {.start = 0,  .end = 4,  .tab = true},  // band+mode
    {.start = 7,  .end = 15},   // date
    {.start = 17, .end = 21, .tab = true},  // time
    {.start = 23, .end = 26},   // number
    {.start = 29, .end = 29 + MAX_CALL_LENGTH - 1, .tab = true},   // call
    {.start = 44, .end = 46},   // sent RST
    {.start = 49, .end = 52},   // rcvd RST
    {.start = 54,            .tab = true},  // exchange -- end set at runtime
};

#define FIELD_INDEX_CALL        4
#define FIELD_INDEX_EXCHANGE    (G_N_ELEMENTS(fields) - 1)

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
void get_qso(int nr, char *buffer) {

    assert(nr < nr_qsos);
    strcpy(buffer, qsos[nr]);
    assert(strlen(buffer) == (LOGLINELEN - 1));
}

/* save editbuffer back to log */
void putback_qso(int nr, char *buffer) {
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


void edit_last(void) {

    bool changed = false, needs_rescore = false;
    int j = 0, b, k;
    int editline = NR_LINES - 1;
    char editbuffer[LOGLINELEN + 1];

    if (nr_qsos == 0)
	return;			/* nothing to edit */

    stop_background_process();

    fields[FIELD_INDEX_EXCHANGE].end = EOEXCH - 1;  // set current end of exchange

    int field_index = FIELD_INDEX_CALL;
    b = fields[field_index].start;

    /* start with last QSO */
    get_qso(nr_qsos - (NR_LINES - editline), editbuffer);

    while (j != ESCAPE && j != '\n' && j != KEY_ENTER) {
	highlight_line(editline, editbuffer, b);

	j = key_get();

	// Ctrl-A (^A) or <Home>, beginning of line.
	if (j == CTRL_A || j == KEY_HOME) {
	    b = 1;

	    // Ctrl-E (^E) or <End>, end of line.
	} else if (j == CTRL_E || j == KEY_END) {
	    b = EOEXCH - 1;

	    // <Tab>, switch to next tab field.
	} else if (j == TAB) {
            do {
                field_index = (field_index + 1) % G_N_ELEMENTS(fields);
            }
            while (!fields[field_index].tab);

            b = fields[field_index].start;

	    // Up arrow, move to previous line.
	} else if (j == KEY_UP) {
	    if (editline > (NR_LINES - nr_qsos) && (editline > 0)) {
		unhighlight_line(editline, editbuffer);
		if (changed) {
		    putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);
		    needs_rescore = true;
		    changed = false;
		}
		editline--;
		get_qso(nr_qsos - (NR_LINES - editline), editbuffer);
	    } else {
		logview();
		j = ESCAPE;
	    }

	    // Down arrow, move to next line.
	} else if (j == KEY_DOWN) {

	    if (editline < NR_LINES - 1) {
		unhighlight_line(editline, editbuffer);
		if (changed) {
		    putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);
		    needs_rescore = true;
		    changed = false;
		}
		editline++;
		get_qso(nr_qsos - (NR_LINES - editline), editbuffer);
	    } else
		j = ESCAPE;

	    // Left arrow, move cursor one position left.
	} else if (j == KEY_LEFT) {
	    if (b > fields[field_index].start) {
		b--;
            } else if (field_index > 0) {
                --field_index;
                b = fields[field_index].end;
            }

	    // Right arrow, move cursor one position right.
	} else if (j == KEY_RIGHT) {
	    if (b < fields[field_index].end) {
		b++;
            } else if (field_index < G_N_ELEMENTS(fields) - 1) {
                ++field_index;
                b = fields[field_index].start;
            }

	    // <Insert>, insert a space
	} else if (j == KEY_IC) {
	    for (k = fields[field_index].end; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';
	    changed = true;

	    // <Delete>, shift rest left
	} else if (j == KEY_DC) {
	    for (k = b; k < fields[field_index].end; k++)
		editbuffer[k] = editbuffer[k + 1];
	    editbuffer[fields[field_index].end] = ' ';
	    changed = true;

	} else if (j != ESCAPE) {

	    // Promote lower case to upper case.
	    if ((j >= 97) && (j <= 122))
		j = j - 32;

	    // Accept most all printable characters.
	    if ((j >= 32) && (j < 97)) {
		editbuffer[b] = j;
                if (b < fields[field_index].end) {
		    b++;
                }
		changed = true;
	    }
	}
    }

    unhighlight_line(editline, editbuffer);
    if (changed) {
	putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);
	needs_rescore = true;
	changed = false;
    }
    if (needs_rescore) {
	log_read_n_score();
    }

    scroll_log();

    start_background_process();
}
