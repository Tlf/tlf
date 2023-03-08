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
*        Store_qso  writes qso to disk
*
*--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "globalvars.h"		// Includes glib.h and tlf.h
#include "tlf_curses.h"


void store_qso(const char *file, char *loglineptr) {
    FILE *fp;

    if ((fp = fopen(file, "a"))  == NULL) {
	fprintf(stdout,  "store_qso.c: Error opening file.\n");
	sleep(1);
	endwin();
	exit(1);
    }

    fputs(loglineptr, fp);
    fputc('\n', fp);

    fclose(fp);
}

