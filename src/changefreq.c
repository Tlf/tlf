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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <unistd.h>

#include "freq_display.h"
#include "time_update.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "gettxinfo.h"


void change_freq(void) {

    extern freq_t freq;
    extern int trx_control;

    int brkflg = 0;

    if (trx_control == 0)
	return;

    curs_set(0);

    while (1) {

	freq_display();

	if (get_outfreq() == 0) {
            // last request has been processed, check keys again
	    int x = key_poll();

	    int deltaf = 0;

	    switch (x) {

		// Up arrow, raise frequency by 100 Hz.
		case KEY_UP: {
		    deltaf = 100;
		    break;
		}

		// Down arrow, lower frequency by 100 Hz.
		case KEY_DOWN: {
		    deltaf = -100;
		    break;
		}

		// Right arrow, raise frequency by 20 Hz.
		case KEY_RIGHT: {
		    deltaf = 20;
		    break;
		}

		// Left arrow, lower frequency by 20 Hz.
		case KEY_LEFT: {
		    deltaf = -20;
		    break;
		}

		// <Page-Up>, raise frequency by 500 Hz.
		case KEY_PPAGE: {
		    deltaf = 500;
		    break;
		}

		// <Page-Down>, lower frequency by 500 Hz.
		case KEY_NPAGE: {
		    deltaf = -500;
		    break;
		}

                // no key
                case ERR: {
		    break;
                }

                // any other key: exit frequency change mode
		default: {
		    brkflg = 1;
		    break;
		}

	    }

	    if (deltaf) {
		set_outfreq(freq + deltaf);
	    }
	}

	if (brkflg) {
	    break;
	}


	time_update();

	freq_display();

	usleep(100 * 1000);

    }
    curs_set(1);
}
