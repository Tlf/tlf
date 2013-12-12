/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 * 		 2011, 2013 Thomas Beierlein <tb@forth-ev.de>
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
	 *
	 *              Read country data  from disk file cty.dat
	 *--------------------------------------------------------------*/

#include "dxcc.h"
#include "readctydata.h"
#include <glib.h>
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

int readctydata(void)
{
    char buf[181] = "";
    char ctydb_location[80];
    char *loc;

    FILE *fp_db;

    strcpy(ctydb_location, "cty.dat");
    if ((fp_db = fopen(ctydb_location, "r")) == NULL) {
	strcpy(ctydb_location, PACKAGE_DATA_DIR);
	strcat(ctydb_location, "/cty.dat");

	if ((fp_db = fopen(ctydb_location, "r")) == NULL) {
	    mvprintw(4, 0, "Error opening cty.dat file.\n");
	    refreshp();
	    sleep(5);
	    endwin();
	    exit(1);
	}
    }

    dxcc_init();
    prefix_init();

    // set default for empty country
    dxcc_add("Not Specified        :    --:  --:  --:  -00.00:    00.00:     0.0:     :");

    while (fgets(buf, sizeof(buf), fp_db) != NULL) {

	g_strchomp(buf); 	/* drop CR and/or NL and */
	if (*buf == '\0')	/* ignore empty lines */
	    continue;

	if (buf[0] != ' ') {	// data line

	    dxcc_add(buf);

	} else			// prefix line
	{
	    loc = strtok(buf, " ,;");
	    while (loc != NULL) {

		prefix_add (loc);

		loc = strtok(NULL, " ,;");
	    }
	}
    }

    fclose(fp_db);

    return (0);
}
