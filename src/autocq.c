/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
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
 *        AUTO_CQ
 *
 *--------------------------------------------------------------*/


#include <unistd.h>
#include <ctype.h>

#include "audio.h"
#include "clear_display.h"
#include "cw_utils.h"
#include "globalvars.h"
#include "keyer.h"
#include "keystroke_names.h"
#include "printcall.h"
#include "sendbuf.h"
#include "stoptx.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "time_update.h"


//
// get estimated CQ length in milliseconds
// returns 0 if can't be determined
//
static int get_autocq_time() {
    if (trxmode != CWMODE) {
	return 0;   // unknown
    }
    const int cw_message_len = cw_message_length(message[AUTO_CQ_MSG]);
    return (int)(1200.0 / speed) * cw_message_len;
}

#define NO_KEY (-1)

// non-keypress events:
#define EVENT_CALLSIGN_GRAB     (NO_KEY - 1)

static int handle_immediate_key(int key) {
    int cury, curx;
    switch (key) {
	// keyer speed change
	case KEY_PPAGE: // <Page-Up>
	case KEY_NPAGE: // <Page-Down>

	// auto CQ delay change
	case TERM_KEY_CTRL_PGUP:
	case TERM_KEY_ALT_PGUP:
	case TERM_KEY_CTRL_PGDN:
	case TERM_KEY_ALT_PGDN:

	    getyx(stdscr, cury, curx);  // save cursor
	    key = handle_common_key(key);
	    if (key == 0) {         // key has been processed
		move(cury, curx);   // restore cursor
		key = NO_KEY;       // pretend there was no key press at all
	    }

            break;

	default:
	    // no action
    }
    return key;
}

static int wait_50ms_for_key() {

    usleep(50 * 1000);

    const int inchar = key_poll();
    if (inchar > 0 && inchar != KEY_RESIZE) {
	return handle_immediate_key(inchar);
    }

    return NO_KEY;
}

#define TIME_UPDATE_MS  500


/* wait till VK message is finished or key pressed.
 * calling time_update() each 500 ms.
 */
int vk_wait_finish() {
    int key = NO_KEY;
    int update_timer = TIME_UPDATE_MS;

    while (key == NO_KEY) {
	key = wait_50ms_for_key();

	if (is_vk_finished())
	    return NO_KEY;

	update_timer -= 50;

	if (update_timer <= 0) {
	    time_update();
	    update_timer = TIME_UPDATE_MS;
	}
    }

    return key;
}


// Wait for given ms, polling each 50 ms for a key and calling time_update()
// each 500 ms. Pressing a key terminates the waiting loop.
// Note: this works best if ms >= 500 (or ms = 0)
static int wait_ms(int ms) {
    int key = NO_KEY;
    int update_timer = TIME_UPDATE_MS;
    int wait_timer = ms;

    while (wait_timer > 0 && key == NO_KEY) {

	key = wait_50ms_for_key();

	// check if callsign grab happened
	if (key == NO_KEY && current_qso.call[0] != 0) {
	    key = EVENT_CALLSIGN_GRAB;
	}

	wait_timer -= 50;
	update_timer -= 50;

	if (update_timer <= 0) {
	    time_update();
	    update_timer = TIME_UPDATE_MS;
	}
    }

    return key;
}


int auto_cq(void) {

    const cqmode_t cqmode_save = cqmode;
    cqmode = AUTO_CQ;
    show_header_line();

    const long message_time = get_autocq_time();

    int key = NO_KEY;

    // any unhandled key press terminates auto CQ loop
    while (key == NO_KEY) {

	send_standard_message(AUTO_CQ_MSG);

	move(12, 29);

	// wait till message ends (calculated for CW, playtime for SSB)
	// a key pressed or an event happened
	if (trxmode == CWMODE || trxmode == DIGIMODE) {
	    key = wait_ms(message_time);
	} else {
	    key = vk_wait_finish();
	}

	// wait between calls
	for (int delayval = cqdelay; delayval > 0 && key == NO_KEY; delayval--) {

	    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));
	    mvprintw(12, 29, "Auto CQ  %-2d ", delayval);
	    refreshp();

	    key = wait_ms(500);

	    if (delayval > cqdelay) {   // in case it was shortened while waiting
		delayval = cqdelay;
	    }
	}

	mvaddstr(12, 29, spaces(13));
	move(12, 29);
	refreshp();
    }

    stoptx();

    cqmode = cqmode_save;
    show_header_line();

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvaddstr(12, 29, spaces(13));
    printcall();

    if (key < NO_KEY) {     // map events to NO_KEY
	key = NO_KEY;
    }

    return toupper(key);
}


