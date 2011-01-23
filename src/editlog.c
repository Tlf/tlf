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
	 *        Edit Log
	 *
	 *--------------------------------------------------------------*/

#include "editlog.h"

int logedit(void)
{
    extern char logfile[];
    extern char backgrnd_str[];
    extern int editor;
    extern int stop_backgrnd_process;

    char comstr[40] = "";
    int j;
    int lfile;
    int qsobytes;
    int qsolines;
    int errbytes;
    int rc;
    struct stat statbuf;
    FILE *infile;
    FILE *outfile;
    char inputbuffer[800];
    char *rp;

    if (editor == EDITOR_JOE)
	strcat(comstr, "joe  ");	/*   my favorite editor   */
    else if (editor == EDITOR_VI)
	strcat(comstr, "vi  ");
    else
	strcat(comstr, "e3  ");

    strcat(comstr, logfile);
    rc = system(comstr);
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    erase();
    refreshp();
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }

    if ((lfile = open(logfile, O_RDONLY)) < 0) {

	mvprintw(24, 0, "I can not find the logfile...");
	refreshp();
	sleep(2);

    } else {

	fstat(lfile, &statbuf);
	qsobytes = statbuf.st_size;
	qsolines = qsobytes / 81;
	errbytes = qsobytes - (qsolines * 81);

	if (errbytes != 0) {

	    close(lfile);

	    stop_backgrnd_process = 1;

	    if ((infile = fopen(logfile, "r")) == NULL) {
		mvprintw(24, 0, "Unable to open logfile...");
		refreshp();
		sleep(2);

	    } else {
		if ((outfile = fopen("./cpyfile", "w")) == NULL) {
		    mvprintw(24, 0, "Unable to open cpyfile...");
		    refreshp();
		    sleep(2);
		} else {

		    while (!(feof(infile))) {

			rp = fgets(inputbuffer, 160, infile);

			if (strlen(inputbuffer) != 81) {
			    strcat(inputbuffer, backgrnd_str);
			    inputbuffer[81] = '\0';
			}

			fputs(inputbuffer, outfile);
		    }

		    fclose(infile);
		    fclose(outfile);

		}

		if ((lfile = open("./cpyfile", O_RDWR)) < 0) {

		    mvprintw(24, 0, "I can not find the copy file...");
		    refreshp();
		    sleep(2);
		} else {

		    fstat(lfile, &statbuf);

		    if (statbuf.st_size > 80) {
			rc = ftruncate(lfile, statbuf.st_size - 81);
			fsync(lfile);

		    }

		    close(lfile);
		}

		rename("./cpyfile", logfile);
		remove("./cpyfile");
	    }

	} else
	    close(lfile);

	stop_backgrnd_process = 0;
    }

    close(lfile);

    scroll_log();
    refreshp();

    return (0);
}
