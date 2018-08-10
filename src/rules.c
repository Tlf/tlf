/*
* Tlf - contest logging program for amateur radio operators
* Copyright (C) 2003	  LZ3NY <lz3ny@bfra.org>
* 		2003-2004 Rein Couperus <pa0rct@amsat.org>
* 		2011-2015           Thomas Beierlein <tb@forth-ev.de>
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    char mit_contest_rule[80];
    char contest_conf[80] = "";	/* contest_conf needs room
				   for contest name... (PA0R) */
    char basic_contest_conf[75] = PACKAGE_DATA_DIR;
    FILE *mit_contest_file;

    int status = PARSE_OK;

    /* If no contest is given, whichcontest is set to default "qso"
       (PA0R, Sep 24 2003)*/
    if (strlen(whichcontest) == 0) {
	showmsg("contest name is empty! Assuming general qso mode!! ");
	strcpy(whichcontest, "qso");
	return (PARSE_ERROR);
    }

    if (strlen(whichcontest) >= 40) {
	showmsg("contest name is too long!");
	showmsg("exiting...");
	exit(1);
    }

    memset(mit_contest_rule, '\0', strlen(whichcontest) + 6);
    strcat(contest_conf, "rules/");
    strcat(contest_conf, whichcontest);
    /* If rules are not found in local working directory,
       look in /usr/local/share... (PA0R, Sep 24 2003)*/
    strcat(basic_contest_conf, "/rules/");
    strcat(basic_contest_conf, whichcontest);

    if ((mit_contest_file = fopen(contest_conf, "r")) != NULL) {

	showstring("reading contest rules file:", contest_conf);

	while (fgets(mit_contest_rule, sizeof(mit_contest_rule),
		     mit_contest_file) != NULL) {

	    /* if not comment interpret line */
	    if ((mit_contest_rule[0] != '#') && (mit_contest_rule[0] != ';')) {
		status |= parse_logcfg(mit_contest_rule);
	    }
	}
	fclose(mit_contest_file);
	showstring("Using contest rules file: ", contest_conf);
    } else if ((mit_contest_file = fopen(basic_contest_conf, "r")) != NULL) {

	showstring("reading contest rules file:", basic_contest_conf);

	while (fgets(mit_contest_rule, sizeof(mit_contest_rule),
		     mit_contest_file) != NULL) {

	    /* if not comment interpret line */
	    if ((mit_contest_rule[0] != '#') && (mit_contest_rule[0] != ';')) {
		status |= parse_logcfg(mit_contest_rule);
	    }
	}
	fclose(mit_contest_file);
	showstring("Using contest rules file:", basic_contest_conf);
    }

    else {
	showstring("There is no contest rules file", contest_conf);
	showmsg("Assuming regular QSO operation. Logfile is qso.log");
	strcpy(whichcontest, "qso");	/* default use general qso mode...
					   (PA0R, 24 Sept. 2003) */
	setcontest();
	strcpy(logfile, "qso.log");
	refreshp();
    }

    /*
     * Now, for unspecified digi messages, copy from the CW message,
     * putting CRLF at the start, and changing the trailing \n to a
     * space
     */
    for (int i = 0; i < 25; i++) {
	if (digi_message[i] == NULL) {
	    asprintf(&digi_message[i], "|%s", message[i]);

	    if (digi_message[i] == NULL) {
		showmsg("unable to create digi message!");
		status = PARSE_ERROR;
	    }
	    else {
		char *c = strrchr(digi_message[i], '\n');
		if (c)
		    *c = ' ';
	    }
	}
    }

    return (status);
}
