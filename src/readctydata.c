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
	 *
	 *              Read country data  from disk
	 *--------------------------------------------------------------*/

#include "readctydata.h"
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

int readctydata(void)
{
    extern char prefixlines[MAX_DBLINES][17];
    extern char zonearray[MAX_DBLINES][3];
    extern char ituarray[MAX_DBLINES][3];
    extern int dataindex[MAX_DBLINES];
    extern char datalines[MAX_DATALINES][81];

//      char buf[81] = "";      ### bug fix
    char buf[181] = "";
    char buffer[80];
    char ctydb_location[80];
    int n = 0, i = 0, j = 0, k = 0, o = 0, m = 0;
    static int nrofpfx = 0;
    char *cqloc;
    char *ituloc;

    FILE *fp_db;

    strcpy(ctydb_location, "cty.dat");
    if ((fp_db = fopen(ctydb_location, "r")) == NULL) {
	ctydb_location[0] = '\0';
	strcpy(ctydb_location, PACKAGE_DATA_DIR);
	strcat(ctydb_location, "/cty.dat");

	if ((fp_db = fopen(ctydb_location, "r")) == NULL) {
	    mvprintw(4, 0, "Error opening cty.dat  file.\n");
	    refresh();
	    sleep(5);
	    endwin();
	    exit(1);
	}
    }
    // set default for empty country

    strcpy(datalines[0],
	   "Not Specified        :    --:  --:  --:  -00.00:    00.00:     0.0:     :\r\n");

/* read  ctydb.dat file ---------------------------------------------------- */

    o = 1;			// data lines
    k = 0;			// prefix lines
    i = 0;			// pointer in prefix line
    j = 0;			// pointer in prefix line

    while (!feof(fp_db)) {
	if (fgets(buf, sizeof(buf), fp_db) == NULL) {
	    break;
	}

	if (buf[0] == '\n')
	    continue;

	if (buf[0] != ' ') {	// data line
	    strncpy(datalines[o], buf, strlen(buf) - 1);
	    o++;
	} else			// prefix line
	{
	    strncpy(buffer, buf, 79);
	    buffer[79] = 0;

	    char *loc = NULL;	//PG4I, 26Jul2005
	    if ((loc = strchr(buffer, '\r')))
		*loc = '\0';

//                      buffer[strlen(buffer)-1] = '\0';            // remove     \012
//                      buffer[strlen(buffer)-1] = '\0';           // remove      \015

	    n = strlen(buffer);

	    for (i = 0; i < n; i++) {
		if (buffer[i] == ',' || buffer[i] == ';')
		    buffer[i] = '\0';
	    }

/*			if (j < 4)
				continue;
*/
	    j = 4;

	    while (strcmp(buffer + j, "") != 0) {
		nrofpfx++;

		strcpy(prefixlines[k], buffer + j);

		m = strlen(buffer + j);

		ituloc = strchr(prefixlines[k], '[');	// locate the itu zone
		if (ituloc != NULL) {
		    if (atoi(ituloc + 1) > 9)
			sprintf(ituarray[k], "%d", atoi(ituloc + 1));
		    else
			sprintf(ituarray[k], "0%d", atoi(ituloc + 1));

		    ituloc[0] = '\0';	// struncate the string
		} else
		    ituarray[k][0] = '\0';

		dataindex[k] = o - 1;	// country index

		cqloc = strchr(prefixlines[k], '(');	// locate the cq zone
		if (cqloc != NULL) {
		    if (atoi(cqloc + 1) > 9)
			sprintf(zonearray[k], "%d", atoi(cqloc + 1));
		    else
			sprintf(zonearray[k], "0%d", atoi(cqloc + 1));

		    cqloc[0] = '\0';	// struncate the string
		} else
		    zonearray[k][0] = '\0';

		k++;
		j += m;
		j++;
	    }

	}
    }

    fclose(fp_db);

    return (0);
}
