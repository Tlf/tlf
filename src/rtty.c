/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@eudxf.org>
 *               2012                Thomas Beierlein <tb@forth-ev.de>
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
 *      rtty mini terminal
 *
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "err_utils.h"
#include "globalvars.h"
#include "printcall.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "startmsg.h"
#include "tlf_curses.h"
#include "ui_utils.h"

#include "fldigixmlrpc.h"

static int fdcont;		// global for this file: tty file descriptor
static char ry_term[5][50] = { "", "", "", "", "" };
static int pos = 0;

/* ----------------------- initialize  controller ------------------------ */
int init_controller() {

    struct termios termattribs;

    if ((fdcont = open(controllerport, O_RDWR | O_NONBLOCK)) < 0) {
	showstring(controllerport,
		   ": Open failed for controller port!!!\n");
	sleep(1);
	return -1;
    }

    termattribs.c_iflag = IGNBRK | IGNPAR | IMAXBEL | IXOFF;
    termattribs.c_oflag = 0;
    termattribs.c_cflag = CS8 | CSTOPB | CREAD | CLOCAL;
    termattribs.c_lflag = 0;	/* Set some term flags */

    /*  The ensure there are no read timeouts (possibly writes?) */
    termattribs.c_cc[VMIN] = 1;
    termattribs.c_cc[VTIME] = 0;

    cfsetispeed(&termattribs, B9600);	/* Set input speed */
    cfsetospeed(&termattribs, B9600);	/* Set output speed */

    tcsetattr(fdcont, TCSANOW, &termattribs);	/* Set the serial port */

    if (digikeyer == GMFSK) {
	lseek(fdcont, 0, SEEK_END);
    }

    showstring(controllerport, " opened...\n");

    return fdcont;		// return file descriptor
}

/* ------------------------- deinit controller -------------------------- */
void deinit_controller() {
    if (fdcont) {
	close(fdcont);
	fdcont = 0;
    }
}

/* ---------------------- start new line ---------------------------------*/
static void scroll_up() {
    int i;
    for (i = 0; i < 4; ++i) {
	g_strlcpy(ry_term[i], ry_term[i + 1], 41);
    }
    ry_term[4][0] = '\0';
    pos = 0;

    if (qtc_ry_capture == 1) {
	if (qtc_ry_currline == (QTC_RY_LINE_NR - 1)
		&& qtc_ry_lines[qtc_ry_currline].content[0] != '\0') {
	    for (i = 0; i < (QTC_RY_LINE_NR - 1); i++) {
		g_strlcpy(qtc_ry_lines[i].content,
			  qtc_ry_lines[i + 1].content, 41);
		qtc_ry_lines[i].attr = qtc_ry_lines[i + 1].attr;
	    }
	} else {
	    if (strlen(qtc_ry_lines[qtc_ry_currline].content) > 0) {
		qtc_ry_currline++;
	    }
	}
	qtc_ry_lines[qtc_ry_currline].content[0] = '\0';
	qtc_ry_lines[qtc_ry_currline].attr = 0;
    }
}

/* ------------------------  add text to terminal ------------------------ */
void ry_addchar(char c) {

    FILE *ry_fp;

    // write to log file
    if ((ry_fp = fopen("RTTYlog", "a")) == NULL) {
	TLF_LOG_INFO("cannot open RTTYlog");
	return;
    }
    fputc(c, ry_fp);
    fclose(ry_fp);

    if ((c & 0x80) != 0)
	return;			/* drop non-ascii characters */

    if (c == '\n' || c == '\r') {   /* NL or CR */
	scroll_up();
	return;
    }

    if (iscntrl(c)) { /* replace all other control characters by space */
	c = ' ';
    }

    if (pos >= 40) {
	scroll_up();
    }

    // add char to line
    if (qtc_ry_capture == 1) {
	qtc_ry_lines[qtc_ry_currline].content[pos] = c;
	qtc_ry_lines[qtc_ry_currline].content[pos + 1] = '\0';
    }
    ry_term[4][pos] = c;
    ry_term[4][pos + 1] = '\0';
    ++pos;
}


/* ----------------------  display rtty ---------------------------------- */

void show_rtty(void) {

    if (!miniterm) {
	return;
    }

    attroff(A_STANDOUT);
    attron(modify_attr(COLOR_PAIR(C_HEADER)));

    for (int i = 0; i < 5; ++i) {
	mvaddstr(i + 1, 0, spaces(40));
	mvaddstr(i + 1, 0, ry_term[i]);
    }
    if (commentfield == 0) {
	printcall();
    } else {
	mvaddstr(12, 54, current_qso.comment);
    }
    refreshp();
    attron(A_STANDOUT);

}

/* ---------------------  receive rtty ----------------------------------- */

void rx_rtty() {

    int i;
    int j;
    char line[40];
    char c;
    static bool initialized = false;	/* for one time initialization */
    static int state = 0;		/* 0 - line start found
					   1 - ')' found
					   2 - ':' found
					   3 - additional space passed
					 */

    if (!initialized) {
	for (i = 0; i < 5; ++i) {
	    ry_term[i][0] = '\0';
	}
	pos = 0;
	initialized = true;
    }


    if (fdcont > 0) {

	i = read(fdcont, line, 39);

	if (i == 0)
	    return;

	if (digikeyer == GMFSK) {
	    /* skip begin of line until '):' if keyer == GMFSK */
	    /* RX (2006-03-31 14:41Z): */
	    for (j = 0; j < i; j++) {
		c = line[j];

		switch (state) {
		    case 0:
			if (c == ')')
			    state++;
			break;
		    case 1:
			if (c == ':')
			    state++;
			else
			    state = 0;
			break;
		    case 2:
			if (c == '\n')
			    state = 0;
			else
			    state++;
			break;
		    case 3:
			if (c == '\n')
			    state = 0;

			ry_addchar(c);
			break;
		    default:
			break;
		}
	    }
	} else {
	    /* serial modem */
	    for (j = 0; j < i; j++) {
		ry_addchar(line[j]);
	    }
	}
    } else if (digikeyer == FLDIGI) {
	i = fldigi_get_rx_text(line, sizeof(line));
	for (j = 0; j < i; j++) {
	    ry_addchar(line[j]);
	}
    }

}
