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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE	// For asprintf()
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "globalvars.h"
#include "parse_logcfg.h"
#include "setcontest.h"
#include "startmsg.h"
#include "tlf.h"


extern char whichcontest[];
extern char logfile[];

/* This function overrides the parse_logcfg() details */
/* 10.09.2003 - LZ3NY */

int read_rules() {

    char contest_conf[80] = "";	/* contest_conf needs room
				   for contest name... (PA0R) */
    char basic_contest_conf[75] = PACKAGE_DATA_DIR;
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

    strcat(contest_conf, "rules/");
    strcat(contest_conf, whichcontest);
    /* If rules are not found in local working directory,
       look in /usr/local/share... (PA0R, Sep 24 2003)*/
    strcat(basic_contest_conf, "/rules/");
    strcat(basic_contest_conf, whichcontest);

    if ((fp = fopen(contest_conf, "r")) != NULL) {

	showstring("Reading contest rules file:", contest_conf);

	status = parse_configfile(fp);
	fclose(fp);
    } else if ((fp = fopen(basic_contest_conf, "r")) != NULL) {

	showstring("Reading contest rules file:", basic_contest_conf);

	status = parse_configfile(fp);
	fclose(fp);
    } else {
	showstring("There is no contest rules file:", contest_conf);
	showmsg("Assuming regular QSO operation. Logfile is qso.log");
	setcontest(QSO_MODE);		 /* default use general qso mode...
					   (PA0R, 24 Sept. 2003) */
	strcpy(logfile, "qso.log");
    }

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
