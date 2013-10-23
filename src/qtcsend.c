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
	 *        Handle QTC panel, send QTC, write to log
	 *
	 *--------------------------------------------------------------*/

#include "qtcsend.h"

#include "onechar.h"
#include <panel.h>
#include "genqtclist.h"
#include "nicebox.h"
#include "time_update.h"
#include "log_sent_qtc_to_disk.h"

#include "syslog.h"

extern char hiscall[];
extern int trxmode;
extern t_qtclist qtclist;

enum {
  QTCSENDWINBG = 32,
  QTCSENDLINE,
  QTCSENDINVLINE,
  QTCSENDCURRLINE
};

int qtcpanel = 0;
int currstate;
WINDOW * qtcsendwin;
PANEL * qtcsend_panel;

int qtc_send_panel() {
    char qtchead[32];
    int i, j, x;
    int nrpos = 0;

    init_pair(QTCSENDWINBG,   COLOR_BLUE,   COLOR_GREEN);
    init_pair(QTCSENDLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCSENDINVLINE, COLOR_YELLOW, COLOR_CYAN);
    init_pair(QTCSENDCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);

    int line_inverted = COLOR_PAIR(QTCSENDINVLINE) | A_BOLD;
    int line_currinverted = COLOR_PAIR(QTCSENDCURRLINE) | A_BOLD;
    int line_currnormal = COLOR_PAIR(QTCSENDCURRLINE) | A_NORMAL;
    int line_normal = COLOR_PAIR(QTCSENDLINE) | A_NORMAL;
    
    //if (qtclist.count == 0 || strcmp(hiscall, qtclist.callsign) != 0) {
    if (qtclist.count == 0) {
	j = genqtclist(hiscall);
    }
    else {
	j = qtclist.count;
    }

    if (qtcpanel == 0) {
      qtcsendwin = newwin(13, 35, 10, 2);
      qtcsend_panel = new_panel(qtcsendwin);
      hide_panel(qtcsend_panel);
      qtcpanel = 1;
    }
    show_panel(qtcsend_panel);
    top_panel(qtcsend_panel);
    currstate = curs_set(0);
    werase(qtcsendwin);

    sprintf(qtchead, "QTC #%d send to %s", qtclist.serial, hiscall);
    wnicebox(qtcsendwin, 0, 0, 11, 33, qtchead);
    mvwprintw(qtcsendwin, 12, 2, " QTC - F2: all | ENT: curr ");
    wbkgd(qtcsendwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCSENDWINBG)));
    wattrset(qtcsendwin, line_inverted);
    mvwprintw(qtcsendwin, 1, 1, "                                 ");
    for(i=0; i<10; i++) {
	if(qtclist.qtclines[i].flag == 1) {
	    wattrset(qtcsendwin, line_inverted);
	}
	else {
	    wattrset(qtcsendwin, line_normal);
	}
	mvwprintw(qtcsendwin, i+2, 1, "                                 ");
    }

    wattrset(qtcsendwin, line_inverted);
    mvwprintw(qtcsendwin, 1, 4, "%d/%d", qtclist.serial, qtclist.marked);

    for(i=0; i<qtclist.count; i++) {
	if (qtclist.qtclines[i].flag == 1) {
	    wattrset(qtcsendwin, line_inverted);
	}
	else {
	    wattrset(qtcsendwin, line_normal);
	}

	//if (i+1 == j) {		// default current line
	if (i == 0) {		// default current line
	    if (qtclist.qtclines[i].flag == 1) {
		wattrset(qtcsendwin, line_currinverted);
	    }
	    else {
		wattrset(qtcsendwin, line_currnormal);
	    }
	    mvwprintw(qtcsendwin, i+2, 1, "                                 ");
	}
	nrpos = (i<9) ? 2 : 1;
	mvwprintw(qtcsendwin, i+2, nrpos, "%d", i+1);
	mvwprintw(qtcsendwin, i+2, 4, "%s", qtclist.qtclines[i].qtc);
	if (qtclist.qtclines[i].sent == 1) {
	    mvwprintw(qtcsendwin, i+2, 30, "*");
	}
    }
    i=1;
    refreshp();

    x = -1;
    while(x != 27) {

	usleep(10000);
	time_update();
	x = onechar();

	switch(x) {
	  case 152:		// up
		    if (i > 1) {
			i--;
			// set the new position
			if (qtclist.qtclines[i-1].flag == 1) {
			    wattrset(qtcsendwin, line_currinverted);
			}
			else {
			    wattrset(qtcsendwin, line_currnormal);
			}
			mvwprintw(qtcsendwin, i+1, 1, "                                 ");
			nrpos = (i<10) ? 2 : 1;
			mvwprintw(qtcsendwin, i+1, nrpos, "%d", i);
			mvwprintw(qtcsendwin, i+1, 4, "%s", qtclist.qtclines[i-1].qtc);
			if (qtclist.qtclines[i-1].sent == 1) {
			    mvwprintw(qtcsendwin, i+1, 30, "*");
			}

			// set the old position
			if (qtclist.qtclines[i].flag == 1) {
			    wattrset(qtcsendwin, line_inverted);
			}
			else {
			    wattrset(qtcsendwin, line_normal);
			}
			mvwprintw(qtcsendwin, i+2, 1, "                                 ");
			nrpos = (i<9) ? 2 : 1;
			mvwprintw(qtcsendwin, i+2, nrpos, "%d", (i+1));
			mvwprintw(qtcsendwin, i+2, 4, "%s", qtclist.qtclines[i].qtc);
			if (qtclist.qtclines[i].sent == 1) {
			    mvwprintw(qtcsendwin, i+2, 30, "*");
			}
		      
		    }
		    break;
	  case 153:		// down
		    if (i < qtclist.count) {
			i++;
			// set the new position
			if (qtclist.qtclines[i-1].flag == 1) {
			    wattrset(qtcsendwin, line_currinverted);
			}
			else {
			    wattrset(qtcsendwin, line_currnormal);
			}
			mvwprintw(qtcsendwin, i+1, 1, "                                 ");
			nrpos = (i<10) ? 2 : 1;
			mvwprintw(qtcsendwin, i+1, nrpos, "%d", i);
			mvwprintw(qtcsendwin, i+1, 4, "%s", qtclist.qtclines[i-1].qtc);
			if (qtclist.qtclines[i-1].sent == 1) {
			    mvwprintw(qtcsendwin, i+1, 30, "*");
			}
			// set the old position
			if (qtclist.qtclines[i-2].flag == 1) {
			    wattrset(qtcsendwin, line_inverted);
			}
			else {
			    wattrset(qtcsendwin, line_normal);
			}
			mvwprintw(qtcsendwin, i, 1, "                                 ");
			nrpos = (i<11) ? 2 : 1;
			mvwprintw(qtcsendwin, i, nrpos, "%d", (i-1));
			mvwprintw(qtcsendwin, i, 4, "%s", qtclist.qtclines[i-2].qtc);
			if (qtclist.qtclines[i-2].sent == 1) {
			    mvwprintw(qtcsendwin, i, 30, "*");
			}
		    }
		    break;
	  /* case 160:		// INS
		    if (qtclist.qtclines[i-1].flag == 0) {
			qtclist.qtclines[i-1].flag = 1;
			qtclist.marked++;
			wattrset(qtcsendwin, line_inverted);
			mvwprintw(qtcsendwin, 1, 4, "%d/%d", qtclist.serial, qtclist.marked);
			wattrset(qtcsendwin, line_currinverted);
		    }
		    else {
			qtclist.qtclines[i-1].flag = 0;
			qtclist.marked--;
			wattrset(qtcsendwin, line_inverted);
			mvwprintw(qtcsendwin, 1, 4, "%d/%d", qtclist.serial, qtclist.marked);
			wattrset(qtcsendwin, line_currnormal);
		    }
		    mvwprintw(qtcsendwin, i+1, 1, "                                 ");
		    nrpos = (i<10) ? 2 : 1;
		    mvwprintw(qtcsendwin, i+1, nrpos, "%d", i);
		    mvwprintw(qtcsendwin, i+1, 4, "%s", qtclist.qtclines[i-1].qtc);
		    break; */
	  case 10:  		// ENTER
		    if (qtclist.totalsent == 0) {
			// TODO
			qtclist.totalsent = 0;	// sending QTC serial and nr of QTC
			// TODO END
		    }
		    if (qtclist.qtclines[i-1].sent == 0) {
			qtclist.qtclines[i-1].sent = 1;
			qtclist.totalsent++;
			if (qtclist.qtclines[i-1].flag == 1) {
			    wattrset(qtcsendwin, line_currinverted);
			}
			else {
			    wattrset(qtcsendwin, line_currnormal);
			}
			mvwprintw(qtcsendwin, i+1, 30, "*");
		    }
		    i = scroll_down(i);
		    break;
	  case 130:		// F2
		    // send QTC serial and nr of QTC
		    for(j=0; j<qtclist.count; j++) {
			qtclist.qtclines[j].sent = 1;
			if (j == i-1) {
			    wattrset(qtcsendwin, line_currinverted);
			}
			else {
			    wattrset(qtcsendwin, line_inverted);
			}
			mvwprintw(qtcsendwin, j+2, 30, "*");
		    }
		    break;
	  case 161:  		// DELETE
		    if (qtclist.qtclines[i-1].sent == 1) {
			qtclist.qtclines[i-1].sent = 0;
			qtclist.totalsent--;
			if (qtclist.qtclines[i-1].flag == 1) {
			    wattrset(qtcsendwin, line_currinverted);
			}
			else {
			    wattrset(qtcsendwin, line_currnormal);
			}
			mvwprintw(qtcsendwin, i+1, 30, " ");
		    }
		    i = scroll_down(i);
		    break;
	  case 138:			// SHIFT + Fn
		    x = onechar();
		    if (x == 81) {	// shift + F2
			for(j=0; j<qtclist.count; j++) {
			    qtclist.qtclines[j].sent = 0;
			    if (j == i-1) {
				wattrset(qtcsendwin, line_currinverted);
			    }
			    else {
				wattrset(qtcsendwin, line_inverted);
			    }
			    mvwprintw(qtcsendwin, j+2, 30, " ");
			}
		    }
	}
	refreshp();
    }
    hide_panel(qtcsend_panel);
    curs_set(currstate);
    //x = onechar();

    return 0;
}

int scroll_down(int i) {
    int nrpos;

    init_pair(QTCSENDWINBG,   COLOR_BLUE,   COLOR_GREEN);
    init_pair(QTCSENDLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCSENDINVLINE, COLOR_YELLOW, COLOR_CYAN);
    init_pair(QTCSENDCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);

    int line_inverted = COLOR_PAIR(QTCSENDINVLINE) | A_BOLD;
    int line_currinverted = COLOR_PAIR(QTCSENDCURRLINE) | A_BOLD;
    int line_currnormal = COLOR_PAIR(QTCSENDCURRLINE) | A_NORMAL;
    int line_normal = COLOR_PAIR(QTCSENDLINE) | A_NORMAL;
    
    if (i < qtclist.count) {
	i++;
	// set the new position
	if (qtclist.qtclines[i-1].flag == 1) {
	    wattrset(qtcsendwin, line_currinverted);
	}
	else {
	    wattrset(qtcsendwin, line_currnormal);
	}
	mvwprintw(qtcsendwin, i+1, 1, "                                 ");
	nrpos = (i<10) ? 2 : 1;
	mvwprintw(qtcsendwin, i+1, nrpos, "%d", i);
	mvwprintw(qtcsendwin, i+1, 4, "%s", qtclist.qtclines[i-1].qtc);
	if (qtclist.qtclines[i-1].sent == 1) {
	    mvwprintw(qtcsendwin, i+1, 30, "*");
	}
	// set the old position
	if (qtclist.qtclines[i-2].flag == 1) {
	    wattrset(qtcsendwin, line_inverted);
	}
	else {
	    wattrset(qtcsendwin, line_normal);
	}
	mvwprintw(qtcsendwin, i, 1, "                                 ");
	nrpos = (i<11) ? 2 : 1;
	mvwprintw(qtcsendwin, i, nrpos, "%d", (i-1));
	mvwprintw(qtcsendwin, i, 4, "%s", qtclist.qtclines[i-2].qtc);
	if (qtclist.qtclines[i-2].sent == 1) {
	    mvwprintw(qtcsendwin, i, 30, "*");
	}
    }
    return i;
}
