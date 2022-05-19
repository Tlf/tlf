/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2010-2022      Thomas Beierlein <tb@forth-ev.de>
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
 *
 *              Add spot to bandmap
 *              and prepare spots for cluster
 *
 *--------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "clear_display.h"
#include "get_time.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "splitscreen.h"
#include "ui_utils.h"


/** add call to list of spots
 *
 * format a fake DX spot from call and frequency add it to the spot list
 * and send it to other stations in the LAN
 */
void add_to_spots(char *call, freq_t freq) {

    char spotline[160];
    char spottime[6];

    sprintf(spotline, "DX de TLF-%c:     %9.3f  %s",
	    thisnode, freq / 1000.0, call);
    strcat(spotline, spaces(43));

    format_time(spottime, sizeof(spottime), "%H%MZ");
    strcpy(spotline + 70, spottime);
    strcat(spotline, "\n\n");

    send_lan_message(TLFSPOT, spotline);
    lanspotflg = true;
    addtext(spotline);
    lanspotflg = false;
}

/* ask for spot frequency in kHz  and convert it to Hz */
static freq_t ask_frequency() {
    char frequency[16];

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    mvaddstr(13, 20, "freq. (kHz): ");
    echo();
    getnstr(frequency, 7);
    noecho();
    return atof(frequency) * 1000.0;
}

void add_local_spot(void) {

    if (strlen(hiscall) < 3) {
	return;
    }

    if (!trx_control) {
	freq = ask_frequency();
    }

    add_to_spots(hiscall, freq);

    hiscall[0] = '\0';
}


/* send spot to cluster */

#define MAX_SPOT_AGE 120

/* find last qso record in qso_array, return NULL if no one found */
static struct qso_t *find_last_qso() {
    int i = nr_qsos;
    while (i > 0) {
	struct qso_t *last_qso = g_ptr_array_index(qso_array, i-1);
	if (!last_qso->is_comment) {
	    return last_qso;
	}
	i--;
    }
    return NULL;
}

static bool spot_too_old(const struct qso_t *qso) {
    return ((get_time() - qso->timestamp) > MAX_SPOT_AGE);
}

/* Get call and frequency in Hz of spot.
 * First try to use hiscall as spot call. If empty use data from last qso if
 * not to old.
 * If no frequency data available ask user.
 * Returns true if valid spot data found.
 */
static bool get_spot_data(char **spot_call, freq_t *spot_freq) {
    if (strlen(hiscall) > 2) {
	*spot_call = g_strdup(hiscall);
	if (trx_control) {
	    *spot_freq = freq;
	} else {
	    *spot_freq = ask_frequency();
	}
    } else {

	struct qso_t *last_qso = find_last_qso();
	if (last_qso == NULL) {
	    return false;
	}
	if (spot_too_old(last_qso)) {
	    return false;
	}
	*spot_call = g_strdup(last_qso->call);
	if (last_qso->freq < 1.) {	/* not set */
	    *spot_freq = ask_frequency();
	} else {
	    *spot_freq = last_qso->freq;
	}
    }
    return true;
}

static gchar *prepare_spot(void) {
    gchar *spot_line;
    gchar *spot_call;
    freq_t spot_freq;
    if (get_spot_data(&spot_call, &spot_freq)) {
	spot_line = g_strdup_printf("DX %.1f %s ", spot_freq / 1000.,
		spot_call);
	g_free(spot_call);
    } else {
	beep();
	spot_line = g_strdup("");
    }
    return spot_line;
}

void show_spot_line(char *line) {

    clear_line(LINES - 1);
    mvprintw(LINES - 1, 0, "> %s", line);
    move(LINES - 1, strlen(line) + 2);
    refreshp();
}

#define MAX_SPOT_LEN 70

/* allow simple editing of spot line (add, delete characters)
 * \return   true  if ok to send
 *           fasle if escaped
 */
static bool complete_spot(gchar **line) {
    int c = 0;
    int pos;
    char buffer[MAX_SPOT_LEN + 1];
    g_strlcpy(buffer, *line, MAX_SPOT_LEN + 1);
    pos = strlen(buffer);

    show_spot_line(buffer);
    wmove(stdscr, LINES - 1, strlen(buffer) + 2);
    while ((c = key_get()) != ESCAPE && c != '\n' && c != KEY_ENTER) {
	/* eval key and change line */
	switch (c) {
	    case DELETE:
	    case KEY_BACKSPACE:
		if (pos > 0) {
		    pos--;
		    buffer[pos] = '\0';
		}
	    default:
		if (isprint(c) && (pos < MAX_SPOT_LEN)) {
		    buffer[pos++] = c;
		    buffer[pos] = '\0';
		}
	}
	show_spot_line(buffer);
	wmove(stdscr, LINES - 1, strlen(buffer) + 2);
    }

    g_free (*line);
    *line = g_strdup(buffer);

    return c != ESCAPE;
}


void add_cluster_spot(void) {
    gchar *spot_line = prepare_spot();
    bool ok;

    if (strlen(spot_line) > 0) {
	ok = complete_spot(&spot_line);
	if (ok) {
	    gchar *line = g_strconcat(spot_line, "\n", NULL);
	    send_to_cluster(line);
	    g_free(line);
	}
    }
    g_free (spot_line);
}
