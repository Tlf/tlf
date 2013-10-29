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
	 *        Handle QTC panel, receiv QTC, write to log
	 *
	 *--------------------------------------------------------------*/

#include "qtcrecv.h"

#include "onechar.h"
#include <panel.h>
#include "nicebox.h"
#include "time_update.h"
/* #include "log_sent_qtc_to_disk.h" */

#include "syslog.h"

extern char hiscall[];
extern int trxmode;
extern t_qtcreclist qtcreclist;

enum {
  QTCRECVWINBG = 32,
  QTCRECVLINE,
  QTCRECVINVLINE,
  QTCRECVCURRLINE
};

int qtcrecvpanel = 0;
int currrecstate;
WINDOW * qtcrecvwin;
PANEL * qtcrecv_panel;

int qtc_recv_panel() {
    char qtchead[32];
    int i, j, x;
    int nrpos = 0;
syslog(LOG_DEBUG, "in recv");
    init_pair(QTCRECVWINBG,   COLOR_BLUE,   COLOR_GREEN);
    init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
    init_pair(QTCRECVCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);

    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
    int line_currinverted = COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD;
    int line_currnormal = COLOR_PAIR(QTCRECVCURRLINE) | A_NORMAL;
    int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;
    
    if (strcmp(hiscall, qtcreclist.callsign) != 0) {
	for(i=0; i<10; i++) {
	    qtcreclist.qtclines[i].time[0] = '\0';
	    qtcreclist.qtclines[i].callsign[0] = '\0';
	    qtcreclist.qtclines[i].serial[0] = '\0';
	}
    }

    if (qtcrecvpanel == 0) {
      qtcrecvwin = newwin(13, 35, 10, 2);
      qtcrecv_panel = new_panel(qtcrecvwin);
      hide_panel(qtcrecv_panel);
      qtcrecvpanel = 1;
    }
    show_panel(qtcrecv_panel);
    top_panel(qtcrecv_panel);
    currrecstate = curs_set(0);
    werase(qtcrecvwin);

    sprintf(qtchead, "QTC receive from %s", hiscall);
    wnicebox(qtcrecvwin, 0, 0, 11, 33, qtchead);
    //mvwprintw(qtcrecvwin, 12, 2, " QTC - F2: all | ENT: curr ");
    wbkgd(qtcrecvwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVWINBG)));
    wattrset(qtcrecvwin, line_inverted);
    mvwprintw(qtcrecvwin, 1, 1, "                                 ");

    wattrset(qtcrecvwin, line_inverted);
    mvwprintw(qtcrecvwin, 1, 4, "%d/%d", qtcreclist.serial, qtcreclist.count);


    i=1;
    refreshp();

    x = -1;
    while(x != 27) {

	usleep(10000);
	time_update();
	x = onechar();

	switch(x) {
	  case 152:		// up
		    break;
	  case 153:		// down
		    break;
	  case 10:  		// ENTER
		    break;
	  case 130:		// F2
		    break;
	  case 161:  		// DELETE
		    break;
	  case 138:			// SHIFT + Fn
		    x = onechar();
		    //if (x == 81) {	// shift + F2
		    //}
		     break;
	}

	refreshp();
    }
    hide_panel(qtcrecv_panel);
    curs_set(currrecstate);
    //x = onechar();

    return 0;
}

