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

#include "callinput.h"
#include "clear_display.h"
#include "cw_utils.h"
#include "globalvars.h"
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
    const int cw_message_len = cw_message_length(message[11]);
    return (int)(1200.0 / GetCWSpeed()) * cw_message_len;
}


int auto_cq(void) {

#define NO_KEY -1

    int key = NO_KEY;

    const cqmode_t cqmode_save = cqmode;
    cqmode = AUTO_CQ;
    show_header_line();

    const long message_time = get_autocq_time();

    while (key == NO_KEY) {

	send_standard_message(11);

	mvprintw(12, 29, "");

	attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

	// if length of message is known then wait until its end
	// key press terminates auto CQ loop
	if (message_time > 0) {
	    for (int j = 0; j < 10 && key == NO_KEY; j++) {
		usleep(message_time * 100);
		time_update();
		const int inchar = key_poll();
		if (inchar > 0 && inchar != KEY_RESIZE) {
		    key = inchar;
		}
	    }
	}

	// wait between calls
	for (int delayval = cqdelay; delayval > 0 && key == NO_KEY; delayval--) {

	    mvprintw(12, 29, "Auto CQ  %-2d ", delayval - 1);
	    refreshp();

	    usleep(500000); // 500 ms
	    time_update();

	    const int inchar = key_poll();
	    if (inchar > 0 && inchar != KEY_RESIZE) {
		key = inchar;
	    }
	}

	mvprintw(12, 29, "%s", spaces(13));
	mvprintw(12, 29, "");
	refreshp();
    }

    stoptx();

    cqmode = cqmode_save;
    show_header_line();

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvprintw(12, 29, "%s", spaces(13));
    printcall();

    return toupper(key);
}


