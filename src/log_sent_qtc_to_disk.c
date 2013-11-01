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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*------------------------------------------------------------------------

    Log QSO to disk the QTC line, do all necessary actions to clear qtc store

------------------------------------------------------------------------*/

#include "globalvars.h"
#include "log_sent_qtc_to_disk.h"

pthread_mutex_t qtc_disk_mutex = PTHREAD_MUTEX_INITIALIZER;

//int log_sent_qtc_to_disk(int qtcnr)
int log_sent_qtc_to_disk(int qsonr)
{
    char qtclogline[60], temp[10];
    int qpos = 0, i;
    int has_empty = 0;
    int last_qtc = 0;

    for(i=0; i<10; i++) {

	if (qtclist.qtclines[i].saved == 0 && qtclist.qtclines[i].flag == 1 && qtclist.qtclines[i].sent == 1) { // not saved and marked for sent
	    for(qpos=0; qpos<60; qpos++) {
		qtclogline[qpos] = 32;
	    }
	  
	    qpos = 0;
	    sprintf(temp, " %04d", qtclist.serial);
	    strncpy(qtclogline, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d", qtclist.qtclines[i].qsoline);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(temp, " %04d", qsonr);
	    strncpy(qtclogline+qpos, temp, strlen(temp));
	    qpos+=strlen(temp);

	    sprintf(qtclogline+qpos, " %s\n", qtclist.qtclines[i].qtc);
	    qpos += strlen(qtclist.qtclines[i].qtc)+3;
	    qtclogline[qpos-1] = '\0';

	    store_sent_qtc(qtclogline);
	    
	    // mark qso as sent as qtc
	    qsoflags_for_qtc[qtclist.qtclines[i].qsoline] = 1;
	    if (qtclist.qtclines[i].qsoline > last_qtc) {
		last_qtc = qtclist.qtclines[i].qsoline;
	    }
	    // check if prev qso callsign is the current qtc window,
	    // and excluded from list; if true, set the next_qtc_qso to that
	    // else see below, the next qtc window pointer will set to
	    // next qso after the current window
	    if (qtclist.qtclines[i].qsoline > 0 && qsoflags_for_qtc[qtclist.qtclines[i].qsoline-1] == 0) {
		has_empty = 1;
		next_qtc_qso = qtclist.qtclines[i].qsoline-1;
	    }

	    qtclist.count = 0;
	    qtclist.marked = 0;
	    qtclist.totalsent = 0;
	    // set next_qtc_qso pointer to next qso line,
	    // if the list is continous
	    if (has_empty == 0) {
		next_qtc_qso = qtclist.qtclines[i].qsoline+1;
	    }
	    //next_qtc_qso = qtclist.qtclines[i].qsoline+1;
	    nr_qtcsent++;
	    // totalpoints++??
	    total++;
	}
    }
    for(i=0; i<last_qtc; i++) {
	if (qsoflags_for_qtc[i] == 0) {
	    next_qtc_qso = i;
	    break;
	}
    }
    for(i=0; i<10; i++) {
	qtclist.qtclines[i].qtc[0] = '\0';
	//qtclines.qtclines[i].qsoline = 0;
	qtclist.qtclines[i].flag = 0;
	qtclist.qtclines[i].saved = 0;
	qtclist.qtclines[i].sent = 0;
    }
    return (0);
}

int store_sent_qtc(char *loglineptr)
{
	FILE *fp;

	if  ( (fp = fopen(QTC_SENT_LOG, "a"))  == NULL){
		fprintf(stdout,  "log_sent_qtc_to_disk.c: Error opening file.\n");
		endwin();
		exit(1);
	}
	fputs  (loglineptr, fp);

	fclose(fp);

	return(0);
}