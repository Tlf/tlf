/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011-2014      Thomas Beierlein <tb@forth-ev.de>
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
 *
 *          clusterinfo +  time update
 *--------------------------------------------------------------*/


#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "bandmap.h"
#include "bands.h"
#include "dxcc.h"
#include "err_utils.h"
#include "get_time.h"
#include "getctydata.h"
#include "globalvars.h"
#include "lancode.h"
#include "nicebox.h"		// Includes curses.h
#include "printcall.h"
#include "splitscreen.h"
#include "ui_utils.h"

#define MAXMINUTES 30

char *bandmap[MAX_SPOTS];
int spotarray[MAX_SPOTS];		/* Array of indices into spot_ptr */

void loadbandmap(void);
int getclusterinfo(void);

void clusterinfo(void) {

    int f, j, k;
    char inputbuffer[160] = "";


    /* cluster and bandmap display */
    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    mvprintw(12, 0, "");

    if (cluster == NOCLUSTER) {
	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	for (int i = 14; i < LINES - 1; i++)
	    mvprintw(i, 0, backgrnd_str);
	refreshp();

    }

    if (cluster == MAP) {

	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);

	loadbandmap();

    }

    if (cluster == FREQWINDOW) {
	for (f = 0; f < 8; f++)
	    mvprintw(15 + f, 4, "                           ");

	if (trx_control)
	    node_frequencies[thisnode - 'A'] = freq;
	else
	    node_frequencies[thisnode - 'A'] = atof(band[bandinx]);

	for (f = 0; f < MAXNODES; f++) {
	    if (node_frequencies[f] != 0)
		mvprintw(15 + f, 4, " Stn %c : %5.0f", 'A' + f,
			 node_frequencies[f] / 1000.0);
	}
	nicebox(14, 3, 8, 27, "Frequencies");
    }

    if (cluster == CLUSTER) {

	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);

	g_strlcpy(inputbuffer, backgrnd_str, 79);

	for (j = 15; j <= LINES - 3; j++) {
	    mvprintw(j, 1, "%s", inputbuffer);
	}


	/** \todo minimize lock time */
	pthread_mutex_lock(&spot_ptr_mutex);

	k = getclusterinfo();

	if (k > (MAX_SPOTS - 1)) {
	    k = MAX_SPOTS - 1;
	}

	k -= (LINES - 3 - 14) + 1;
	if (k < 0)
	    k = -1;


	for (j = 15; j <= LINES - 3; j++) {
	    k++;
	    if (k < (MAX_SPOTS - 1) && spotarray[k] > -1) {
		if (k > MAX_SPOTS - 1)
		    k = MAX_SPOTS - 1;

		inputbuffer[0] = '\0';

		if (spotarray[k] >= 0 && spotarray[k] < MAX_SPOTS)
		    g_strlcpy(inputbuffer, spot_ptr[spotarray[k]], 79);
		else {
		    TLF_LOG_INFO("error in packet table");
		}

		if (strlen(inputbuffer) > 14) {
		    mvprintw(j, 1, "%s", inputbuffer);
		}
	    }
	}

	pthread_mutex_unlock(&spot_ptr_mutex);

	nicebox(14, 0, LINES - 3 - 14, 78, "Cluster");
	refreshp();
    }
    printcall();
}


void show_xplanet() {

    int i = 0, j;
    int sysminutes = 0;
    int linepos;
    int spot_age[MAX_SPOTS];
    freq_t spot_freq[MAX_SPOTS];

    char callcopy[20];
    FILE *fp;
    dxcc_data *dx;
    static bool nofile = false;


    if (xplanet == 0 || nofile == true) {
	return;
    }

    /* prune markerfile by opening it for write */
    if ((fp = fopen(markerfile, "w")) == NULL) {
	TLF_LOG_INFO("Opening marker file not possible.");
	nofile = true;		/* remember: no write possible */
	return;
    }

    memset (spot_age, '\0', sizeof(spot_age));
    memset (spot_freq, '\0', sizeof(spot_freq));

    for (i = 0; i < MAX_SPOTS; i++) {
	if (bandmap[i] != NULL) {
	    g_free(bandmap[i]);
	    bandmap[i] = NULL;
	}
    }

    /* parse log of cluster output and find DX announcements.
     * Copy them to bandmap array and find spot_age and spot_freq
     */
    sysminutes = get_minutes();
    i = 0;

    pthread_mutex_lock(&spot_ptr_mutex);

    for (j = 0; j < nr_of_spots; j++) {
	char thisline[83];
	char spottime[6];
	char spotcall[20];

	g_strlcpy(thisline, spot_ptr[j], sizeof(thisline));
	if (strncmp(thisline, "DX de ", 6) == 0 && strlen(thisline) >= 74) {
	    int spotminutes;
	    int timediff;

	    g_strlcpy(spotcall, thisline + 26, 6);

	    /* read and convert hours and minutes to spotminutes */
	    spottime[0] = thisline[70];
	    spottime[1] = thisline[71];
	    spottime[2] = '\0';
	    spotminutes = atoi(spottime);

	    spottime[0] = thisline[72];
	    spottime[1] = thisline[73];
	    spottime[2] = '\0';
	    spotminutes = 60 * spotminutes + atoi(spottime);

	    timediff = (sysminutes - spotminutes) + 5;
	    if (timediff + 30 < 0)
		timediff += 1440;

	    /* is spot recent? */
	    if (timediff <= MAXMINUTES) {

		/* look for duplicates already in bandmap
		 * => kill older one and keep younger entry */
		for (int k = 0; k < i; k++) {
		    g_strlcpy(callcopy, bandmap[k] + 26, 6);

		    if (strncmp(callcopy, spotcall, 4) == 0) {
			bandmap[k][0] = 'd';
			break;
		    }
		}

		bandmap[i] = g_strdup(thisline);
		spot_age[i] = timediff;
		spot_freq[i] = atof(thisline + 17);
		i++;
	    }
	}
    }

    pthread_mutex_unlock(&spot_ptr_mutex);


    /* show last 8 spots via xplanet */
    linepos = (i < 8 ? 0 : i - 8);

    for (j = linepos; j < linepos + 8; j++) {

	if (bandmap[j] != NULL) {
	    char marker_out[60];
	    int lon;
	    int lat;
	    int ctynr;
	    char *color;
	    static char *bandcolor[NBANDS] = {"Red", "Magenta", "Cyan",
		    "Yellow", "Cyan", "Blue",
		    "Cyan", "White", "Cyan",
		    "Green", NULL };

	    strncpy(callcopy, bandmap[j] + 26, 16);	// call
	    for (int m = 0; m < 16; m++) {
		if (callcopy[m] == ' ') {
		    callcopy[m] = '\0';
		    break;
		}	/* use strcspn? */
	    }

	    ctynr = getctynr(callcopy);		// CTY of station

	    if (ctynr != 0 ) {
		/* show no callsign if MARKERDOTS */
		if (xplanet == 2)
		    callcopy[0] = '\0';

		dx = dxcc_by_index(ctynr);
		lon = (int)(dx -> lon) * -1;
		lat = (int)(dx -> lat);

		if (spot_age[j] > 15)
		    strcat(color, "Green");
		else {
		    color = bandcolor[freq2band(spot_freq[j] * 1000)];
		}

		if (color != NULL) {
		    sprintf(marker_out, "%4d   %4d   \"%s\"   color=%s\n",
			    lat, lon, callcopy, color);

		    fputs(marker_out, fp);
		}
	    }
	}
    }

    /* append last dx cluster message to markerfile;
     * will be shown at bottom */
    if (xplanet == 1) {
	char xplanetmsg[160];

	strcpy(xplanetmsg, " -82 -120 ");
	strcat(xplanetmsg, "\"");
	g_strlcat(xplanetmsg, lastmsg, sizeof(xplanetmsg));

	for (int i = 0; i < strlen(lastmsg); i++)
	    if (lastmsg[i] == 34)
		lastmsg[i] = ' ';

	g_strlcat(xplanetmsg, "\"   color=Cyan\n", sizeof(xplanetmsg));

	if (strlen(xplanetmsg) > 20){
	    fputs(xplanetmsg, fp);
	}
    }
    fclose(fp);
}


/* ----------------------------------------------------*/

void loadbandmap(void) {

    show_xplanet();
    bandmap_show();
    refreshp();
}



int getclusterinfo(void) {

    int i;
    int si;
    char calldupe[12];

    strcpy(calldupe, my.call);
    calldupe[strlen(my.call) - 1] = '\0';

    for (si = 0; si < (MAX_SPOTS - 2); si++)
	spotarray[si] = -1;

    i = 0;
    si = 0;

    while (1) {

	if (strstr(spot_ptr[i], "DX de") != NULL) {
	    spotarray[si] = i;
	    si++;

	} else if (strstr(spot_ptr[i], calldupe) != NULL) {
	    if ((announcefilter <= 2)) {
		spotarray[si] = i;
		si++;
	    }

	} else if (strstr(spot_ptr[i], "To ALL") != NULL) {
	    if ((announcefilter <= 1)) {
		spotarray[si] = i;
		si++;
	    }

	} else if ((announcefilter == 0)
		   && (strlen(spot_ptr[i]) > 20)) {
	    spotarray[si] = i;
	    si++;
	}

	i++;

	if (i > (nr_of_spots - 1))
	    break;
    }

    return si;
}


