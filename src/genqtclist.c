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
	/* ------------------------------------------------------------
	 *        Generate QTC list to send
	 *
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "genqtclist.h"

#include <syslog.h>

int genqtclist(char * callsign)
{
    int qtclistlen;
    int s = 0, i = 0;

    //qtclistlen = ((nr_qsos - next_qtc_qso) > 10) ? 10 : (nr_qsos - next_qtc_qso);
    qtclistlen = 10;
    qtclist.serial = nr_qtcsent+1;
    qtclist.marked = 0;
    qtclist.totalsent = 0;
    qtclist.count = 0;
    strncpy(qtclist.callsign, callsign, strlen(callsign));
    for(s = 0; s < qtclistlen; s++) {
	qtclist.qtclines[s].qtc[0] = '\0';
	qtclist.qtclines[s].flag = 0;
	qtclist.qtclines[s].saved = 0;
	qtclist.qtclines[s].sent = 0;
    }

    s=next_qtc_qso;
    syslog(LOG_DEBUG, "%d - %d", s, qtclistlen);
    // while(s<next_qtc_qso+qtclistlen) {
    while (qtclist.count < qtclistlen && s < nr_qsos) {
	if (strncmp(qsos[s]+29, callsign, strlen(callsign)) != 0) {	// exclude current callsign
	  if (qsoflags_for_qtc[s] == 0) {
	      genqtcline(qtclist.qtclines[i].qtc, qsos[s]);
	      if (trxmode == DIGIMODE) {
		  qtclist.qtclines[i].flag = 1;
		  qtclist.marked++;
	      }
	      else {
		  if (i == 0) {
		      qtclist.qtclines[i].flag = 1;
		      qtclist.marked++;
		  }
		  else {
		      qtclist.qtclines[i].flag = 0;
		  }
	      }
	      qtclist.qtclines[i].qsoline = s;
	      qtclist.count++;
	      syslog(LOG_DEBUG, "%d: %s", i, qsos[s]);
	      i++;
	  }
	}
	//else {
	  // if current callsign is in next 10 qso, it will be skipped
	  // try to increment the list to 10
	  //if ((nr_qsos - next_qtc_qso) > 11) {
	  //if ((nr_qsos - next_qtc_qso) < 11) {
	  //  qtclistlen--;
	  //}
	//}
	s++;
    }
    //qtclist.count = i;

    return qtclist.count;
}

int genqtcline(char * qtc, char * line) {
    int i, qpos;

    strncpy(qtc, line+17, 2);
    strncpy(qtc+2, line+20, 2);
    qtc[4] = ' ';
    qpos = 5;

    for(i=29; line[i] != 32; i++) {
	strncpy(qtc+qpos, line+i, 1);
	qpos++;
    }
    while(qpos<20) {
       qtc[qpos] = 32;
       qpos++;
    }
    strncpy(qtc+qpos, line+53, 4);
    qpos += 4;
    qtc[qpos] = '\0';

    return 0;
}