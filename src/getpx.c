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

/* ------------------------------------------------------------
 *     get the prefix
 *
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <string.h>

#include "globalvars.h"		// Includes glib.h and tlf.h


/** \brief Get prefix from call
 *
 * Analyses callsign and extract prefix information like follows:
 *    - If callsign contains only letters, copy first two and append '0'
 *    - Remember a portable prefix area if present .../3
 *    - Copy first character (letter or number)
 *    - Copy all following letters
 *    - Copy all following numbers
 *    - If portable prefix remembered and last character is digit
 *	replace last character
 *    - If last character was a letter append '0'
 *
 * \param checkcall Call to analyse
 *	    (!has to be in normalized format:
 *	     - call
 *	     - portable prefix/call
 *	     - call/portable area)
 */

/** \todo fix problem: kl32a/4 wird zu kl34 ??? */

int letters_only(const char *call) {
    int i;

    for (i = 0; i < strlen(call); i++) {
	if (!isalpha(call[i]))
	    return 0;
    }
    return 1;
}

void getpx(char *checkcall) {
    char pxbuffer[16] = "";
    int i, len;
    char portable = '\0';

    if (letters_only(checkcall)) {
	/* only characters in call */
	strncpy(pxbuffer, checkcall, 2);
	strcat(pxbuffer, "0");
    } else {
	len = strlen(checkcall);
	if (len >= 2) {
	    if ((checkcall[len - 2] == '/') && isdigit(checkcall[len - 1]))
		/*  portable /3 */
		portable = checkcall[len - 1];
	}

	for (i = 0; i < len; i++) {
	    if (((checkcall[i] <= 'Z') && (checkcall[i] >= 'A')) || (i == 0))
		pxbuffer[i] = checkcall[i];
	    else
		break;
	}
	for (; i < len; i++) {
	    if (((checkcall[i] <= '9') && (checkcall[i] >= '0')) || (i == 0))
		pxbuffer[i] = checkcall[i];
	    else
		break;
	}

	if (portable != '\0' && isdigit(pxbuffer[i - 1]))
	    pxbuffer[i - 1] = portable;

	if (isalpha(pxbuffer[i - 1]))
	    pxbuffer[i] = '0';
    }
    strcpy(pxstr, pxbuffer);
}
