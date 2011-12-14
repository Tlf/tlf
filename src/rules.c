/***************************************************************************
                          rules.c  -  description
                             -------------------
    begin                : Tue Sep 10 2003
    copyright            : (C) 2003 by lz3ny
    email                : lz3ny@bfra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rules.h"
#include "tlf.h"
#include "startmsg.h"

extern char whichcontest[];
extern char logfile[];

/* This function overrides the parse_logcfg() details */
/* 10.09.2003 - LZ3NY */

int read_rules()
{
    char mit_contest_rule[80];
    char contest_conf[80] = "";	// contest_conf needs room for contest name... (PA0R)
    char basic_contest_conf[75] = PACKAGE_DATA_DIR;
    FILE *mit_contest_file;

/* If no contest is given, whichcontest is set to default "qso"   (PA0R, Sep 24 2003)*/
    if (strlen(whichcontest) == 0) {
	showmsg("contest name is empty! Assuming general qso mode!! ");
//              mvprintw(5,0,"\n contest name is empty!\n Assuming general qso mode!! ");
	strcpy(whichcontest, "qso");
	return (1);
    }

    if (strlen(whichcontest) >= 40) {
	showmsg("contest name is too long!");
	showmsg("exiting...");
//      mvprintw(5,0,"\n contest name is too long!\n exiting...\n ");
	exit(1);
    }

    memset(mit_contest_rule, '\0', strlen(whichcontest) + 6);
    strcat(contest_conf, "rules/");
    strcat(contest_conf, whichcontest);
/* If rules are not found in local working directory, look in /usr/local/share... (PA0R, Sep 24 2003)*/
    strcat(basic_contest_conf, "/rules/");
    strcat(basic_contest_conf, whichcontest);

    if ((mit_contest_file = fopen(contest_conf, "r")) != NULL) {

	showstring("reading contest rules file:", contest_conf);
//      mvprintw(6,0,"reading contest rules file: %s ... \n",contest_conf);
//      refreshp();

	while ( fgets(mit_contest_rule, sizeof(mit_contest_rule),
		       mit_contest_file) != NULL ) {

	    /* if not comment interpret line */
	    if ((mit_contest_rule[0] != '#') && (mit_contest_rule[0] != ';')) {
		parse_logcfg(mit_contest_rule);
	    }
	}
	fclose(mit_contest_file);
	showstring("Using contest rules file: ", contest_conf);
//       mvprintw(7,0,"\nUsing contest rules file: %s\n",contest_conf);
//       refreshp();
    } else if ((mit_contest_file = fopen(basic_contest_conf, "r")) != NULL) {

	showstring("reading contest rules file:", basic_contest_conf);
//      mvprintw(6,0,"reading contest rules file: %s ... \n",basic_contest_conf);
//      refreshp();

	while ( fgets(mit_contest_rule, sizeof(mit_contest_rule),
		       mit_contest_file) != NULL ) {

	    /* if not comment interpret line */
	    if ((mit_contest_rule[0] != '#') && (mit_contest_rule[0] != ';')) {
		parse_logcfg(mit_contest_rule);
	    }
	}
	fclose(mit_contest_file);
	showstring("Using contest rules file:", basic_contest_conf);
//       mvprintw(7,0,"\nUsing contest rules file: %s\n",basic_contest_conf);
//       refreshp();
    }

    else {
	showstring("There is no contest rules file", contest_conf);
	showmsg("Assuming regular QSO operation. Logfile is qso.log");
//      mvprintw(7,0,"\nThere is no contest rules file %s!\nAssuming regular QSO operation.\nLogfile is qso.log\n",contest_conf);
	strcpy(whichcontest, "qso");	// default use general qso mode... (PA0R, 24 Sept. 2003)
	setcontest();
	strcpy(logfile, "qso.log");
	refreshp();
    }
    return (0);
}
