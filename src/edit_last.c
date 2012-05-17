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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	/* ------------------------------------------------------------
	 *   edit the 5 latest qsos
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "edit_last.h"
#include <glib.h>
#include <assert.h>

#define NR_LINES 5
#define NR_COLS 80


/* highlite the edit line and set the cursor */
static void highlite_line(int row, char *line, int column)
{
    char ln[NR_COLS+1];

    g_strlcpy (ln, line, NR_COLS+1);
    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvprintw(7 + row, 0, ln);
    mvprintw(7 + row, column, "");
    refreshp();
}

/* reset the highlite state */
static void unhighlite_line(int row, char *line)
{
    char ln[81];

    g_strlcpy (ln, line, NR_COLS+1);
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvprintw(7 + row, 0, ln);
}


/* get a copy of selected QSO into the buffer */
void get_qso(int nr, char *buffer) 
{
    assert (nr < nr_qsos);
    strcpy (buffer, qsos[nr]);
    assert (strlen(buffer) == (LOGLINELEN - 1));
}

/* save editbuffer back to log */
void putback_qso (int nr, char *buffer)
{
    FILE *fp;

    assert (strlen(buffer) == (LOGLINELEN - 1));
    assert (nr < nr_qsos);

    if ((fp = fopen(logfile, "r+")) == NULL) {
	mvprintw(24, 0, "Can not open logfile...");
	refreshp();
	sleep(2);
    }else {
	fseek(fp, nr * LOGLINELEN, SEEK_SET);
	fputs(buffer, fp);
	fputs("\n", fp);

	fclose(fp);

	strcpy(qsos[nr], buffer);
    }
}


void edit_last(void)
{

    int j = 0, b, k;
    int editline = NR_LINES-1;
    char editbuffer[LOGLINELEN+1];

    if (nr_qsos == 0)
	return;			/* nothing to edit */

    stop_backgrnd_process = 1;	//(no qso add during edit process)

    b = 29;

    /* start with last QSO */
    get_qso (nr_qsos - (NR_LINES - editline), editbuffer);
    highlite_line(editline, editbuffer, b);

    while ((j != 27) && (j != '\n')) {

	j = onechar();

	if (j == 1) {		// ctrl A, beginning of line
	    b = 1;
	    highlite_line(editline, editbuffer, b);

	} else if (j == 5) {	// ctrl E, end of line
	    b = 77;
	    highlite_line(editline, editbuffer, b);

	} else if (j == 9) {	// TAB, next field
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

	    highlite_line(editline, editbuffer, b);

	} else if (j == 152) {	// up
	    if (editline > (NR_LINES - nr_qsos) && (editline > 0)) {
		unhighlite_line(editline, editbuffer);
		putback_qso (nr_qsos - (NR_LINES -editline), editbuffer);
		editline--;
		get_qso (nr_qsos - (NR_LINES - editline), editbuffer);
		highlite_line(editline, editbuffer, b);
	    } else {
		logview();
		j = 27;
	    }

	} else if (j == 153) {	// down

	    if (editline < NR_LINES-1) {
		unhighlite_line(editline, editbuffer);
		putback_qso (nr_qsos - (NR_LINES -editline), editbuffer);
		editline++;
		get_qso (nr_qsos - (NR_LINES - editline), editbuffer);
		highlite_line(editline, editbuffer, b);
	    } else
		j = 27;		/* escape */

	} else if (j == 155) {  // left

	    if (b >= 1)
		b--;

	    highlite_line(editline, editbuffer, b);

	} else if (j == 154) {	// right
	    if (b < 79)
		b++;

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 160) && (b >= 0) && (b < 28)) {	// insert

	    for (k = 28; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 160) && (b >= 29) && (b < 39)) {	// insert  call
	    for (k = 39; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 160) && (b >= 54) && (b < 64)) {	// insert

	    for (k = 64; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 160) && (b >= 68) && (b < 76)) {	// insert

	    for (k = 76; k > b; k--)
		editbuffer[k] = editbuffer[k - 1];
	    editbuffer[b] = ' ';

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 161) && (b >= 1) && (b < 28)) {	// delete

	    for (k = b; k < 28; k++)
		editbuffer[k] = editbuffer[k + 1];

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 161) && (b >= 29) && (b < 39)) {	// delete

	    for (k = b; k < 39; k++)
		editbuffer[k] = editbuffer[k + 1];

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 161) && (b >= 68) && (b < 76)) {	// delete

	    for (k = b; k < 76; k++)
		editbuffer[k] = editbuffer[k + 1];

	    highlite_line(editline, editbuffer, b);

	} else if ((j == 161) && (b >= 54) && (b < 64)) {	// delete

	    for (k = b; k < 64; k++)
		editbuffer[k] = editbuffer[k + 1];

	    highlite_line(editline, editbuffer, b);

	} else if (j != 27) {

	    if ((j >= 97) && (j <= 122))
		j = j - 32;

	    if ((j >= 32) && (j < 97)) {
		editbuffer[b] = j;
		mvprintw(7 + editline, 0, editbuffer);
		if ((b < strlen(editbuffer) - 2) && (b < 80))
		    b++;
		mvprintw(7 + editline, b, "");
	    }
	}
    }

    unhighlite_line(editline, editbuffer);
    putback_qso (nr_qsos - (NR_LINES -editline), editbuffer);

    scroll_log();

    stop_backgrnd_process = 0;
}
