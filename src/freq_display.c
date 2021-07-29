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

#include <hamlib/rig.h>

#include "freq_display.h"
#include "globalvars.h"
#include "nicebox.h"		// Includes curses.h
#include "tlf.h"
#include "ui_utils.h"

static void print_dot(int y, int x);
static void clear_freq_display(int y, int x);
static void print_big_number(int number, int y_position, int x_position,
			     int location);

void freq_display(void) {

    const int x_position = 40;
    const int y_position = 17;

    clear_freq_display(y_position, x_position);
    nicebox(16, 39, 5, 35, "TRX");
    print_dot(y_position + 4, 28 + x_position + 1);

    char fbuffer[8];
    sprintf(fbuffer, "%7.1f", freq / 1000.0);

    // display the digits
    int x_offset = 4;
    for (int i = 0; i <= 6; ++i) {
	if (i == 5) {   // skip decimal dot
	    x_offset += 2;
	    continue;
	}
	const int digit = fbuffer[i] - '0';
	if (digit >= 0) {
	    print_big_number(digit, y_position, x_position, x_offset);
	}
	x_offset += 5;
    }

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

    if (trxmode == CWMODE)
	mvprintw(18, 41, "CW");
    else if (trxmode == SSBMODE)
	mvprintw(19, 41, "SSB");
    else
	mvprintw(19, 41, "DIG");

    refreshp();
}


void print_big_number(int number, int y_position, int x_position,
		      int location) {

    switch (number) {

	case 1: {

	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 2: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position);
	    print_dot(y_position + 4, location + x_position);
	    print_dot(y_position + 4, location + x_position + 1);
	    print_dot(y_position + 4, location + x_position + 2);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 3: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position);
	    print_dot(y_position + 4, location + x_position + 1);
	    print_dot(y_position + 4, location + x_position + 2);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}
	case 4: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 5: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position);
	    print_dot(y_position + 4, location + x_position + 1);
	    print_dot(y_position + 4, location + x_position + 2);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 6: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position + 1, location + x_position);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position);
	    print_dot(y_position + 4, location + x_position + 1);
	    print_dot(y_position + 4, location + x_position + 2);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 7: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 8: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position);
	    print_dot(y_position + 4, location + x_position + 1);
	    print_dot(y_position + 4, location + x_position + 2);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 9: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 1);
	    print_dot(y_position + 2, location + x_position + 2);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

	case 0: {

	    print_dot(y_position, location + x_position);
	    print_dot(y_position, location + x_position + 1);
	    print_dot(y_position, location + x_position + 2);
	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position);
	    print_dot(y_position + 4, location + x_position + 1);
	    print_dot(y_position + 4, location + x_position + 2);
	    print_dot(y_position + 4, location + x_position + 3);
	    break;
	}

    }

}

void print_dot(int y, int x) {

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvprintw(y, x, " ");

}

void clear_freq_display(int y, int x) {

    attroff(A_STANDOUT);
    attron(modify_attr(COLOR_PAIR(C_LOG)));

    for (int i = 0; i < 5; ++i) {
	mvprintw(y + i, x, spaces(35));
    }

}
