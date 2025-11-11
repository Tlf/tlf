/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2015 Thomas Beierlein <tb@forth-ev.de>
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

/* User Interface helpers for ncurses based user interface */


#include <pthread.h>
#include <unistd.h>
#include <glib.h>

#include "clear_display.h"
#include "clusterinfo.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "stoptx.h"
#include "tlf_panel.h"
#include "showmsg.h"
#include "splitscreen.h"

extern int ymax, xmax;

static int key_kNXT3 = 0;
static int key_kPRV3 = 0;
static int key_kNXT5 = 0;
static int key_kPRV5 = 0;

static pthread_mutex_t panel_mutex = PTHREAD_MUTEX_INITIALIZER;

static const bool WAIT = true;
static const bool NO_WAIT = false;

static int getkey(bool wait);
static int onechar(void);
void resize_layout(void);

/** fake refresh code to use update logic for panels */
void refreshp() {
    pthread_mutex_lock(&panel_mutex);
    update_panels();
    doupdate();
    pthread_mutex_unlock(&panel_mutex);
}


/** add A_BOLD to attributes if 'use_rxvt' is not set */
int modify_attr(int attr) {

    if (!use_rxvt)
	attr |= A_BOLD;

    return attr;
}


/** lookup key code by capability name
 *
 * ncurses automatically maps extended capabilities (such as kNXT3 or similar)
 * to keycodes. But there is no fixed ordering for that.
 * So we look up the key code by its name on strtup and use that afterwards.
 * \param capability  - capability name
 * \return              keycode or 0 if no associated key found
 */
static int lookup_key(char *capability) {
    char *esc_sequence = NULL;
    int keycode = 0;

    if (*capability == '\0') {
	return 0;
    }

    esc_sequence = tigetstr(capability);

    if (esc_sequence == NULL || esc_sequence == (char *) - 1) {
	return 0;
    }

    keycode = key_defined(esc_sequence);

    return keycode;
}

/** lookup all needed special keys not defined by ncurses */
void lookup_keys() {
    bool alt_not_ok = false;
    bool ctrl_not_ok = false;

    key_kNXT3 = lookup_key("kNXT3");
    key_kPRV3 = lookup_key("kPRV3");
    key_kNXT5 = lookup_key("kNXT5");
    key_kPRV5 = lookup_key("kPRV5");

    if ((key_kNXT3 || key_kPRV3) == 0) {
	showmsg("Terminal does not support Alt-PgUp/PgDn keys");
	alt_not_ok = true;
    }

    if ((key_kNXT5 || key_kPRV5) == 0) {
	showmsg("Terminal does not support Ctrl-PgUp/PgDn keys");
	ctrl_not_ok = true;
    }

    if (alt_not_ok && ctrl_not_ok) {
	showmsg("See ':CQD' in man page for setting Auto_CQ delay");
	showmsg("");
	beep();
	sleep(2);
    }
}

/** key_get  wait for next key from terminal
 *
 */
int key_get() {
    return getkey(WAIT);
}

/** key_poll return next key from terminal if there is one
 *
 */
int key_poll() {
    return getkey(NO_WAIT);
}


/* helper function to set 'nodelay' mode according to 'wait'
 * parameter and then ask for the next character
 * leaves 'nodelay' afterwards always as FALSE (meaning: wait for
 * character
 * in both ('wait' and 'no-wait') mode terminal resizes are handled,
 * but no KEY_RESIZE is returned.
 */
static int getkey(bool wait) {

    nodelay(stdscr, wait ? FALSE : TRUE);

    bool done = !wait;
    int x;
    do {
	x = onechar();

	if (x == KEY_RESIZE) {
	    resize_layout();
	    x = 0;  // do not propagate KEY_RESIZE to caller
	} else {
	    done = true;
	}

    } while (!done);

    nodelay(stdscr, FALSE);

    return x;
}

/* Map terminal-specific keys to our internal code
 * as Ncurses does not provide predefined values for them
 */
static int map_terminal_key(int key) {
    // Alt-<Page-Down>
    if (key_kNXT3 && key == key_kNXT3) {
	return TERM_KEY_ALT_PGDN;
    }
    // Alt-<Page-Up>
    if (key_kPRV3 && key == key_kPRV3) {
	return TERM_KEY_ALT_PGUP;
    }
    // Ctrl-<Page-Down>
    if (key_kNXT5 && key == key_kNXT5) {
	return TERM_KEY_CTRL_PGDN;
    }
    // Ctrl-<Page-Up>
    if (key_kPRV5 && key == key_kPRV5) {
	return TERM_KEY_CTRL_PGUP;
    }

    return key;
}

/* New onechar() that takes advantage of Ncurses keypad mode and processes
 * certain escaped keys and assigns them to Ncurses values known by
 * keyname().  Also catches Escape and processes it immediately as well
 * as calling stoptx() for minimal delay.
 */
static int onechar(void) {
    int x = 0;
    int trash = 0;

    x = getch();

    /* Replace Ctl-H (Backspace) and Delete with KEY_BACKSPACE */
    if (x == CTRL_H || x == DELETE)
	x = KEY_BACKSPACE;

    if (x == ESCAPE) {
	nodelay(stdscr, TRUE);

	x = getch();

	/* Escape pressed, not an escaped key. */
	if (x == ERR) {
	    stoptx();

	    return ESCAPE;

	} else if (x != 91) {

	    switch (x) {

		case 32 ... 57:   // Alt-Space to Alt-9,   160 - 185
		case 97 ... 122:  // Alt-a to alt-z,       225 - 250
		    x += 128;
		    break;

		/* Not all terminals support Ctl-Shift-ch so
		 * treat them as Alt-ch
		 */
		case 65 ... 78:   //   alt-A to alt-N,     225 - 238
		case 80 ... 90:   //   alt-P to alt-Z,     240 - 250
		    x += 160;
		    break;

		case 79: {
		    x = getch();

		    /* Catch Alt-O */
		    if (x == ERR) {
			x = 239;
			break;
		    }

		    /* Key codes for Shift-F1 to Shift-F4 in Xfce terminal. */
		    if (x == 49) {
			x = getch();

			if (x == 59) {
			    x = getch();
			    if (x == 50) {
				x = getch();

				switch (x) {

				    case 80: {
					x = KEY_F(13);
					break;
				    }

				    case 81: {
					x = KEY_F(14);
					break;
				    }

				    case 82: {
					x = KEY_F(15);
					break;
				    }

				    case 83: {
					x = KEY_F(16);
					break;
				    }
				}
			    }
			}
		    }
		}
	    }

	    nodelay(stdscr, FALSE);

	} else {
	    nodelay(stdscr, FALSE);

	    x = getch();        /* Get next code after 91 */

	    switch (x) {

		/* Key codes for this section:
		 * 27 91 49 126 Home
		 * 27 91 52 126 End
		 */
		case 49: {
		    x = getch();

		    if (x == 126) {
			x = KEY_HOME;
			break;
		    }
		}

		case 52: {
		    x = KEY_END;
		    trash = getch();
		    break;
		}
	    }
	}
    }

    /* It seems Xterm treats Alt-Space through Alt-9 with a prefix
     * character of 194 followed by 160 through 185.
     */
    if (x == 194) {
	nodelay(stdscr, TRUE);

	trash = getch();

	if (trash == ERR)
	    return x;

	x = trash;

	// Alt-Space to Alt-9
	if (x >= 160 && x <= 185) {
	    nodelay(stdscr, FALSE);

	    return x;
	}
    }

    /* It seems Xterm treats Alt-a to Alt-z with a prefix
     * character of 195 followed by 161-186 (a-z) or
     * 129-154 (A-Z).
     */
    if (x == 195) {
	nodelay(stdscr, TRUE);

	trash = getch();

	if (trash == ERR)
	    return x;

	x = trash;

	switch (x) {

	    case 161 ... 186:  // Alt-a to Alt-z  225 - 250
		x += 64;
		break;

	    case 129 ... 154:  // Alt-A to Alt-Z  225 - 250
		x += 96;
		break;
	}

	nodelay(stdscr, FALSE);
    }

    x = map_terminal_key(x);

    return x;
}


/* Handles resizing of windows
 * As ncurses handles most of it itself, it just redraws the main
 * display and resizes only the packet panel windows
 */
void resize_layout(void) {
    getmaxyx(stdscr, ymax, xmax);
    if (!tlf_initialized) {
	return;     // only update terminal size
    }
    clear_display();
    clusterinfo();
    refresh_splitlayout();
}

#define MAX_SPACES  1000

static char *spaces_buffer = NULL;

const char *spaces(int n) {

    if (spaces_buffer == NULL) {
	spaces_buffer = g_strnfill(MAX_SPACES, ' ');
    }

    if (n < 0) {
	n = 0;
    } else if (n > MAX_SPACES) {
	n = MAX_SPACES;
    }

    return spaces_buffer + (MAX_SPACES - n);
}
