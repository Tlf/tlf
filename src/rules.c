/*
* Tlf - contest logging program for amateur radio operators
* Copyright (C) 2003	  LZ3NY <lz3ny@bfra.org>
* 		2003-2004 Rein Couperus <pa0rct@amsat.org>
* 		2011-2019 Thomas Beierlein <tb@forth-ev.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "globalvars.h"
#include "parse_logcfg.h"
#include "setcontest.h"
#include "showmsg.h"
#include "tlf.h"
#include "utils.h"


/* This function overrides the parse_logcfg() details */
/* 10.09.2003 - LZ3NY */

int read_rules() {

    char *contest_conf;
    char *rules_file;
    FILE *fp;

    int status = PARSE_OK;

    /* If no contest is given, whichcontest is set to default "qso"
       (PA0R, Sep 24 2003)*/
    if (strlen(whichcontest) == 0) {
	showmsg("Contest name is empty! Assuming general qso mode!! ");
	strcpy(whichcontest, "qso");
	return (PARSE_ERROR);
    }

    if (strlen(whichcontest) >= 40) {
	showmsg("Contest name is too long!");
	showmsg("Exiting...");
	exit(1);
    }

    contest_conf = g_strconcat("rules", G_DIR_SEPARATOR_S, whichcontest, NULL);

    rules_file = find_available(contest_conf);


    if ((fp = fopen(rules_file, "r")) != NULL) {

	showstring("Reading contest rules file:", rules_file);

	status = parse_configfile(fp);
	fclose(fp);

    } else {

	showstring("There is no contest rules file:", contest_conf);
	showmsg("Assuming regular QSO operation. Logfile is qso.log");
	setcontest(QSO_MODE);		 /* default use general qso mode...
					   (PA0R, 24 Sept. 2003) */
	strcpy(logfile, "qso.log");
    }
    g_free(rules_file);
    g_free(contest_conf);

    /*
     * Now, for unspecified digi messages, copy from the CW message,
     * putting CRLF at the start, and changing the trailing \n to a
     * space
     */
    int i;
    for (i = 0; i < 25; i++) {
	if (digi_message[i] == NULL) {
	    if (asprintf(&digi_message[i], "|%s", message[i]) == -1) {
		showmsg("Out of memory: unable to create digi message!");
		sleep(2);
		exit(EXIT_FAILURE);
	    } else {
		char *c = strrchr(digi_message[i], '\n');
		if (c)
		    *c = ' ';
	    }
	}
    }

    return status;
}
