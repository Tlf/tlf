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
	/* ------------------------------------------------------------
	 *        Handle QTC panel, receiv QTC, write to log
	 *
	 *--------------------------------------------------------------*/

#include "qtcrecv.h"

#include "onechar.h"
#include <panel.h>
#include "nicebox.h"
#include "time_update.h"
#include <ctype.h>
#include "rtty.h"
#include "sendbuf.h"
#include "log_recv_qtc_to_disk.h"
#include "globalvars.h"
#include "qtcutil.h"
#include "speedup.h"
#include "speeddown.h"
#include "cw_utils.h"

extern char hiscall[];
extern int trxmode;
extern t_qtcreclist qtcreclist;
extern int keyerport;
extern char buffer[];
extern int nr_qsos;

enum {
  QTCRECVWINBG = 32,
  QTCRECVLINE,
  QTCRECVINVLINE,
  QTCRECVCURRLINE,
  QTCRECVBG
};

int qtcrecvpanel = 0;
int currrecstate;
WINDOW * qtcrecvwin;
PANEL * qtcrecv_panel;
t_qtcfieldset fieldset;
// array values: hor position, cursor position, field len to fill with spaces
int pos[6][3] = {{3, 3, 15}, {3, 6, 4}, {8, 8, 2}, {3, 3, 4}, {8, 8, 14}, {24, 24, 4}};
int curpos = 0;
int curfieldlen = 0;
static char last_rtty_line[2][50] = {"", ""};	// local copy and store to remain
static char prevqtccall[15] = "";
int curr_rtty_line = 0;
int capturing = 0;
char helpmsg[6][2][26] = {
  { "Check the callsign and", "press [TAB]" },
  { "Enter the QTC serial,", "and press [TAB]" },
  { "Enter the QTC number,", "and press [TAB]" },
  { "Enter the time of QSO,", "and press [TAB]" },
  { "Enter the callsign of", "QSO, and press [TAB]" },
  { "Enter the serial of QSO,", "and press [TAB]" },
};
int nrofqtc = 0;

int qtc_recv_panel() {
    char qtchead[32];
    int i, j, x;
    int tfi;
    int currqtc = -1;

    capturing = 0;
    last_rtty_line[0][0] = '\0'; last_rtty_line[1][0] = '\0';
    init_pair(QTCRECVWINBG,   COLOR_BLUE,   COLOR_GREEN);
    init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
    init_pair(QTCRECVCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);

    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
    //int line_currinverted = COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD;
    //int line_currnormal = COLOR_PAIR(QTCRECVCURRLINE) | A_NORMAL;
    //int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;
    
    if (strlen(hiscall) > 0) {
	strncpy(qtcreclist.callsign, hiscall, strlen(hiscall));
    }

    if (strcmp(qtcreclist.callsign, prevqtccall) != 0 || strlen(qtcreclist.callsign) == 0) {
	strcpy(buffer, "QTC QRV ");
	sendbuf();
	qtcreclist.count = 0;
	qtcreclist.serial = 0;
	qtcreclist.confirmed = 0;
	qtcreclist.sentcfmall = 0;
	for(i=0; i<10; i++) {
	    qtcreclist.qtclines[i].status = 0;
	    qtcreclist.qtclines[i].time[0] = '\0';
	    qtcreclist.qtclines[i].callsign[0] = '\0';
	    qtcreclist.qtclines[i].serial[0] = '\0';
	    qtcreclist.qtclines[i].confirmed = 0;
	}
	fieldset.active = 0;
	fieldset.qtcreclist = qtcreclist;
	curr_rtty_line = 0;
    }
    strncpy(prevqtccall, qtcreclist.callsign, strlen(qtcreclist.callsign));
    
    if (qtcrecvpanel == 0) {
      qtcrecvwin = newwin(14, 65, 9, 2);
      qtcrecv_panel = new_panel(qtcrecvwin);
      hide_panel(qtcrecv_panel);
      qtcrecvpanel = 1;
    }
    show_panel(qtcrecv_panel);
    top_panel(qtcrecv_panel);
    //currrecstate = curs_set(0);
    werase(qtcrecvwin);

    sprintf(qtchead, "QTC receive");
    wnicebox(qtcrecvwin, 0, 0, 12, 33, qtchead);
    sprintf(qtchead, "HELP");
    wnicebox(qtcrecvwin, 0, 35, 12, 28, qtchead);
    //mvwprintw(qtcrecvwin, 12, 2, " QTC - F2: all | ENT: curr ");
    wbkgd(qtcrecvwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVWINBG)));
    wattrset(qtcrecvwin, line_inverted);
    mvwprintw(qtcrecvwin, 1, 1, "                                 ");
    mvwprintw(qtcrecvwin, 2, 1, "      /                          ");

    showfield(0);	// QTC CALL
    showfield(1);	// QTC serial
    showfield(2);	// QTC nr of row

    clear_help_block();
    
    wattrset(qtcrecvwin, line_inverted);
    for(i=0; i<10; i++) {
	wattrset(qtcrecvwin, line_inverted);
	mvwprintw(qtcrecvwin, i+3, 1, "                                 ");
	for(j=0; j<3; j++) {
	    showfield((i*3)+j+3);	// QTC fields...
	}
    }
    number_fields();
    showfield(fieldset.active);
    curpos = 0;
    i=1;

    refreshp();

    x = -1;
    while(x != 27) {

        nodelay(stdscr, TRUE);

	while (x < 1) {

	    usleep(5000);
	    time_update();
	    if (trxmode == DIGIMODE && (keyerport == GMFSK
	           || keyerport == MFJ1278_KEYER)) {
	        show_rtty();
	    }
	    x = onechar();
	  
	}
        nodelay(stdscr, FALSE);

	switch(x) {
	  case 227:		// ALT-c
		    if (trxmode == DIGIMODE) {
			readqtcfromfile();
		    }
	  case 152:		// up
		    if (fieldset.active > 0) {	// nr of QTC record field idx
			if (fieldset.active == 1 || fieldset.active == 2) {
			    fieldset.active = 0;
			    showfield(1);
			    showfield(2);
			}
			else if (fieldset.active == 3 || fieldset.active == 4) {
			    fieldset.active -= 2;
			    showfield(fieldset.active+2);
			}
			else {
			    fieldset.active -= 3;
			    showfield(fieldset.active+3);
			}
		    }
		    showfield(fieldset.active);
		    curpos = 0;
		    break;
	  case 153:		// down
		    if (fieldset.active < 30) {	// last line serial field idx
		        if (fieldset.active == 0) {
			    fieldset.active = 1;
			    showfield(0);
			}
			else if (fieldset.active == 1 || fieldset.active == 2) {
			    fieldset.active += 2;
			    showfield(1);
			    showfield(2);
			}
			else {
			    fieldset.active += 3;
			    showfield(fieldset.active-3);
			}
		    }
		    showfield(fieldset.active);
		    curpos = 0;
		    break;
	  case 155:	// left
		    if (curpos < curfieldlen) {	// curpos is a shift, means lenght - position
		       curpos++;
		       showfield(fieldset.active);
		    }
		    break;
	  case 154:	// right
		    if (curpos > 0) {
			curpos--;
			showfield(fieldset.active);
		    }
		    break;
	  case 10:  		// ENTER
		    if (fieldset.active > 2) {
			currqtc = ((fieldset.active-3)/3);
			//if ((fieldset.active-3)%3 == 2) {
			// TODO - is it need to restrict only for last field to allow the ENTER?
			if (1) {
			    if (qtcreclist.qtclines[currqtc].status == 0 &&
				strlen(qtcreclist.qtclines[currqtc].time) == 4 &&
				strlen(qtcreclist.qtclines[currqtc].callsign) > 0 &&
				strlen(qtcreclist.qtclines[currqtc].serial) > 0
			    ) {
				qtcreclist.qtclines[currqtc].status = 2;
				show_status(currqtc);
				if (currqtc < qtcreclist.count) {
				    if (trxmode == DIGIMODE) {
					qtcreclist.qtclines[currqtc].confirmed = 1; // compatibility for other modes
					qtcreclist.confirmed++;
					fieldset.active+=3;	// go to next line exch field
					showfield(fieldset.active-3);
				    }
				    else {
				        if (qtcreclist.qtclines[currqtc].confirmed == 0) {
					    qtcreclist.qtclines[currqtc].confirmed = 1;
					    qtcreclist.confirmed++;
					    sprintf(buffer, "R ");
					    sendbuf();
					}
					tfi = (fieldset.active-3)%3;
					//fieldset.active++;	// go to next line time field
					fieldset.active += (3-tfi);
					showfield(fieldset.active-(3-tfi));
				    }
				    showfield(fieldset.active);
				}
			    }
			    else if (qtcreclist.qtclines[currqtc].status == 1 && qtcreclist.qtclines[currqtc].confirmed != 1) {
				if (trxmode == CWMODE) {
				    sprintf(buffer, "AGN ");
				    sendbuf();
				}
				if (trxmode == DIGIMODE) {
				    sprintf(buffer, "%02d PSE AGN", currqtc+1);
				    sendbuf();
				}
			    }

			    if (qtcreclist.confirmed == qtcreclist.count) {
				if (qtcreclist.sentcfmall == 0) {
				    if (log_recv_qtc_to_disk(nr_qsos) == 0) {
					qtcreclist.sentcfmall = 1;
					// TODO
					// send 'CFM all' to station
					if (trxmode == DIGIMODE) {
					    strcpy(buffer, "QSL ALL QTC");
					    sendbuf();
					}
					if (trxmode == CWMODE) {
					    strcpy(buffer, "CFM ALL TNX ");
					    sendbuf();
					}
					// TODO
				    }
				}
				x = 27;	// close the window
			    }
			}
		    }
		    break;
	  case 130:		// F2
		    for(j=0; j<qtcreclist.count; j++) {
			if (qtcreclist.qtclines[j].status == 0 &&
			    strlen(qtcreclist.qtclines[j].time) == 4 &&
			    strlen(qtcreclist.qtclines[j].callsign) > 0 &&
			    strlen(qtcreclist.qtclines[j].serial) > 0
			) {
			    qtcreclist.confirmed++;
			    qtcreclist.qtclines[j].status = 2;
			    show_status(j);
			}
		    }
		    showfield(fieldset.active);
		    break;
	  case 161:  		// DELETE
		    delete_from_field(0);
		    break;
	  case 138:			// SHIFT + Fn
		    x = onechar();
		    if (x == 81) {	// shift + F2
			for(j=0; j<qtcreclist.count; j++) {
			    if (qtcreclist.qtclines[j].status == 2) {
				qtcreclist.confirmed--;
			    }
			    qtcreclist.qtclines[j].status = 0;
			    show_status(j);
			}
			showfield(fieldset.active);
		    }
		    break;
	  case 9:		// TAB
		    if (trxmode == DIGIMODE) {
			if (fieldset.active == 32) {
			    fieldset.active = 0;
			    showfield(32);
			}
			else {
			    fieldset.active++;
			    showfield(fieldset.active-1);
			}
		    }
		    else {
			if ((fieldset.active < 3) || ((fieldset.active-3)%3 >= 0 && (fieldset.active-3)%3 < 2)) {
			    fieldset.active++;
			    showfield(fieldset.active-1);
			}
			else {
			    if ((fieldset.active-3)%3 == 2) {
				fieldset.active -= 2;
				showfield(fieldset.active+2);
			    }
			}
		    }
		    curpos = 0;
		    showfield(fieldset.active);
		    break;
	  case 90:		// SHIFT + TAB
		    if (trxmode == DIGIMODE) {
			if (fieldset.active == 0) {
			    fieldset.active = 32;
			    showfield(0);
			}
			else {
			    fieldset.active--;
			    showfield(fieldset.active+1);
			}
		    }
		    else {
			if ((fieldset.active < 3) || ((fieldset.active-3)%3 > 0 && (fieldset.active-3)%3 <= 2)) {
			    fieldset.active--;
			    showfield(fieldset.active+1);
			}
			else {
			    if ((fieldset.active-3)%3 == 0) {
				fieldset.active += 2;
				showfield(fieldset.active-2);
			    }
			}
		    }
		    showfield(fieldset.active);
		    curpos = 0;
		    break;
	  case 32:	// space
		    modify_field(x);
		    break;
	  case 48 ... 57:	// numbers
		    if (trxmode == DIGIMODE && fieldset.active > 0 && fieldset.active < 3 && qtcreclist.count == 0 && capturing == 0) {
			capturing = 1;
			wattrset(qtcrecvwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
			mvwprintw(qtcrecvwin, 1, 19, "CAPTURE ON ");
		    }
		    modify_field(x);
		    break;
	  /*case 65 ... 90:	// letters
		    modify_field(x);
		    break;*/
	  case 97 ... 122:	// letters
		    modify_field(x-32);
		    break;	  
	  case 47:	// '/' sign
		    modify_field(x);
		    break;
	  case 63:	// '?' sign
		    modify_field(x);
		    break;
	  case 127:	// backspace
		    delete_from_field(1);
		    break;
	  case 156:	// up
		    speedup();
		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2d", GetCWSpeed());
		    break;
	  case 157:	// down
		    speeddown();
		    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		    mvprintw(0, 14, "%2d", GetCWSpeed());
		    break;
	}

	refreshp();
	if (x != 27) {
	    x = 0;
	}
    }
    hide_panel(qtcrecv_panel);
    capturing = 0;
    //curs_set(currrecstate);
    //x = onechar();

    return 0;
}

int showfield(int fidx) {

	char fieldval[20], filled[20];
	int qtcrow, winrow, fi, posidx, i;

	init_pair(QTCRECVWINBG,   COLOR_BLUE,   COLOR_GREEN);
	init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
	init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
	init_pair(QTCRECVCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);

	//int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
	int line_currinverted = COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD;
	int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;

	posidx = 0;
	if (fidx == 0) {
	    sprintf(fieldval, "%s", qtcreclist.callsign);
	    winrow = 1;
	    posidx = 0;
	    nrofqtc = qtc_get(qtcreclist.callsign, bandinx);
	    if (nrofqtc < 0) {
		nrofqtc = 0;
	    }
	    put_qtc();
	}
	else if (fidx == 1) {
	    sprintf(fieldval, "%4d", qtcreclist.serial);
	    winrow = 2;
	    posidx = 1;
	}
	else if (fidx == 2) {
	    sprintf(fieldval, "%d", qtcreclist.count);
	    winrow = 2;
	    posidx = 2;
	}
	else {
	    fi = fidx-3;
	    winrow = (fi/3)+3;
	    qtcrow = winrow-3;
	    switch(fi%3) {
		case 0:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].time);
			posidx = 3;
			break;
		case 1:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].callsign);
			posidx = 4;
			break;
		case 2:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].serial);
			posidx = 5;
			break;
	    }
	    show_status(qtcrow);
	}

	curfieldlen = strlen(fieldval);

	for(i=0; i<pos[posidx][2]; i++) {
	    filled[i] = ' ';
	}
	filled[i] = '\0';
	if (fidx == fieldset.active) {
	    wattrset(qtcrecvwin, line_currinverted);
	}
	else {
	    wattrset(qtcrecvwin, line_normal);
	}

	mvwprintw(qtcrecvwin, winrow, pos[posidx][0], filled);
	mvwprintw(qtcrecvwin, winrow, pos[posidx][0], "%s", fieldval);
	if (fidx == fieldset.active) {
	    show_help_msg(posidx);
	    if (pos[posidx][0] == pos[posidx][1]) {
		wmove(qtcrecvwin, winrow, (pos[posidx][1] + strlen(fieldval))-curpos);
	    }
	    else {
		wmove(qtcrecvwin, winrow, pos[posidx][1]-curpos);
	    }
	}

	return 0;
}

int modify_field(int pressed) {
	char fieldval[16];
	int fi, winrow, qtcrow, posidx, stridx;

	posidx = 0;
	if (fieldset.active == 0 && (isalnum(pressed) || pressed == '/') && strlen(qtcreclist.callsign) < pos[0][2]-1) {
	    shift_right(qtcreclist.callsign);
	    qtcreclist.callsign[strlen(qtcreclist.callsign)-curpos] = pressed;
	    qtcreclist.callsign[strlen(qtcreclist.callsign)+1] = '\0';
	    if (curpos > 0) {
		curpos--;
	    }
	    showfield(0);
	    nrofqtc = qtc_get(qtcreclist.callsign, bandinx);
	    if (nrofqtc < 0) {
		nrofqtc = 0;
	    }
	    strncpy(prevqtccall, qtcreclist.callsign, strlen(qtcreclist.callsign));
	}
	else if (fieldset.active == 1 && isdigit(pressed)) {
	    sprintf(fieldval, "%d", (qtcreclist.serial)*10);
	    shift_right(fieldval);
	    fieldval[strlen(fieldval)-(1+curpos)] = pressed;
	    fieldval[strlen(fieldval)] = '\0';
	    if (curpos > 0) {
		curpos--;
	    }
	    if(strlen(fieldval) <= pos[1][2]) {
	      qtcreclist.serial = atoi(fieldval);
	      showfield(1);
	    }
	}
	else if (fieldset.active == 2 && isdigit(pressed)) {
	    sprintf(fieldval, "%d", qtcreclist.count*10);
	    shift_right(fieldval);
	    fieldval[strlen(fieldval)-(1+curpos)] = pressed;
	    fieldval[strlen(fieldval)] = '\0';
	    if (curpos > 0) {
		curpos--;
	    }
	    if(strlen(fieldval) <= pos[2][2] && atoi(fieldval) <= 10) {
	      qtcreclist.count = atoi(fieldval);
	      number_fields();
	      showfield(2);
	    }
	}
	else if (fieldset.active >= 3 && fieldset.active <= 32) {
	    fi = fieldset.active-3;
	    winrow = (fi/3)+3;
	    qtcrow = winrow-3;
	    stridx = fi%3;
	    switch(stridx) {
		case 0:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].time);
			posidx = 3;
			break;
		case 1:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].callsign);
			posidx = 4;
			break;
		case 2:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].serial);
			posidx = 5;
			break;
	    }
	    if (pressed == '?') {
		//if (qtcreclist.qtclines[qtcrow].status == 2) {
		    //qtcreclist.confirmed--;
		//}
		qtcreclist.qtclines[qtcrow].status = 1;	// set incomplete the qtc status
		show_status(qtcrow);
	    }
	    if ( ( ( (stridx == 0 || stridx == 2) && (isdigit(pressed) || pressed == '?') ) || stridx == 1) && strlen(fieldval) < pos[posidx][2]) {
		shift_right(fieldval);
		fieldval[strlen(fieldval)-curpos] = pressed;
		fieldval[strlen(fieldval)+1] = '\0';
		switch(stridx) {
		    case 0: strcpy(qtcreclist.qtclines[qtcrow].time, fieldval);
			    break;
		    case 1: strcpy(qtcreclist.qtclines[qtcrow].callsign, fieldval);
			    break;
		    case 2: strcpy(qtcreclist.qtclines[qtcrow].serial, fieldval);
			    break;
		}
		if (curpos > 0) {
		    curpos--;
		}
		showfield(fieldset.active);
		if (stridx == 0 && strlen(fieldval) == pos[posidx][2] && fieldset.active <= 32) {	// auto TAB if curr field is QTC time, and len is 4
		    curpos = 0;
		    fieldset.active++;
		    showfield(fieldset.active-1);
		    showfield(fieldset.active);
		}
	    }
	    
	}
	return 0;
}

int delete_from_field(int dir) {

	char fieldval[16];
	int fi, winrow, qtcrow, stridx;

	if (fieldset.active == 0) {
	    if (strlen(qtcreclist.callsign) > 0) {
		sprintf(fieldval, "%s", qtcreclist.callsign);
		shift_left(fieldval, dir);
		strcpy(qtcreclist.callsign, fieldval);
		showfield(0);
	    }
	}
	else if (fieldset.active == 1) {
	    sprintf(fieldval, "%d", qtcreclist.serial);
	    if(strlen(fieldval) >= 0) {
	      //sprintf(fieldval, "%d", qtcreclist.serial/10);
	      shift_left(fieldval, dir);
	      qtcreclist.serial = atoi(fieldval);
	      showfield(1);
	    }
	}
	else if (fieldset.active == 2) {
	    sprintf(fieldval, "%d", qtcreclist.count);
	    if(strlen(fieldval) >= 0) {
	      //sprintf(fieldval, "%d", qtcreclist.count/10);
	      shift_left(fieldval, dir);
	      qtcreclist.count = atoi(fieldval);
	      showfield(2);
	    }
	}
	else {
	    fi = fieldset.active-3;
	    winrow = (fi/3)+3;
	    qtcrow = winrow-3;
	    stridx = fi%3;
	    switch(stridx) {
		case 0:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].time);
			break;
		case 1:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].callsign);
			break;
		case 2:	sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].serial);
			break;
	    }

	    if (strlen(fieldval) > 0) {
		//fieldval[strlen(fieldval)-2] = '\0';
		shift_left(fieldval, dir);
		//strncpy(newfieldval, fieldval, strlen(fieldval)-2);
		switch(stridx) {
		    case 0: strcpy(qtcreclist.qtclines[qtcrow].time, fieldval);
			    break;
		    case 1: strcpy(qtcreclist.qtclines[qtcrow].callsign, fieldval);
			    break;
		    case 2: strcpy(qtcreclist.qtclines[qtcrow].serial, fieldval);
			    break;
		}
		showfield(fieldset.active);
	    }
	    
	}
	return 0;
}

int shift_right(char * fieldval) {
	int i;
	for(i=strlen(fieldval); i<strlen(fieldval)-curpos; i--) {
	    fieldval[i] = fieldval[i-1];
	}
	fieldval[strlen(fieldval)+1] = '\0';
	return 0;
}

int shift_left(char * fieldval, int shift) {
	int i;
	for(i=strlen(fieldval)-(curpos+shift); i<strlen(fieldval); i++) {
	    fieldval[i] = fieldval[i+1];
	}
	if (shift == 0) {
	  curpos--;
	}
	fieldval[strlen(fieldval)] = '\0';
	return 0;
}

int show_status(int idx) {

	char flag = ' ';
	int i, status = 0;

	init_pair(QTCRECVWINBG,   COLOR_BLUE,   COLOR_GREEN);
	init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
	init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
	init_pair(QTCRECVCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);
	init_pair(QTCRECVBG,      COLOR_BLUE,   COLOR_CYAN);

	//int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
	//int line_currinverted = COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD;
	//int line_currnormal = COLOR_PAIR(QTCRECVCURRLINE) | A_NORMAL;
	//int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;

	status = 0;
	if (idx < qtcreclist.count) {
	    if (strlen(qtcreclist.qtclines[idx].time) != 4) {
		status = 1;
	    }
	    else {
		for(i=0;i<strlen(qtcreclist.qtclines[idx].time);i++) {
		    if (qtcreclist.qtclines[idx].time[i] == '?' || ! isdigit(qtcreclist.qtclines[idx].time[i])) {
			status = 1;
			break;
		    }
		}
	    }
	    if (strlen(qtcreclist.qtclines[idx].callsign) < 3) {
		status = 1;
	    }
	    else {
		for(i=0;i<strlen(qtcreclist.qtclines[idx].callsign);i++) {
		    if (qtcreclist.qtclines[idx].callsign[i] == '?') {
			status = 1;
			break;
		    }
		}
	    }
	    if (strlen(qtcreclist.qtclines[idx].serial) == 0) {
		status = 1;
	    }
	    for(i=0;i<strlen(qtcreclist.qtclines[idx].serial);i++) {
		if (qtcreclist.qtclines[idx].serial[i] == '?' || ! isdigit(qtcreclist.qtclines[idx].serial[i])) {
		    status = 1;
		    break;
		}
	    }
	    if (status == 1) {
		/*if (qtcreclist.qtclines[idx].status == 2) {
		    qtcreclist.confirmed--;
		}*/
		qtcreclist.qtclines[idx].status = 1;
	    }
	    else if (qtcreclist.qtclines[idx].status != 2) {	// unset incomplete mark if not marked as complete
		qtcreclist.qtclines[idx].status = 0;
	    }

	    switch(qtcreclist.qtclines[idx].status) {
		case 0:	flag = ' ';
			    break;
		case 1:	flag = '?';
			    break;
		case 2:	flag = '*';
			    break;
	    }
	    wattrset(qtcrecvwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
	    mvwprintw(qtcrecvwin, idx+3, 30, "%c", flag);
	}
	return 0;
}

int number_fields() {
    int i;

    init_pair(QTCRECVBG,      COLOR_BLUE,   COLOR_CYAN);
    wattrset(qtcrecvwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
    for(i=0;i<10;i++) {
	mvwprintw(qtcrecvwin, i+3, 1, "  ");
    }
    for(i=0;i<qtcreclist.count;i++) {
	mvwprintw(qtcrecvwin, i+3, 1, "%2d", i+1);
    }
    return 0;
}

int readqtcfromfile() {
    FILE *fp;
    char line[50], temps[20];
    int linenr, i, j, capidx0, capidx1, capidx2;

    if ((fp = fopen("RTTY.qtc", "r"))) {
	linenr = 0;
	while(fgets(line, 50, fp) && linenr < 11) {
	    if (linenr == 0) {
		j=0; i=0;
		temps[0] = '\0';
		while(! isdigit(line[j])) {
		    j++;
		}
		while(isdigit(line[j])) {
		    temps[i] = line[j];
		    i++; j++;
		}
		temps[i] = '\0';
		qtcreclist.serial = atoi(temps);
		showfield(0);
		while(! isdigit(line[j])) {
		    j++;
		}
		i=0;
		while(isdigit(line[j])) {
		    temps[i] = line[j];
		    i++; j++;
		}
		temps[i] = '\0';
		qtcreclist.count = atoi(temps);
		showfield(1);
	    }
	    else {
		j=0;
		// skip chars until it isn't number
		while(! isdigit(line[j])) {
		    j++;
		}
		temps[0] = '\0';
		i=0;
		// parse time
		while(isdigit(line[j]) && i<4) {
		    temps[i]=line[j];
		    i++; j++;
		}
		while(i<4) {
		    temps[i] = '\?';
		    i++;
		}
		temps[i] = '\0';
		strncpy(qtcreclist.qtclines[linenr-1].time, temps, i);

		showfield(2+((linenr-1)*3));
		capidx0 = j;

		// parse serial
		j=strlen(line)-1;
		while(! isdigit(line[j])) {
		    j--;
		}
		capidx2=j+1;
		i=0;
		while(isdigit(line[j]) && i<4) {
		    j--;
		    i++;
		}
		capidx1=j;
		j++;
		i=0;
		while(j<capidx2) {
		    temps[i] = line[j];
		    i++; j++;
		}
		temps[i] = '\0';
		while(i < 3) {
		    temps[i] = '?';
		    temps[i+1] = '\0';
		    i++;
		}
		strncpy(qtcreclist.qtclines[linenr-1].serial, temps, i);
		showfield(2+((linenr-1)*3)+3);

		// parse callsign
		j = capidx0;
		while(! isalnum(line[j])) {
		    j++;
		}
		i=0;
		while((isalnum(line[j]) || line[j] == '/') && i<15) {
		    temps[i] = line[j];
		    i++; j++;
		}
		temps[i] = '\0';
		strncpy(qtcreclist.qtclines[linenr-1].callsign, temps, i);
		showfield(2+((linenr-1)*3)+2);
	      
	    }
	    linenr++;
	}
	fclose(fp);
    }

    return 0;
}

int clear_help_block() {
    int i;


    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;

    wattrset(qtcrecvwin, line_inverted);
    for(i=1;i<13;i++) {
	mvwprintw(qtcrecvwin, i, 36, "                            ");
    }
    return 0;
}

int show_help_msg(msgidx) {
    clear_help_block();
    int i = 0;

    while(helpmsg[msgidx][i][0] != '\0') {
	mvwprintw(qtcrecvwin, i+1, 36, helpmsg[msgidx][i]);
	i++;
    }

    return 0;
}

int put_qtc() {

    init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;

    wattrset(qtcrecvwin, line_normal);
    mvwprintw(qtcrecvwin, 1, 19, "%2dQ on %d", nrofqtc, atoi(band[bandinx]));
    return 0;
}

/* int move_cursor(int dir) {
	int fi, winrow;

	if (fieldset.active == 0 || fieldset.active == 1) {
	    winrow = 1;
	}
	else {
	    fi = fieldset.active-2;
	    winrow = (fi/3)+2;
	}
	wmove(qtcrecvwin, winrow, pos[posidx][1]);
} */

