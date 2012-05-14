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
	 *    make sure logfile is present
	 *
	 *--------------------------------------------------------------*/

#include "checklogfile.h"

void checklogfile(void)
{

    extern char logfile[26];
    extern char backgrnd_str[];

    int lfile;
    int qsobytes;
    int qsolines;
    int errbytes;
    int rc;
    struct stat statbuf;
    char inputbuffer[800];
    char *rp;

    FILE *infile;
    FILE *outfile;
    FILE *fp;

    if ((fp = fopen(logfile, "a")) == NULL) {
	fprintf(stdout, "Opening logfile not possible.\n");
	exit(1);
    }

    fclose(fp);

    if ((lfile = open(logfile, O_RDWR)) < 0) {

	mvprintw(24, 0, "I can not find the logfile...");
	refreshp();
	sleep(2);
	exit(0);

    } else {

	fstat(lfile, &statbuf);
	qsobytes = statbuf.st_size;
	qsolines = qsobytes / LOGLINELEN;
	errbytes = qsobytes % LOGLINELEN;

	if (errbytes != 0) {

	    close(lfile);

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

			if (strlen(inputbuffer) != LOGLINELEN) {
			    strcat(inputbuffer, backgrnd_str);
			    inputbuffer[LOGLINELEN] = '\0';
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
			rc = ftruncate(lfile, statbuf.st_size - LOGLINELEN);
			fsync(lfile);

		    }

		    close(lfile);
		}

		rename("./cpyfile", logfile);
		remove("./cpyfile");
	    }

	} else
	    close(lfile);
    }

}
