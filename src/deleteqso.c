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
#include "qtcutil.h"

#define QTCRECVCALLPOS 30
#define QTCSENTCALLPOS 35

extern int qtcdirection;
extern int nr_qtcsent;

void delete_qso(void)
{

    int x, rc, i, look, qtclen, s;
    int lfile, qtcfile;
    struct stat statbuf;
    struct stat qstatbuf;
    char logline[100];
    char call[15], bandmode[6];

    mvprintw(13, 29, "OK to delete last qso (y/n)?");
    x = onechar();

    if ((x == 'y') || (x == 'Y')) {

	if ((lfile = open(logfile, O_RDWR)) < 0) {

	    mvprintw(24, 0, "I can not find the logfile...");
	    refreshp();
	    sleep(2);
	} else {

	    fstat(lfile, &statbuf);

	    if (statbuf.st_size >= LOGLINELEN) {
	        if (qtcdirection > 0) {
		    // seek to last line to read it
		    lseek(lfile, ((int)statbuf.st_size - LOGLINELEN), SEEK_SET);
		    rc = read(lfile, logline, LOGLINELEN-1);
		    // catch the band and mode (for QTC)
		    strncpy(bandmode, logline, 5);
		    bandmode[5] = '\0';
		    call[14] = '\0';
		    // catch the last callsign
		    strncpy(call, logline+29, 14);
		    i = strlen(call);
		    // strip it
		    for(i=strlen(call)-1; call[i] == ' '; i--);
		    call[i+1] = '\0';
		    // if qtc had been set up
		    if (qtcdirection & 1) {
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

				if (! (strncmp(call, logline+QTCRECVCALLPOS, strlen(call)) == 0 && strncmp(bandmode, logline, 5) == 0)) {
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
                    if (qtcdirection & 2) {
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
                                if (! (strncmp(call, logline+QTCSENTCALLPOS, strlen(call)) == 0 && strncmp(bandmode, logline, 5) == 0)) {
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
