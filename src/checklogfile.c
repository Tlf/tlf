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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* ------------------------------------------------------------
 *    make sure logfile is present and can be opened for append
 *    - create one if it does not exist
 *    - check that length of logfile is a integer m^ultiple of
 *      the loglinelen
 *
 *--------------------------------------------------------------*/


#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include "err_utils.h"
#include "ignore_unused.h"
#include "showmsg.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"


/** Repair log file
 *
 * Try to repair log file if some lines are too short.
 * Same is used to convert old logfile format v1 to new v2
 *
 * \return 0 on success
 */
int repair_log(char *filename) {
    gchar *backupfile;
    gchar *cmd;
    char *buffer = NULL;
    size_t buffer_len;
    gchar *fill;
    int rc;
    int read;
    FILE *infp;
    FILE *outfp;

    /* make a backup of the original file */
    backupfile = g_strconcat(filename, ".bak", NULL);
    showstring("Backing up original file as: ", backupfile);

    cmd = g_strconcat("cp ", filename, " ", backupfile, NULL);
    rc = system(cmd);
    g_free(cmd);

    if (rc != 0) {
	showmsg("Could not backup logfile. Giving up!");
	return 1;
    }


    showmsg("Converting file to new format");
    if ((infp = fopen(backupfile, "r")) == NULL) {
	showmsg("Could not convert logfile. Sorry!");
	return 1;
    }

    if ((outfp = fopen(filename, "w")) == NULL) {
	fclose(infp);
	showmsg("Could not convert logfile. Sorry!");
	return 1;
    }

    errno = 0;
    while ((read = getline(&buffer, &buffer_len, infp)) != -1) {
	if (read > 0) {
	    /* strip trailing whitespace (and newline) */
	    g_strchomp(buffer);

	    /* append spaces */
	    fill = g_strnfill((LOGLINELEN - 1) - strlen(buffer), ' ');
	    strcat(buffer, fill);
	    g_free(fill);

	    fputs(buffer, outfp);
	    fputs("\n", outfp);
	    errno = 0;	    /* fputs leaves errno undefined */
	}
    }
    if (errno == ENOMEM) {
	fprintf(stderr, "Error in: %s:%d", __FILE__, __LINE__);
	perror("RuntimeError: ");
	exit(EXIT_FAILURE);
    }

    free(buffer);
    fclose(outfp);
    fclose(infp);
    g_free(backupfile);

    showmsg("Done");
    sleep(2);

    return 0;
}

int checklogfile_new(char *filename) {
    int lineno;
    int tooshort;
    int read;
    char *buffer = NULL;
    size_t buffer_len;
    FILE *fp;

    /* check if logfile exist and can be opened for read */
    if ((fp = fopen(filename, "r")) == NULL) {

	if (errno == EACCES) {
	    showstring("Can not access log file: ", filename);
	    return 1;
	}

	if (errno == ENOENT) {
	    /* File not found, create new one */
	    showmsg("Log file not found, creating new one");
	    sleep(2);
	    if ((fp = fopen(filename, "w")) == NULL) {
		/* cannot create logfile */
		showmsg("Creating logfile not possible");
		return 1;
	    }
	    /* New logfile created */
	    fclose(fp);
	    return 0;
	}

	showstring("Can not check log file: ", filename);
	return 1;
    }


    /* check each line of the logfile of correct format */
    lineno = 0;
    tooshort = 0;

    errno = 0;
    while ((read = getline(&buffer, &buffer_len, fp)) != -1) {
	if (read > 0) {
	    int band, linelen;
	    int bandok = 0;

	    lineno++;

	    /* if no logline -> complain and back */
	    band = atoi(buffer);

	    if ((band == 160) ||
		    (band == 80) ||
		    (band == 60) ||
		    (band == 40) ||
		    (band == 30) ||
		    (band == 20) ||
		    (band == 17) ||
		    (band == 15) ||
		    (band == 12) ||
		    (band == 10))
		bandok = 1;

	    if (!((buffer[0] == ';') || bandok)) {
		/* msg no valid logline in line #, cannot handle it */
		shownr("No valid log line in line ", lineno);
		free(buffer);
		fclose(fp);
		return 1;
	    }

	    linelen = strlen(buffer);

	    /* if to long -> complain and back */
	    if (linelen > LOGLINELEN) {
		/* msg length of line # to long,
		* cannot handle that log file format */
		shownr("Log line to long in line ", lineno);
		showmsg("Can not handle that log format");
		free(buffer);
		fclose(fp);
		return 1;
	    }

	    /* if to short -> remember */
	    if (linelen < LOGLINELEN) {
		tooshort = 1;
	    }
	}
    }
    if (errno == ENOMEM) {
	fprintf(stderr, "Error in: %s:%d", __FILE__, __LINE__);
	perror("RuntimeError: ");
	exit(EXIT_FAILURE);
    }

    free(buffer);
    fclose(fp);

    if (tooshort) {
	char c;

	/* some lines in logfile are too short, maybe old logfile format */
	showmsg("Some log lines are too short (maybe an old log format)!");
	showmsg("Shall I try to repair? Y/(N) ");
	echo();
	c = toupper(key_get());
	noecho();

	if (c != 'Y') {
	    return 1;			/* giving up */
	}

	/* trying to repair */
	return repair_log(filename);
    }

    return 0;
}


void checklogfile(void) {
    extern char logfile[];

    int qsobytes;
    int read;
    struct stat statbuf;
    char *inputbuffer = NULL;
    size_t read_len;

    FILE *infile;
    FILE *outfile;
    FILE *fp;

    if ((fp = fopen(logfile, "a")) == NULL) {
	TLF_LOG_WARN("I can not find the logfile ...");
    } else {
	fstat(fileno(fp), &statbuf);
	fclose(fp);

	qsobytes = statbuf.st_size;

	if ((qsobytes % LOGLINELEN) != 0) {
	    if ((infile = fopen(logfile, "r")) == NULL) {
		TLF_LOG_WARN("Unable to open logfile...");
	    } else {
		if ((outfile = fopen("./cpyfile", "w")) == NULL) {
		    fclose(infile);
		    TLF_LOG_WARN("Unable to open cpyfile...");

		} else {
		    errno = 0;
		    while ((read = getline(&inputbuffer, &read_len, infile)) != -1) {
			if (read > 0) {
			    if (strlen(inputbuffer) != LOGLINELEN) {
				/* append spaces */
				for (int i = strlen(inputbuffer);
					i < LOGLINELEN; i++) {

				    strcat(inputbuffer, " ");
				}

				inputbuffer[LOGLINELEN - 1] = '\n';
				inputbuffer[LOGLINELEN] = '\0';
			    }
			    fputs(inputbuffer, outfile);
			    errno = 0;	    /* fputs leaves errno undefined */
			}
		    }
		    if (errno == ENOMEM) {
			fprintf(stderr, "Error in: %s:%d", __FILE__, __LINE__);
			perror("RuntimeError: ");
			exit(EXIT_FAILURE);
		    }
		    free(inputbuffer);
		    fclose(infile);
		    fclose(outfile);
		}
		rename("./cpyfile", logfile);
		remove("./cpyfile");
	    }
	}
    }
}
