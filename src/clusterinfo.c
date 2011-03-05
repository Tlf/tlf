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
	 *
	 *          clusterinfo +  time update
	 *--------------------------------------------------------------*/

#include "clusterinfo.h"

void clusterinfo(char *timestr)
{
    extern int use_rxvt;
    extern int cluster;
    extern char backgrnd_str[];
    extern float freq;
    extern float mem;
    extern char band[9][4];
    extern int bandinx;
    extern int trx_control;
    extern int showfreq;
    extern int showscore_flag;
    extern int spotarray[MAX_SPOTS];
    extern char spot_ptr[MAX_SPOTS][82];
    extern int lan_active;
    extern float node_frequencies[MAXNODES];
    extern char thisnode;
    extern int time_master;
    extern int nroflines;
    extern struct tm *time_ptr;

    int f, j, k;
    char inputbuffer[160] = "";
    char time_buf[40] = "";
    static int frcounter;
    static int daysecs = 0;

    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);
    strncpy(time_buf, timestr, 8);
    mvaddstr(12, 0, band[bandinx]);
    mvprintw(12, 17, time_buf);

    if (daysecs > 60) {		// update the date 1x per minute
	daysecs = 0;
	get_time();
	strftime(time_buf, 60, "%d-%b-%y", time_ptr);
	mvprintw(12, 7, time_buf);
    } else {
	daysecs++;
    }

    if (trx_control == 1) {
	if (freq != 0.0) {

	    if (use_rxvt == 0)
		attron(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
	    else
		attron(COLOR_PAIR(COLOR_WHITE));

	    if ((showfreq == 0) || (showscore_flag == 1))
		mvprintw(13, 68, "TRX: %7.1f", freq);

	    if (mem > 0.0)
		mvprintw(14, 68, "MEM: %7.1f", mem);
	    else
		mvprintw(14, 68, "            ");

	    if ((showfreq == 1) && (showscore_flag == 0)) {

		freq_display();
	    }
	}
    }

    frcounter++;

    if (frcounter >= 60) {	// 60 seconds
	frcounter = 0;
	if (lan_active != 0) {
	    send_freq(freq);
	    if (time_master == 1)
		send_time();
	}
    }

    refresh();

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    mvprintw(12, 0, "");

    if (cluster == MAP || cluster == SPOTS) {

	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	nroflines = loadbandmap();

    }
    if (cluster == FREQWINDOW) {
	for (f = 0; f < 8; f++)
	    mvprintw(15 + f, 4, "                           ");

	if (trx_control == 0)
	    node_frequencies[thisnode - 'A'] = atof(band[bandinx]);
	else
	    node_frequencies[thisnode - 'A'] = freq;

	for (f = 0; f < MAXNODES; f++) {
	    if (node_frequencies[f] != 0)
		mvprintw(15 + f, 4, " Stn %c : %5.0f", 'A' + f,
			 node_frequencies[f]);
	}
	nicebox(14, 3, 8, 27, "Frequencies");
    }

    if (cluster == CLUSTER) {

	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	inputbuffer[0] = '\0';
	strncat(inputbuffer, backgrnd_str, 78);

	for (j = 15; j <= 22; j++) {
	    mvprintw(j, 1, "%s", inputbuffer);
	}

	inputbuffer[0] = '\0';

	k = 0;

	while (spotarray[k] > -1) {
	    k++;
	    if (k > (MAX_SPOTS - 2))
		break;
	}

	k -= 9;
	if (k < 0)
	    k = -1;

	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

	inputbuffer[0] = '\0';
	strncat(inputbuffer, backgrnd_str, 78);

	for (j = 15; j <= 22; j++) {
	    mvprintw(j, 1, "%s", inputbuffer);
	}

	for (j = 15; j <= 22; j++) {

	    if (k < (MAX_SPOTS - 2) && spotarray[++k] > -1) {
		if (k > MAX_SPOTS - 1)
		    k = MAX_SPOTS - 1;

		if (spotarray[k] >= 0 && spotarray[k] < MAX_SPOTS)
		    strcpy(inputbuffer, spot_ptr[spotarray[k]]);
		else {
		    mvprintw(24, 0, "error in packet table");
		}

		if (strlen(inputbuffer) > 14) {
		    strncat(inputbuffer, backgrnd_str, 65);
		    inputbuffer[78] = '\0';
		    mvprintw(j, 1, "%s", inputbuffer);
		} else {
		    inputbuffer[0] = '\0';
		    strncat(inputbuffer, backgrnd_str, 79);
		    inputbuffer[78] = '\0';
		    mvprintw(j, 1, "%s", inputbuffer);
		}

		inputbuffer[0] = '\0';

	    }

	}

	nicebox(14, 0, 8, 78, "Cluster");
	refresh();
    }

    printcall();

}
