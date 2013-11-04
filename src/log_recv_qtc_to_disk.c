/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013 Ervin Hegedüs - HA2OS <airween@gmail.com>
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
/*------------------------------------------------------------------------

    Log QSO to disk the received QTC line, do all necessary actions to clear qtc store

------------------------------------------------------------------------*/

#include "globalvars.h"
#include "log_recv_qtc_to_disk.h"

int log_recv_qtc_to_disk(int qsonr)
{
    char qtclogline[60], temp[20];
    int qpos = 0, i, tempi;

    for(i=0; i<10; i++) {

	if (strlen(qtcreclist.qtclines[i].time) == 4 &&
	    strlen(qtcreclist.qtclines[i].callsign) > 0 &&
	    strlen(qtcreclist.qtclines[i].serial) > 0) { // all fields are filled
	    for(qpos=0; qpos<60; qpos++) {
		qtclogline[qpos] = 32;
	    }
	  
	    qpos = 0;

	    sprintf(temp, " %04d", qsonr);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);
	    
	    sprintf(temp, " %04d", qtcreclist.serial);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d", qtcreclist.count);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);
	    
	    sprintf(temp, " %s", qtcreclist.qtclines[i].time);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %s", qtcreclist.qtclines[i].callsign);
	    tempi = strlen(temp);
	    while(tempi <= 15) {
		temp[tempi] = ' ';
		tempi++;
	    }
	    temp[tempi] = '\0';

	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    tempi = atoi(qtcreclist.qtclines[i].serial);
	    if(tempi < 1000) {
		sprintf(temp, "  %03d\n", tempi);
	    }
	    else {
		sprintf(temp, " %04d\n", tempi);
	    }
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);
	    
	    qtclogline[qpos] = '\0';

	    store_recv_qtc(qtclogline);

	    total++;
	}
    }
    for(i=0; i<10; i++) {
	qtcreclist.qtclines[i].time[0] = '\0';
	qtcreclist.qtclines[i].callsign[0] = '\0';
	qtcreclist.qtclines[i].serial[0] = '\0';
	qtcreclist.qtclines[i].status = 0;
    }

    qtcreclist.count = 0;
    qtcreclist.serial = 0;
    qtcreclist.confirmed = 0;
    qtcreclist.sentcfmall = 0;
    qtcreclist.callsign[0] = '\0';

    return (0);
}

int store_recv_qtc(char *loglineptr)
{
	FILE *fp;

	if  ( (fp = fopen(QTC_RECV_LOG, "a"))  == NULL){
		fprintf(stdout,  "log_recv_qtc_to_disk.c: Error opening file.\n");
		endwin();
		exit(1);
	}
	fputs  (loglineptr, fp);

	fclose(fp);

	return(0);
}