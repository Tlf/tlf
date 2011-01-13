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

    char buf[181] = "";
    char ctydb_location[80];
    int i = 0, j = 0, k = 0, o = 0;
    static int nrofpfx = 0;
    char *cqloc;
    char *ituloc;
    char *loc;

    FILE *fp_db;

    strcpy(ctydb_location, "cty.dat");
    if ((fp_db = fopen(ctydb_location, "r")) == NULL) {
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

    while (fgets(buf, sizeof(buf), fp_db) != NULL) {

	/* drop CR and/or NL */
	if ((loc = strpbrk(buf, "\r\n")))
	    *loc = '\0';
	/* else {
	 * Fehlermeldung 'string too long */

	if (*buf == '\0')	/* ignore empty lines */
	    continue;

	if (buf[0] != ' ') {	// data line
	    strncpy(datalines[o], buf, sizeof(datalines[0]) - 1);
	    datalines[o][sizeof(datalines[0]) - 1] = '\0';
	    o++;
	} else			// prefix line
	{
	    loc = strtok(buf, " ,;");
	    while (loc != NULL) {
		nrofpfx++;

		strcpy(prefixlines[k], loc);

		ituloc = strchr(prefixlines[k], '[');	// locate the itu zone
		if (ituloc != NULL) {
		    sprintf(ituarray[k], "%02d", atoi(ituloc + 1));
		    *ituloc = '\0';	// truncate the string
		} else
		    ituarray[k][0] = '\0';

		cqloc = strchr(prefixlines[k], '(');	// locate the cq zone
		if (cqloc != NULL) {
		    sprintf(zonearray[k], "%02d", atoi(cqloc + 1));
		    *cqloc = '\0';	// truncate the string
		} else
		    zonearray[k][0] = '\0';

		dataindex[k] = o - 1;	// remember country index in pfx

		k++;
		loc = strtok(NULL, " ,;");
	    }
	}
    }
#ifdef test
    shownr ("Number of Prefixes read =", nrofpfx);
    for (n=0; n<nrofpfx; n++)
	fprintf(stderr,"%s\n",prefixlines[n]);
#endif

    fclose(fp_db);

    return (0);
}
