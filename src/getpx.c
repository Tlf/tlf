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

	/* ------------------------------------------------------------
	 *     get the prefix
	 *
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "getpx.h"

/** \brief Get prefix from call 
 *
 * Analyses callsign and extract prefix information like follows:
 *    - Remember a portable prefix area if present .../8
 *    - Copy first character (letter or number)
 *    - Copy all following letters
 *    - Copy one more character (number or /)
 *    - If next char is a digit, copy it too
 *        - else if portable prefix remembered replace last character (number)
 *    - If last copied character was '/' replace it by '0'
 *        - else if last character was a letter append '0'
 *
 * \param checkcall Call to analyse
 */
 
/** \todo fix problem: k3a/2 wird nicht als K2 erkannt,
 *  kl32a/4 wird zu kl34 ???, check andere Implementierungen */

void getpx(char *checkcall)
{
    char pxbuffer[16] = "";
    int i, len;
    char j = 0;

    len = strlen(checkcall);

    if (len >= 2) {

	if ((checkcall[len - 2] == '/') && isdigit(checkcall[len - 1]))	/*  portable /3 */
	    j = checkcall[len - 1];
    }

    for (i = 0; i < len; i++) {
	if (((checkcall[i] <= 'Z') && (checkcall[i] >= 'A')) || (i == 0))
	    pxbuffer[i] = checkcall[i];
	else
	    break;
    }
    pxbuffer[i] = checkcall[i];

    i++;

    if (isdigit(checkcall[i])) {
	pxbuffer[i] = checkcall[i];
	pxbuffer[i + 1] = '\0';
    } else {
	if (j != 0) {
	    pxbuffer[i - 1] = j;
	}
	pxbuffer[i] = '\0';
    }

    strcpy(pxstr, pxbuffer);

    if (pxstr[strlen(pxstr) - 1] == '/') {
	pxstr[strlen(pxstr) - 1] = '0';
    } else if ((pxstr[strlen(pxstr) - 1] <= 'Z')
	       && (pxstr[strlen(pxstr) - 1] >= 'A'))
	strcat(pxstr, "0");

}
