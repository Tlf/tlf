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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "freq_display.h"

int freq_display(void)
{

    extern float freq;
    extern int trxmode;

    int x_position = 40;
    int y_position = 17;
    int location = 0;
    char fbuffer[8];

    print_space(y_position, x_position);
    print_space(y_position + 1, x_position);
    print_space(y_position + 2, x_position);
    print_space(y_position + 3, x_position);
    print_space(y_position + 4, x_position);
    nicebox(16, 39, 5, 35, "TRX");
    print_dot(y_position + 4, 28 + x_position + 1);

    if (freq > 7300)
	sprintf(fbuffer, "%5.1f", freq);
    else
	sprintf(fbuffer, " %5.1f", freq);

    location = 32;

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

    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);

    if (trxmode == CWMODE)
	mvprintw(18, 41, "CW");
    else if (trxmode == SSBMODE)
	mvprintw(19, 41, "SSB");
    else
	mvprintw(19, 41, "DIG");

    refresh();

    return (0);
}

int print_big_number(int number, int y_position, int x_position,
		     int location)
{

    switch (number) {

    case 1:{

	    print_dot(y_position, location + x_position + 3);
	    print_dot(y_position + 1, location + x_position + 3);
	    print_dot(y_position + 2, location + x_position + 3);
	    print_dot(y_position + 3, location + x_position + 3);
	    print_dot(y_position + 4, location + x_position + 3);

	    break;
	}

    case 2:{

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

    case 3:{

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
    case 4:{

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

    case 5:{

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

    case 6:{

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

    case 7:{

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

    case 8:{

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

    case 9:{

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

    case 0:{

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
    refresh();

    return (0);
}

int print_dot(int y, int x)
{

    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
    mvprintw(y, x, " ");

    return (0);
}

int print_space(int y, int x)
{

    extern int use_rxvt;

    attroff(A_STANDOUT);

    if (use_rxvt == 0)
	attron(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
    else
	attron(COLOR_PAIR(COLOR_WHITE));

    mvprintw(y, x, "                                   ");

    return (0);
}
