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

    b = SOCALL;

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

	    // <Tab>, next field.
	} else if (j == TAB) {
	    if (b < SOTIME)
		b = SOTIME;
	    else if (b < SOCALL)
		b = SOCALL;
	    else if (b < SOEXCH)
		b = SOEXCH;
	    else
		b = 1;

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
	    if (b >= 1)
		b--;

	    // Right arrow, move cursor one position right.
	} else if (j == KEY_RIGHT) {
	    if (b < EOEXCH - 1)
		b++;

	    // <Insert>, positions 0 to 26.
	} else if ((j == KEY_IC) && (b >= 0) && (b < 27)) {
	    for (k = 26; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';
	    changed = true;

	    // <Insert>, positions 29 to 40.
	} else if ((j == KEY_IC) && (b >= 29) && (b < 40)) {
	    for (k = 40; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';
	    changed = true;

	    // <Insert>, positions 54 to end of field.
	} else if ((j == KEY_IC) && (b >= SOEXCH) && (b < EOEXCH - 1)) {
	    for (k = EOEXCH - 1; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';
	    changed = true;

	    // <Delete>, positions 1 to 27.
	} else if ((j == KEY_DC) && (b >= 1) && (b < 28)) {
	    for (k = b; k < 28; k++)
		editbuffer[k] = editbuffer[k + 1];
	    changed = true;

	    // <Delete>, positions 29 to 40.
	} else if ((j == KEY_DC) && (b >= 29) && (b < 41)) {
	    for (k = b; k < 40; k++)
		editbuffer[k] = editbuffer[k + 1];
	    editbuffer[40] = ' ';
	    changed = true;

	    // <Delete>, positions 54 to 63.
	} else if ((j == KEY_DC) && (b >= SOEXCH) && (b < EOEXCH)) {
	    for (k = b; k < EOEXCH - 1; k++)
		editbuffer[k] = editbuffer[k + 1];
	    editbuffer[EOEXCH - 1] = ' ';
	    changed = true;

	} else if (j != ESCAPE) {

	    // Promote lower case to upper case.
	    if ((j >= 97) && (j <= 122))
		j = j - 32;

	    // Accept most all printable characters.
	    if ((j >= 32) && (j < 97)) {
		editbuffer[b] = j;
		if ((b < strlen(editbuffer) - 2) && (b < EOEXCH - 1))
		    b++;
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
