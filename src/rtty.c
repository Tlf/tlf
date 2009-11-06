/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@eudxf.org>
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
 	*      rtty mini terminal
 	*
 	*--------------------------------------------------------------*/

#include "rtty.h"
#include <termios.h>
#include <curses.h>
#include <string.h>
#include <stdio.h>
#include "startmsg.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "tlf.h"
#include "printcall.h"

static int fdcont;		// global for this file: tty file descriptor
static char ry_term[4][50] = {"", "", "", "" };

/* ------------------------------------- initialize  controller --------------------------------- */
int init_controller() {
 
extern char controllerport[];

struct termios termattribs;

    		if ((fdcont = open(controllerport, O_RDWR | O_NONBLOCK)) < 0) {
                		showstring(controllerport,  ": Open  failed for controller port!!!\n");
				sleep(1);
                return(-1);
    		}

	termattribs.c_iflag = IGNBRK | IGNPAR | IMAXBEL | IXOFF;
    	termattribs.c_oflag = 0;
    	termattribs.c_cflag = CS8 | CSTOPB | CREAD | CLOCAL;
    	termattribs.c_lflag = 0;		    /* Set some term flags */

    	/*  The ensure there are no read timeouts (possibly writes?) */
    	termattribs.c_cc[VMIN] = 1;
    	termattribs.c_cc[VTIME] = 0;

	cfsetispeed(&termattribs,B9600);	    /* Set input speed */
    	cfsetospeed(&termattribs,B9600);	    /* Set output speed */

	tcsetattr(fdcont,TCSANOW,&termattribs);  /* Set the serial port */

	showstring(controllerport, " opened...\n");

return(fdcont);		// return file descriptor
}

/* -------------------------------------  add text to terminal ------------------------------------------- */

int ry_addtext(char *line) {

//extern char ry_term[][];	### bug fix

int k, m, j;
char *ptr;
char bufferline[80];
FILE *ry_fp;

ptr = line;
k = strlen(line);

if  ( (ry_fp = fopen("RTTYlog","a"))  == NULL){
	mvprintw(24,0, "cannot open RTTYlog");
	refresh();
	return(-1);
} else {
	fputs(line, ry_fp);

	fclose(ry_fp);

}

if (k > 40)
	return(0);

strcpy(bufferline, line);

	for (j = 0; j < k; j++) {
		if (bufferline[j] == 10 || bufferline[j] == 13|| bufferline[j]==7) {
			bufferline[j] = 32;
			if (j < 40) {
				strcat (bufferline, "                                      ");
				bufferline[39] = '\0';
				j = 0;
				k = 0;
			}
		}
	}


m = strlen(ry_term[4]);

if (k <= (40 - strlen(ry_term[4]))) {
	strcat (ry_term[4], bufferline);
	ry_term[4][40] = '\0';
	line[0]='\0';
	bufferline[0]='\0';
}else
{
	strncat(ry_term[4], bufferline, 39 - m);
	ry_term[4][40] = '\0';
//	bufferline += (40-m);
	strcpy(bufferline, bufferline + (40-m));
	strncpy(ry_term[0], ry_term[1], 40);
	strncpy(ry_term[1], ry_term[2], 40);
	strncpy(ry_term[2], ry_term[3], 40);
	strncpy(ry_term[3], ry_term[4], 40);

	ry_term[4][0] = '\0';
	strcat(ry_term[4], bufferline);
	line[0] ='\0';
	bufferline[0]='\0';
}


return(0);
}

/* -------------------------------------  display rtty ------------------------------------------- */

int show_rtty(void) {

extern int use_rxvt;
//extern char ry_term[][];		### bug fix
extern int trxmode;
//extern char hiscall[];
extern int miniterm;
extern int commentfield;
extern char comment[];

if (trxmode != DIGIMODE || miniterm == 0)
	return(-1);

	attroff(A_STANDOUT);
	if (use_rxvt == 0) attron(COLOR_PAIR(COLOR_GREEN) | A_BOLD );
	else  attron(COLOR_PAIR(COLOR_GREEN) );

		mvprintw(1,0, "                                         ");
		mvprintw(1,0, ry_term[0]);
		mvprintw(2,0, "                                         ");
		mvprintw(2,0, ry_term[1]);
		mvprintw(3,0, "                                         ");
		mvprintw(3,0, ry_term[2]);
		mvprintw(4,0, "                                         ");
		mvprintw(4,0, ry_term[3]);
		mvprintw(5,0, "                                         ");
		mvprintw(5,0, ry_term[4]);
		if (commentfield == 0) {
			printcall();
		} else {
			mvprintw(12,54, comment);
		}
		refresh();
		attron(A_STANDOUT);

		return(0);
}

/* -------------------------------------  receive rtty ------------------------------------------- */

int rx_rtty () {

extern char hiscall[];
extern int miniterm;

//extern char ry_term[][];		### bug fix

int i = 0;
char line[40];
static int miniterm_status = 0;


if (fdcont > 0)
{

	if (miniterm_status == 0 && miniterm == 1) {
		miniterm_status = 1;
		ry_term[0][0] = '\0';
		ry_term[1][0] = '\0';
		ry_term[2][0] = '\0';
		ry_term[3][0] = '\0';
		ry_term[4][0] = '\0';
	}

	i = read (fdcont, line, 39);
//RX (2006-03-31 14:41Z): 		remove
	if ( i > 0) {
		line[i] = '\0';
		
		if (i > 23) 
			strcpy(line, line + 24);

		ry_addtext(line);

		}

}

if (strlen(hiscall) > 0)
	show_rtty();

return (0);
}
