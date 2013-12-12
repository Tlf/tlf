/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012           Thomas Beierlein <tb@forth-ev.de>
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
	 *       delete  last qso
	 *
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "deleteqso.h"

void delete_qso(void)
{

    int x, rc;
    int lfile;
    struct stat statbuf;

    mvprintw(13, 29, "OK to delete last qso (y/n)?");
    x = onechar();

    if ((x == 'y') || (x == 'Y')) {

	if ((lfile = open(logfile, O_RDWR)) < 0) {

	    mvprintw(24, 0, "I can not find the logfile...");
	    refreshp();
	    sleep(2);
	} else {

	    fstat(lfile, &statbuf);

	    if (statbuf.st_size >= LOGLINELEN)
		rc = ftruncate(lfile, statbuf.st_size - LOGLINELEN);

	    fsync(lfile);
	    close(lfile);

	    if (qsos[nr_qsos][0] != ';') {
		band_score[bandinx]--;
		qsonum--;
		qsonr_to_str();
	    }

	    nr_qsos--;
	    qsos[nr_qsos][0] = '\0';
	}

	scroll_log();

    }

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    mvprintw(13, 29, "                            ");

    printcall();

    clear_display();
}
