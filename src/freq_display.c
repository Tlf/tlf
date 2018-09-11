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


#include "freq_display.h"
#include "nicebox.h"		// Includes curses.h
#include "tlf.h"
#include "ui_utils.h"


int freq_display(void) {

    extern float freq;
    extern int trxmode;

    int x_position = 40;
    int y_position = 17;
    char fbuffer[8];

    print_space(y_position, x_position);
    print_space(y_position + 1, x_position);
    print_space(y_position + 2, x_position);
    print_space(y_position + 3, x_position);
    print_space(y_position + 4, x_position);
    nicebox(16, 39, 5, 35, "TRX");
    print_dot(y_position + 4, 28 + x_position + 1);

    sprintf(fbuffer, "%7.1f", freq);

    if (fbuffer[0] != ' ')
	print_big_number(fbuffer[0] - 48, y_position, x_position, 4);
    if (fbuffer[1] != ' ')
	print_big_number(fbuffer[1] - 48, y_position, x_position, 9);
    if (fbuffer[2] != ' ')
	print_big_number(fbuffer[2] - 48, y_position, x_position, 14);
    if (fbuffer[3] != ' ')
	print_big_number(fbuffer[3] - 48, y_position, x_position, 19);
    if (fbuffer[4] != ' ')
	print_big_number(fbuffer[4] - 48, y_position, x_position, 24);
    if (fbuffer[6] != ' ')
	print_big_number(fbuffer[6] - 48, y_position, x_position, 31);

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

    if (trxmode == CWMODE)
	mvprintw(18, 41, "CW");
    else if (trxmode == SSBMODE)
	mvprintw(19, 41, "SSB");
    else
	mvprintw(19, 41, "DIG");

    refreshp();

    return (0);
}

int print_big_number(int number, int y_position, int x_position,
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
    refreshp();

    return (0);
}

int print_dot(int y, int x) {

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
    mvprintw(y, x, " ");

    return (0);
}

int print_space(int y, int x) {

    attroff(A_STANDOUT);
    attron(modify_attr(COLOR_PAIR(C_LOG)));

    mvprintw(y, x, "                                   ");

    return (0);
}
