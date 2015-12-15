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


#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "clear_display.h"
#include "deleteqso.h"
#include "printcall.h"
#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "qsonr_to_str.h"
#include "tlf_curses.h"
#include "scroll_log.h"
#include "ui_utils.h"

#define QTCRECVCALLPOS 30
#define QTCSENTCALLPOS 35


void delete_last_qtcs(char *call, char *bandmode) {
    int rc, look, qtclen, s;
    int qtcfile;
    struct stat qstatbuf;
    char logline[100];

    // clean up received qtcs with same call and mode
    if (qtcdirection & RECV) {
	if ((qtcfile = open(QTC_RECV_LOG, O_RDWR)) < 0) {
	    mvprintw(5, 0, "Error opening QTC received logfile.\n");
	    sleep(1);
	}
	fstat(qtcfile, &qstatbuf);
	if ((int)qstatbuf.st_size > QTCRECVCALLPOS) {
	    look = 1;
	    qtclen = 0;
	    // iterate till the current line from back of logfile
	    // callsigns is the current callsign
	    // this works only for fixed length qtc line!
	    lseek(qtcfile, 0, SEEK_SET);
	    while (look == 1) {
		lseek(qtcfile, ((int)qstatbuf.st_size - (91+qtclen)), SEEK_SET);
		rc = read(qtcfile, logline, 90);
		logline[90] = '\0';

		if (!(strncmp(call, logline+QTCRECVCALLPOS, strlen(call)) == 0
		   && strncmp(bandmode, logline, 5) == 0)) {
		    // stop searching
		    look = 0;
		}
		else {
		    qtclen += 91;
		    qtc_dec(call, RECV);
		}
	    }
	    rc = ftruncate(qtcfile, qstatbuf.st_size - qtclen);
	    fsync(qtcfile);
	}
	close(qtcfile);
    }

    // clean up sent qtcs with same call and mode
    if (qtcdirection & SEND) {
	if ((qtcfile = open(QTC_SENT_LOG, O_RDWR)) < 0) {
	    mvprintw(5, 0, "Error opening QTC sent logfile.\n");
	    sleep(1);
	}
	fstat(qtcfile, &qstatbuf);
	if ((int)qstatbuf.st_size > QTCSENTCALLPOS) {
	    look = 1;
	    qtclen = 0;
	    s = nr_qsos;
	    while(s >= 0 && qsoflags_for_qtc[s] != 1) {
		s--;
	    }
	    // iterate till the current line from back of logfile
	    // callsigns is the current callsign
	    // this works only for fixed length qtc line!
	    while (s >= 0 && look == 1) {
		lseek(qtcfile, ((int)qstatbuf.st_size - (96+qtclen)), SEEK_SET);
		rc = read(qtcfile, logline, 95);
		if (!(strncmp(call, logline+QTCSENTCALLPOS, strlen(call)) == 0
		   && strncmp(bandmode, logline, 5) == 0)) {
		    // stop searching
		    look = 0;
		}
		else {
		    qtclen += 96;
		    qtc_dec(call, SEND);

		    qsoflags_for_qtc[s] = 0;
		    next_qtc_qso = s;
		    while(s >= 0 && qsoflags_for_qtc[s] != 1) {
			s--;
		    }
		}
	    }
	    rc = ftruncate(qtcfile, qstatbuf.st_size - qtclen);
	    nr_qtcsent--;
	    fsync(qtcfile);
	}
	close(qtcfile);
    }
}


void delete_qso(void)
{
    int x, rc;
    struct stat statbuf;
    int lfile;
    char logline[100];
    char call[15], bandmode[6];

    mvprintw(13, 29, "OK to delete last qso (y/n)?");
    x = key_get();

    if ((x == 'y') || (x == 'Y')) {

	if ((lfile = open(logfile, O_RDWR)) < 0) {

	    mvprintw(24, 0, "I can not find the logfile...");
	    refreshp();
	    sleep(2);
	} else {

	    fstat(lfile, &statbuf);

	    if (statbuf.st_size >= LOGLINELEN) {
	        if (qtcdirection > 0) {
		    // read band, mode and call from last QSO line
		    lseek(lfile, ((int)statbuf.st_size - LOGLINELEN), SEEK_SET);
		    rc = read(lfile, logline, LOGLINELEN-1);

		    g_strlcpy(bandmode, logline, 6);
		    g_strlcpy(call, logline+29, 15);
		    g_strchomp(call);

		    // delete QTC's for that combination of band, mode and call
		    delete_last_qtcs(call, bandmode);
                }
		rc = ftruncate(lfile, statbuf.st_size - LOGLINELEN);
	    }

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
