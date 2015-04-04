/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
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
	 *      List and set parameters
	 *
	 *--------------------------------------------------------------*/
#include "setparameters.h"
#include "tlf.h"
#include "setcontest.h"
#include "writeparas.h"
#include "onechar.h"
#include "getmessages.h"
#include "checklogfile.h"
#include "getwwv.h"
#include "scroll_log.h"
#include "readcalls.h"
#include "clear_display.h"
#include "checkparameters.h"


int setparameters(void)
{

    extern int cluster;
    extern int shortqsonr;
    extern int searchflg;
    extern int contest;
    extern int announcefilter;
    extern int showscore_flag;
    extern char call[];
    extern char logfile[];
    extern char whichcontest[];
    extern int stop_backgrnd_process;

    int i = '9';
    char callcpy[12] = "";
    char logbuffer[20];

    stop_backgrnd_process = 1;	/* to prevent race condition */

    while ((i == '7') || (i == '8') || (i == '9')) {

	for (i = 14; i <= 22; i++)
	    mvprintw(i, 2, "                         ");

	if (cluster == NOCLUSTER)
	    mvprintw(14, 2, "1: Cluster = OFF");
	else if (cluster == MAP)
	    mvprintw(14, 2, "1: Cluster = BANDMAP");
	else
	    mvprintw(14, 2, "1: Cluster = Full info");

	if (shortqsonr == SHORTCW)
	    mvprintw(15, 2, "2: SHORT QSONR");
	else
	    mvprintw(15, 2, "2: LONG QSONR");

	if (searchflg == SEARCHWINDOW)
	    mvprintw(16, 2, "3: Duping ON");
	else
	    mvprintw(16, 2, "3: Duping OFF");

	if (contest == CONTEST)
	    mvprintw(17, 2, "4: CONTEST MODE");
	else
	    mvprintw(17, 2, "4: QSO MODE");

	if (announcefilter == FILTER_ANN)
	    mvprintw(18, 2, "5: FILTER ON");
	else
	    mvprintw(18, 2, "5: FILTER OFF");

	if (showscore_flag == 0)
	    mvprintw(19, 2, "6: Score window OFF");
	else
	    mvprintw(19, 2, "6: Score window ON");

	mvprintw(20, 2, "7: Logfile: %s", logfile);

	strncpy(callcpy, call, strlen(call) - 1);
	mvprintw(21, 2, "8: Call:    %s", callcpy);
	mvprintw(22, 2, "9: Contest: %s", whichcontest);

	attroff(A_STANDOUT);
	mvprintw(23, 25, "Change parameter: 7,8,9, none");

	refreshp();

	i = onechar();

	if (i == '7') {

	    mvprintw(20, 14, "                    ");
	    mvprintw(20, 14, "");

	    echo();
	    getnstr(logbuffer, 20);
	    noecho();

	    logfile[0] = '\0';

//                      if (logbuffer[0] != '.')
//                              strcat(logfile, "./");
	    strcat(logfile, logbuffer);
	    logfile[strlen(logfile) - 1] = '\0';

	    writeparas();

	    clear();

	    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
	    getmessages();	/* read .paras file */
	    sleep(2);

	    checklogfile();	/* make sure logfile is there */

	    setcontest();	/* set contest parameters */

	    getwwv();		/* get the latest wwv info from packet */

	    scroll_log();	/* read the last 5  log lines and set the qso number */

	    readcalls();	/* read the logfile for score and dupe */

	    clear_display();	/* tidy up the display */

	    return (0);
	}
	if (i == '8') {

	    mvprintw(21, 14, "                    ");
	    mvprintw(21, 14, "");

	    echo();
	    getnstr(call, 20);
	    noecho();
	    strcat(call, "\n");
	}
	if (i == '9') {

	    mvprintw(1, 2, "cqww      ");
	    mvprintw(2, 2, "wpx       ");
	    mvprintw(3, 2, "arrldx_usa");
	    mvprintw(4, 2, "pacc_pa   ");
	    mvprintw(5, 2, "dxped     ");
	    mvprintw(6, 2, "qso       ");
	    mvprintw(7, 2, "waedc     ");
	    mvprintw(1, 12, "  other     ");
	    mvprintw(2, 12, "            ");
	    mvprintw(3, 12, "  arrldx_dx ");
	    mvprintw(4, 12, "  pacc_dx   ");
	    mvprintw(5, 12, "            ");
	    mvprintw(6, 12, "            ");
	    mvprintw(7, 12, "   waedc    ");

	    mvprintw(22, 14, "                    ");
	    mvprintw(22, 14, "");

	    echo();
	    getnstr(whichcontest, 20);
	    noecho();

	    setcontest();

	}

	writeparas();
	beep();
    }

    stop_backgrnd_process = 0;	/* release backgrnd process */

    return (0);
}
