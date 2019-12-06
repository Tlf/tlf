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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#include "scroll_log.h"
#include "tlf_curses.h"
#include "ui_utils.h"

#define NR_LINES 5
#define NR_COLS 80


/* highlight the edit line and set the cursor */
static void highlight_line(int row, char *line, int column) {

    char ln[NR_COLS + 1];

    g_strlcpy(ln, line, NR_COLS + 1);
    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvprintw(7 + row, 0, ln);
    mvprintw(7 + row, column, "");
    refreshp();
}

/* reset the highlight state */
static void unhighlight_line(int row, char *line) {

    char ln[NR_COLS + 1];

    g_strlcpy(ln, line, NR_COLS + 1);
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvprintw(7 + row, 0, ln);
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

    int j = 0, b, k;
    int editline = NR_LINES - 1;
    char editbuffer[LOGLINELEN + 1];

    if (nr_qsos == 0)
	return;			/* nothing to edit */

    stop_background_process();

    b = 29;

    /* start with last QSO */
    get_qso(nr_qsos - (NR_LINES - editline), editbuffer);

    while (j != 27 && j != '\n' && j != KEY_ENTER) {
	highlight_line(editline, editbuffer, b);

	j = key_get();

	// Ctrl-A (^A) or <Home>, beginning of line.
	if (j == CTRL_A || j == KEY_HOME) {
	    b = 1;

	    // Ctrl-E (^E) or <End>, end of line.
	} else if (j == CTRL_E || j == KEY_END) {
	    b = 77;

	    // <Tab>, next field.
	} else if (j == TAB) {
	    if (b < 17)
		b = 17;
	    else if (b < 29)
		b = 29;
	    else if (b < 54)
		b = 54;
	    else if (b < 68)
		b = 68;
	    else if (b < 77)
		b = 77;
	    else
		b = 1;

	    // Up arrow, move to previous line.
	} else if (j == KEY_UP) {
	    if (editline > (NR_LINES - nr_qsos) && (editline > 0)) {
		unhighlight_line(editline, editbuffer);
		putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);
		editline--;
		get_qso(nr_qsos - (NR_LINES - editline), editbuffer);
	    } else {
		logview();
		j = 27;
	    }

	    // Down arrow, move to next line.
	} else if (j == KEY_DOWN) {

	    if (editline < NR_LINES - 1) {
		unhighlight_line(editline, editbuffer);
		putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);
		editline++;
		get_qso(nr_qsos - (NR_LINES - editline), editbuffer);
	    } else
		j = 27;		/* escape */

	    // Left arrow, move cursor one position left.
	} else if (j == KEY_LEFT) {
	    if (b >= 1)
		b--;

	    // Right arrow, move cursor one position right.
	} else if (j == KEY_RIGHT) {
	    if (b < 79)
		b++;

	    // <Insert>, positions 0 to 27.
	} else if ((j == KEY_IC) && (b >= 0) && (b < 28)) {
	    for (k = 28; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    // <Insert>, positions 29 to 38.
	} else if ((j == KEY_IC) && (b >= 29) && (b < 39)) {
	    for (k = 39; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    // <Insert>, positions 54 to 63.
	} else if ((j == KEY_IC) && (b >= 54) && (b < 64)) {
	    for (k = 64; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    // <Insert>, positions 68 to 75.
	} else if ((j == KEY_IC) && (b >= 68) && (b < 76)) {
	    for (k = 76; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    // <Delete>, positions 1 to 27.
	} else if ((j == KEY_DC) && (b >= 1) && (b < 28)) {
	    for (k = b; k < 28; k++)
		editbuffer[k] = editbuffer[k + 1];

	    // <Delete>, positions 29 to 38.
	} else if ((j == KEY_DC) && (b >= 29) && (b < 39)) {
	    for (k = b; k < 39; k++)
		editbuffer[k] = editbuffer[k + 1];

	    // <Delete>, positions 68 to 75.
	} else if ((j == KEY_DC) && (b >= 68) && (b < 76)) {
	    for (k = b; k < 76; k++)
		editbuffer[k] = editbuffer[k + 1];

	    // <Delete>, positions 54 to 63.
	} else if ((j == KEY_DC) && (b >= 54) && (b < 64)) {
	    for (k = b; k < 64; k++)
		editbuffer[k] = editbuffer[k + 1];

	} else if (j != 27) {

	    // Promote lower case to upper case.
	    if ((j >= 97) && (j <= 122))
		j = j - 32;

	    // Accept most all printable characters.
	    if ((j >= 32) && (j < 97)) {
		editbuffer[b] = j;
		if ((b < strlen(editbuffer) - 2) && (b < 80))
		    b++;
	    }
	}
    }

    unhighlight_line(editline, editbuffer);
    putback_qso(nr_qsos - (NR_LINES - editline), editbuffer);

    scroll_log();

    start_background_process();
}
