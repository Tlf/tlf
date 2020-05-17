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
#include "dxcc.h"
#include "err_utils.h"
#include "get_time.h"
#include "getctydata.h"
#include "lancode.h"
#include "nicebox.h"		// Includes curses.h
#include "printcall.h"
#include "tlf.h"
#include "ui_utils.h"

#define MAXMINUTES 30


extern int bandinx;
extern pthread_mutex_t spot_ptr_mutex;

char *bandmap[MAX_SPOTS];
int spotarray[MAX_SPOTS];		/* Array of indices into spot_ptr */

int loadbandmap(void);
int getclusterinfo(void);

void clusterinfo(void) {

    extern int cluster;
    extern freq_t freq;
    extern char band[NBANDS][4];
    extern int bandinx;
    extern int trx_control;
    extern char spot_ptr[MAX_SPOTS][82];
    extern freq_t node_frequencies[MAXNODES];
    extern char thisnode;

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

	if (trx_control == 0)
	    node_frequencies[thisnode - 'A'] = atof(band[bandinx]);
	else
	    node_frequencies[thisnode - 'A'] = freq;

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



/* ----------------------------------------------------*/

int loadbandmap(void) {

    extern char *bandmap[MAX_SPOTS];
    extern int xplanet;
    extern char markerfile[];
    extern char lastmsg[];
    extern char spot_ptr[MAX_SPOTS][82];
    extern int nr_of_spots;


    int i = 0, j, m, x;
    unsigned int k;
    int spotminutes = 0;
    int sysminutes = 0;
    int timediff = 0;
    int linepos;
    int spot_age[MAX_SPOTS];
    freq_t spot_freq[MAX_SPOTS];

    char thisline[83];
    char spotcall[20];
    char spottime[6];
    char spotline[38];
    char callcopy[20];
    FILE *fp;
    char marker_out[60];
    char color[sizeof("Magenta")];
    int lon;
    int lat;
    int zz;
    int nofile = 0;
    int iswarc = 0;
    char xplanetmsg[160];
    dxcc_data *dx;


    for (i = 0; i < MAX_SPOTS; i++) {
	if (bandmap[i] != NULL) {
	    g_free(bandmap[i]);
	    bandmap[i] = NULL;
	}

	spot_age[i] = 0;
	spot_freq[i] = 0.;
    }

    i = 0;

    sysminutes = get_minutes();

    /* parse log of cluster output and find DX announcements.
     * Copy them to bandmap array and find spot_age and spot_freq
     */

    pthread_mutex_lock(&spot_ptr_mutex);

    for (j = 0; j < nr_of_spots; j++) {

	g_strlcpy(thisline, spot_ptr[j], sizeof(thisline));
	if (strncmp(thisline, "DX de ", 6) == 0) {

	    g_strlcpy(spotcall, thisline + 26, 6);

	    strncpy(spottime, thisline + 70, 4);	// how old?
	    spottime[4] = spottime[3];
	    spottime[3] = spottime[2];
	    spottime[2] = ':';
	    spottime[5] = '\0';
	    spotminutes = 60 * atoi(spottime) + atoi(spottime + 3);
	    timediff = (sysminutes - spotminutes) + 5;
	    if (timediff + 30 < 0)
		timediff += 1440;

	    /* is spot recent? */
	    if ((timediff + 30) <= (MAXMINUTES + 30)) {

		/* look for duplicates already in bandmap
		 * => kill older one and keep younger entry */
		for (k = 0; k < i; k++) {
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

    linepos = (i < 8 ? 0 : i - 8);

    /* prune markerfile by opening it for write */
    if (xplanet > 0 && nofile == 0) {
	if ((fp = fopen(markerfile, "w")) == NULL) {
	    nofile = 1;			/* remember: no write possible */
	    TLF_LOG_INFO("Opening marker file not possible.");
	} else
	    fclose(fp);
    }

    for (j = linepos; j < linepos + 8; j++) {

	if (bandmap[j] != NULL) {
	    g_strlcpy(spotline, bandmap[j] + 17, 23);	// freq and call
	    g_strlcpy(spottime, bandmap[j] + 70, 6);	// time
	    strcat(spotline, spottime);

	    strncpy(callcopy, bandmap[j] + 26, 16);	// call
	    for (m = 0; m < 16; m++) {
		if (callcopy[m] == ' ') {
		    callcopy[m] = '\0';
		    break;
		}	/* use strcspn? */
	    }

	    x = getctynr(callcopy);		// CTY of station


	    if (x != 0 && xplanet > 0 && nofile == 0) {

		if ((fp = fopen(markerfile, "a")) == NULL) {
		    TLF_LOG_INFO("Opening markerfile not possible.");
		} else {

		    /* show no callsign if MARKERDOTS */
		    if (xplanet == 2)
			callcopy[0] = '\0';

		    dx = dxcc_by_index(x);
		    lon = (int)(dx -> lon) * -1;
		    lat = (int)(dx -> lat);

		    *color = '\0';

		    if (spot_age[j] > 15)
			strcat(color, "Green");
		    else {
			iswarc = 0;
			if (spot_freq[j] >= 10100.0 && spot_freq[j] <= 10150.0)
			    iswarc = 1;
			if (spot_freq[j] >= 18068.0 && spot_freq[j] <= 18168.0)
			    iswarc = 1;
			if (spot_freq[j] >= 24890.0 && spot_freq[j] <= 24990.0)
			    iswarc = 1;

			if (iswarc == 0) {
			    if (spot_freq[j] < 3500.0)
				strcat(color, "Red");
			    if (spot_freq[j] >= 3500.0
				    && spot_freq[j] <= 4000.0)
				strcat(color, "Magenta");
			    if (spot_freq[j] >= 7000.0
				    && spot_freq[j] <= 7300.0)
				strcat(color, "Yellow");
			    if (spot_freq[j] >= 14000.0
				    && spot_freq[j] <= 14350.0)
				strcat(color, "Blue");
			    if (spot_freq[j] >= 21000.0
				    && spot_freq[j] <= 21450.0)
				strcat(color, "White");
			    if (spot_freq[j] >= 28000.0
				    && spot_freq[j] <= 29700.0)
				strcat(color, "Green");

			} else {
			    strcat(color, "Cyan");
			}
		    }

		    if (*color != '\0') {
			sprintf(marker_out, "%4d   %4d   \"%s\"   color=%s\n",
				lat, lon, callcopy, color);

			fputs(marker_out, fp);
		    }


		    fclose(fp);
		}
	    }

	}
    }

    /* append last dx cluster message to markerfile; will be shown at bottom */
    if (xplanet == 1 && nofile == 0) {

	strcpy(xplanetmsg, " -82 -120 ");
	strcat(xplanetmsg, "\"");
	strcat(xplanetmsg, lastmsg);

	for (zz = 0; zz < strlen(lastmsg); zz++)
	    if (lastmsg[zz] == 34)
		lastmsg[zz] = ' ';

	strcat(xplanetmsg, "\"   color=Cyan\n");

	if ((fp = fopen(markerfile, "a")) == NULL) {
	    TLF_LOG_INFO("Opening markerfile not possible.");
	} else {
	    if (strlen(xplanetmsg) > 20)
		fputs(xplanetmsg, fp);

	    fclose(fp);
	}
    }


    bandmap_show();

    refreshp();

    return (i);			/* nr of found spot lines */
}



int getclusterinfo(void) {

    extern char spot_ptr[MAX_SPOTS][82];
    extern int nr_of_spots;
    extern int announcefilter;
    extern char call[];

    int i;
    int si;
    char calldupe[12];

    strcpy(calldupe, call);
    calldupe[strlen(call) - 1] = '\0';

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


