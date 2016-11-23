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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include "printcall.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "startmsg.h"
#include "tlf_curses.h"
#include "ui_utils.h"

#include "fldigixmlrpc.h"

static int fdcont;		// global for this file: tty file descriptor
static char ry_term[5][50] = { "", "", "", "", "" };

/* ----------------------- initialize  controller ------------------------ */
int init_controller()
{
    extern char controllerport[];
    extern int keyerport;

    struct termios termattribs;

    if ((fdcont = open(controllerport, O_RDWR | O_NONBLOCK)) < 0) {
	showstring(controllerport,
		   ": Open  failed for controller port!!!\n");
	sleep(1);
	return (-1);
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

    if (keyerport == GMFSK) {
	lseek(fdcont, 0, SEEK_END);
    }

    showstring(controllerport, " opened...\n");

    return (fdcont);		// return file descriptor
}

/* ------------------------- deinit controller -------------------------- */
void deinit_controller()
{
    if (fdcont)
	close(fdcont);
}

/* ------------------------  add text to terminal ------------------------ */
void ry_addchar(char c)
{
    static int k = 0;
    int i = 0;
    FILE *ry_fp;

    if ((ry_fp = fopen("RTTYlog", "a")) == NULL) {
	mvprintw(24, 0, "cannot open RTTYlog");
	refreshp();
	return;
    } else {
	fputc(c, ry_fp);
	fclose(ry_fp);
    }

    if ((c & 0x80) != 0)
	return;			/* drop on ascii characters */

    if ((c == '\n') || (c == '\r')) {
	/* start new line */
	g_strlcpy(ry_term[0], ry_term[1], 41);
	g_strlcpy(ry_term[1], ry_term[2], 41);
	g_strlcpy(ry_term[2], ry_term[3], 41);
	g_strlcpy(ry_term[3], ry_term[4], 41);
	ry_term[4][0] = '\0';
	k = 0;

	if (qtc_ry_capture == 1) {
	    if (qtc_ry_currline == (QTC_RY_LINE_NR - 1)
	     && qtc_ry_lines[qtc_ry_currline].content[0] != '\0') {
		for(i=0; i<(QTC_RY_LINE_NR - 1); i++) {
		    g_strlcpy(qtc_ry_lines[i].content,
			      qtc_ry_lines[i+1].content, 41);
		    qtc_ry_lines[i].attr = qtc_ry_lines[i+1].attr;
		}
	    }
	    else {
		if (strlen(qtc_ry_lines[qtc_ry_currline].content) > 0) {
		    qtc_ry_currline++;
		}
	    }
	    qtc_ry_lines[qtc_ry_currline].content[0] = '\0';
	    qtc_ry_lines[qtc_ry_currline].attr = 0;
	}
    }
    else {
	if (iscntrl( c )) {
	    /* replace all other control characters by space */
	    c = ' ';
	}

	if (k >= 40) {
	    // scroll line
	    g_strlcpy(ry_term[0], ry_term[1], 41);
	    g_strlcpy(ry_term[1], ry_term[2], 41);
	    g_strlcpy(ry_term[2], ry_term[3], 41);
	    g_strlcpy(ry_term[3], ry_term[4], 41);
	    ry_term[4][0] = '\0';
	    k = 0;

	    if (qtc_ry_capture == 1) {
		if (qtc_ry_currline == (QTC_RY_LINE_NR - 1)
		 && qtc_ry_lines[qtc_ry_currline].content[0] != '\0') {
		    for(i=0; i<(QTC_RY_LINE_NR - 1); i++) {
			g_strlcpy(qtc_ry_lines[i].content,
				  qtc_ry_lines[i+1].content, 41);
			qtc_ry_lines[i].attr = qtc_ry_lines[i+1].attr;
		    }
		}
		else {
		    qtc_ry_currline++;
		}
		qtc_ry_lines[qtc_ry_currline].content[0] = '\0';
		qtc_ry_lines[qtc_ry_currline].attr = 0;
	    }
	}

	// add char to line
	if (qtc_ry_capture == 1) {
	    qtc_ry_lines[qtc_ry_currline].content[k] = c;
	    qtc_ry_lines[qtc_ry_currline].content[k+1] = '\0';
	}
	ry_term[4][k++] = c;
	ry_term[4][k] = '\0';
    }
}


/* ----------------------  display rtty ---------------------------------- */

int show_rtty(void)
{
    extern int trxmode;
    extern int miniterm;
    extern int commentfield;
    extern char comment[];

    if (trxmode != DIGIMODE || miniterm == 0)
	return (-1);

    attroff(A_STANDOUT);
    attron(modify_attr(COLOR_PAIR(C_HEADER)));

    mvprintw(1, 0, "                                        ");
    mvprintw(1, 0, "%s", ry_term[0]);
    mvprintw(2, 0, "                                        ");
    mvprintw(2, 0, "%s", ry_term[1]);
    mvprintw(3, 0, "                                        ");
    mvprintw(3, 0, "%s", ry_term[2]);
    mvprintw(4, 0, "                                        ");
    mvprintw(4, 0, "%s", ry_term[3]);
    mvprintw(5, 0, "                                        ");
    mvprintw(5, 0, "%s", ry_term[4]);
    if (commentfield == 0) {
	printcall();
    } else {
	mvprintw(12, 54, comment);
    }
    refreshp();
    attron(A_STANDOUT);

    return (0);
}

/* ---------------------  receive rtty ----------------------------------- */

int rx_rtty()
{
    extern int miniterm;
    extern int keyerport;

    int i;
    int j;
    char line[40];
    char c;
    static int miniterm_status = 0;	/* for one time initialization */
    static int state = 0;		/* 0 - line start found
					   1 - ')' found
					   2 - ':' found
					   3 - additional space passed
					 */

    if (fdcont > 0) {

	if (miniterm_status == 0 && miniterm == 1) {
	    miniterm_status = 1;
	    ry_term[0][0] = '\0';
	    ry_term[1][0] = '\0';
	    ry_term[2][0] = '\0';
	    ry_term[3][0] = '\0';
	    ry_term[4][0] = '\0';
	}

	i = read(fdcont, line, 39);

	if (i == 0)
	    return 0;

	if (keyerport == GMFSK) {
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

			ry_addchar( c );
			break;
		    default:
			break;
		}
	    }
	}
	else {
	    /* serial modem */
	    for (j = 0; j < i; j++) {
		ry_addchar( line[j] );
	    }
	}
    }
    else if (keyerport == FLDIGI) {
	i = fldigi_get_rx_text(line);
	for (j = 0; j < i; j++) {
	    ry_addchar( line[j] );
	}
    }

    return (0);
}
