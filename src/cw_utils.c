/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013 Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "globalvars.h"
#include <stdlib.h>
#include <string.h>

#include <glib.h>

int speed = 20;

/** get length of CW characters
 *
 * converts a given CW character into the number of dot elements
 * \param ch the character to convert
 * \return number of dots for the character including the following character
 *         space
 */
unsigned int getCWdots(char ch) {

    unsigned int length;

    switch (ch) {
	case 'A':
	    length = 9;
	    break;
	case 'B':
	    length = 13;
	    break;
	case 'C':
	    length = 15;
	    break;
	case 'D':
	    length = 11;
	    break;
	case 'E':
	    length = 5;
	    break;
	case 'F':
	    length = 13;
	    break;
	case 'G':
	    length = 13;
	    break;
	case 'H':
	    length = 11;
	    break;
	case 'I':
	    length = 7;
	    break;
	case 'J':
	    length = 17;
	    break;
	case 'K':
	    length = 13;
	    break;
	case 'L':
	    length = 13;
	    break;
	case 'M':
	    length = 11;
	    break;
	case 'N':
	    length = 9;
	    break;
	case 'O':
	    length = 15;
	    break;
	case 'P':
	    length = 15;
	    break;
	case 'Q':
	    length = 17;
	    break;
	case 'R':
	    length = 11;
	    break;
	case 'S':
	    length = 9;
	    break;
	case 'T':
	    length = 7;
	    break;
	case 'U':
	    length = 11;
	    break;
	case 'V':
	    length = 13;
	    break;
	case 'W':
	    length = 13;
	    break;
	case 'X':
	    length = 15;
	    break;
	case 'Y':
	    length = 17;
	    break;
	case 'Z':
	    length = 15;
	    break;
	case '0':
	    length = 23;
	    break;
	case '1':
	    length = 21;
	    break;
	case '2':
	    length = 19;
	    break;
	case '3':
	    length = 17;
	    break;
	case '4':
	    length = 15;
	    break;
	case '5':
	    length = 13;
	    break;
	case '6':
	    length = 15;
	    break;
	case '7':
	    length = 17;
	    break;
	case '8':
	    length = 19;
	    break;
	case '9':
	    length = 21;
	    break;
	case '/':
	    length = 17;
	    break;
	case '?':
	    length = 19;
	    break;
	case ' ':
	    length = 3;
	    break;
	default:
	    length = 0;
    }
    return (length);
}


/** calculate dot length of a cw message
 *
 * Calculate the length of a given CW message in dot elements.
 * Expands '%' into your own call.
 * \param message the CW message
 * \return number of dot elements in the message
 */
unsigned int cw_message_length(char *message) {

    int i;
    int message_length = 0;
    char cwmessage[80];
    int testchar, j;

    strncpy(cwmessage, message, 79);
    cwmessage[79] = '\0';

    for (i = 0; i < strlen(cwmessage); i++) {

	testchar = cwmessage[i];
	if (testchar == '%') {
	    for (j = 0; j < strlen(my.call); j++) {
		testchar = my.call[j];
		message_length += getCWdots(testchar);
	    }

	} else
	    message_length += getCWdots(testchar);

    }
    return (message_length);
}

