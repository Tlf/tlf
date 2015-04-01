/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013-2014 Ervin Hegedüs - HA2OS <airween@gmail.com>
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
	 *        Handle QTC panel, receive and send QTC, write to log
	 *
	 *--------------------------------------------------------------*/

#include "qtcwin.h"

#include "ui_utils.h"
#include <panel.h>
#include "nicebox.h"
#include "time_update.h"
#include <ctype.h>
#include "rtty.h"
#include "sendbuf.h"
#include "log_recv_qtc_to_disk.h"
#include "log_sent_qtc_to_disk.h"
#include "globalvars.h"
#include "qtcutil.h"
#include "genqtclist.h"
#include "speedupndown.h"
#include "cw_utils.h"
#include "keyer.h"
#include "callinput.h"
#include "get_time.h"
#include "write_keyer.h"

#include <sys/time.h>

#define DIRCLAUSE (direction == RECV) || (direction == SEND && (activefield == 0 || activefield == 2))

extern char hiscall[];
extern char lastcall[];
extern int trxmode;
extern t_qtcreclist qtcreclist;
extern t_qtclist qtclist;
extern int keyerport;
extern int nr_qsos;
extern char qtc_recv_msgs[12][80];
extern char qtc_send_msgs[12][80];
extern char qtc_phrecv_message[14][80];
extern char qtc_phsend_message[14][80];
extern int data_ready;
extern struct t_qtc_store_obj *qtc_temp_obj;
extern int qtcrec_record;
extern char qtcrec_record_command[2][50];
extern char qtcrec_record_command_shutdown[50];
extern char wkeyerbuffer[];
extern t_qtc_ry_line qtc_ry_lines[QTC_RY_LINE_NR];
extern int qtc_ry_currline;
extern int qtc_ry_capture;

enum {
  QTCRECVWINBG = 32,
  QTCRECVLINE,
  QTCRECVINVLINE,
  QTCRECVCURRLINE,
  QTCRECVBG,
  KEYERLINE
};

char sentqtc_queue[11][30];
int sentqtc_p;

int qtcpanel = 0;
int currrecstate;
WINDOW * qtcwin;
PANEL * qtc_panel;
WINDOW * ry_win;
PANEL * ry_panel;
WINDOW * ry_help_win;
PANEL * ry_help_panel;
int activefield;
// array values: hor position, cursor position, field len to fill with spaces
int pos[6][3] = {{3, 3, 15}, {3, 6, 4}, {8, 8, 2}, {3, 3, 4}, {8, 8, 14}, {24, 24, 4}};
int curpos = 0;
int curfieldlen = 0;
static char last_rtty_line[2][50] = {"", ""};	// local copy and store to remain
static char prevqtccall[15] = "";
int curr_rtty_line = 0;
int capturing = 0;
char help_rec_msgs[6][26] = {
    "Enter callsign",
    "Enter the QTC serial",
    "Enter the QTC number",
    "Enter the time",
    "Enter the CALL",
    "Enter the SERIAL"
};
char help_send_msgs[7][26] = {
    "Enter callsign",
    "",
    "Enter the QTC number",
    "Press ENTER to send QTC",
    "",
    "",
    "Press CTRL+S to SAVE!"
};

char * qtccallsign;
int * qtccount;
int * qtccurrdiretion;

int qtc_main_panel(int direction) {
    char qtchead[32], tempc[40];
    int i, j, x;
    int tfi, tlen = 0;
    int currqtc = -1;
    char reccommand[100] = "";

    capturing = 0;
    last_rtty_line[0][0] = '\0'; last_rtty_line[1][0] = '\0';
    init_pair(QTCRECVWINBG,   COLOR_BLUE,   COLOR_GREEN);
    init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
    init_pair(QTCRECVCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);
    init_pair(KEYERLINE,      COLOR_WHITE,  COLOR_BLACK);

    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
    int line_currinverted = COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD;
    int line_currnormal = COLOR_PAIR(QTCRECVCURRLINE) | A_NORMAL;
    int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;
    attr_t attributes;
    short cpair;

    static int record_run = -1;

    if (strlen(hiscall) > 0) {
	if (direction == RECV) {
	    strncpy(qtcreclist.callsign, hiscall, strlen(hiscall));
	    qtcreclist.callsign[strlen(hiscall)] = '\0';
	}
	if (direction == SEND) {
	    strncpy(qtclist.callsign, hiscall, strlen(hiscall));
	    qtclist.callsign[strlen(hiscall)] = '\0';
	}
    }
    else if (strlen(lastcall) > 0) {
	if (direction == RECV) {
	    strncpy(qtcreclist.callsign, lastcall, strlen(lastcall));
	    qtcreclist.callsign[strlen(lastcall)] = '\0';
	}
	if (direction == SEND) {
	    strncpy(qtclist.callsign, lastcall, strlen(lastcall));
	    qtclist.callsign[strlen(lastcall)] = '\0';
	}
    }

    qtccurrdiretion = &direction;

    if (direction == RECV) {
	if (strcmp(qtcreclist.callsign, prevqtccall) != 0 || strlen(qtcreclist.callsign) == 0) {
	    qtcreclist.count = 0;
	    qtcreclist.serial = 0;
	    qtcreclist.confirmed = 0;
	    qtcreclist.sentcfmall = 0;
	    for(i=0; i<10; i++) {
		qtcreclist.qtclines[i].status = 0;
		qtcreclist.qtclines[i].time[0] = '\0';
		qtcreclist.qtclines[i].callsign[0] = '\0';
		qtcreclist.qtclines[i].serial[0] = '\0';
		qtcreclist.qtclines[i].receivedtime[0] = '\0';
		qtcreclist.qtclines[i].confirmed = 0;
	    }
	    activefield = 0;
	    curr_rtty_line = 0;
	    //qtc_ry_currline = 0;
	    qtc_ry_copied = 0;
	}
	if (qtcreclist.count == 0) {
	    activefield = 0;
	    curr_rtty_line = 0;
	}
	strncpy(prevqtccall, qtcreclist.callsign, strlen(qtcreclist.callsign));
	qtccallsign = qtcreclist.callsign;
	qtccount = &qtcreclist.count;
    }
    if (direction == SEND) {
	if (strcmp(qtclist.callsign, prevqtccall) != 0 || strlen(qtclist.callsign) == 0 || qtclist.count == 0) {
	    qtc_temp_obj = qtc_get(qtclist.callsign);
	    j = genqtclist(qtclist.callsign, (10-(qtc_temp_obj->total)));
	    activefield = 0;
	}
	else {
	    j = qtclist.count;
	}
	strncpy(prevqtccall, qtclist.callsign, strlen(qtclist.callsign));
	qtccallsign = qtclist.callsign;
	qtccount = &qtclist.count;
    }

    if (qtcpanel == 0) {
      qtcwin = newwin(14, 75, 9, 2);
      qtc_panel = new_panel(qtcwin);
      hide_panel(qtc_panel);
      qtcpanel = 1;
      ry_win = newwin(14, 40, 9, 37);
      ry_panel = new_panel(ry_win);
      hide_panel(ry_panel);
      ry_help_win = newwin(5, 75, 4, 2);
      ry_help_panel = new_panel(ry_help_win);
      hide_panel(ry_help_panel);
    }
    show_panel(qtc_panel);
    top_panel(qtc_panel);
    //currrecstate = curs_set(0);
    werase(qtcwin);

    if (direction == RECV) {
      sprintf(qtchead, "QTC receive");
    }
    if (direction == SEND) {
      sprintf(qtchead, "QTC send");
    }
    wnicebox(qtcwin, 0, 0, 12, 33, qtchead);
    sprintf(qtchead, "HELP");
    wnicebox(qtcwin, 0, 35, 12, 38, qtchead);
    //mvwprintw(qtcwin, 12, 2, " QTC - F2: all | ENT: curr ");
    wbkgd(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVWINBG)));
    wattrset(qtcwin, line_inverted);
    mvwprintw(qtcwin, 1, 1, "                                 ");
    if (direction == RECV) {
	mvwprintw(qtcwin, 2, 1, "      /                          ");
    }
    if (direction == SEND) {
	mvwprintw(qtcwin, 2, 1, "     %d/%2d                        ", qtclist.serial, qtclist.count);
    }

    showfield(0);	// QTC CALL
    if (direction == RECV) {
      showfield(1);	// QTC serial
    }
    showfield(2);	// QTC nr of row

    clear_help_block();

    if (direction == RECV) {
	wattrset(qtcwin, line_inverted);
	for(i=0; i<10; i++) {
	    wattrset(qtcwin, line_inverted);
	    mvwprintw(qtcwin, i+3, 1, "                                 ");
	    for(j=0; j<3; j++) {
		showfield((i*3)+j+3);	// QTC fields...
	    }
	}
	number_fields();
    }
    if (direction == SEND) {
	show_sendto_lines();
    }
    if (*qtccurrdiretion == RECV && trxmode == DIGIMODE) {
	wattrset(qtcwin, line_inverted);
	if (qtc_ry_capture == 0) {
	    mvwprintw(qtcwin, 2, 11, "CAPTURE OFF");
	}
	else {
	    mvwprintw(qtcwin, 2, 11, "CAPTURE ON ");
	    show_rtty_lines();
	}
	wattrset(qtcwin, line_normal);
    }

    showfield(activefield);
    curpos = 0;
    i=1;

    refreshp();

    x = -1;
    while(x != 27) {

	while (x < 1) {

	    usleep(5000);
	    time_update();
	    if (trxmode == DIGIMODE && (keyerport == GMFSK
	           || keyerport == MFJ1278_KEYER)) {
	        show_rtty();
	    }
	    x = key_poll();

	}

	switch(x) {
	  //case 227:		// ALT-c
	  case 20:		// CTRL-t
		    if (trxmode == DIGIMODE) {
			show_rtty_lines();
		    }
	  case 152:		// up
		    if (direction == RECV) {
			if (activefield > 0) {	// nr of QTC record field idx
			    if (activefield == 1 || activefield == 2) {
				activefield = 0;
				showfield(1);
				showfield(2);
			    }
			    else if (activefield == 3 || activefield == 4) {
				activefield -= 2;
				showfield(activefield+2);
			    }
			    else {
				activefield -= 3;
				showfield(activefield+3);
			    }
			}
			showfield(activefield);
			curpos = 0;
		    }
		    if (direction == SEND && ((trxmode != DIGIMODE && qtclist.totalsent < *qtccount) || (trxmode == DIGIMODE && qtclist.totalsent == *qtccount))) {
			switch (activefield) {
			    case 0: activefield = 12;
				    show_help_msg(activefield);
				    showfield(0);
				    wattrset(qtcwin, line_currinverted);
				    mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[9].qtc);
				    break;
			    case 2: activefield = 0;
				    showfield(2);
				    showfield(0);
				    break;
			    case 3: activefield = 2;
				    showfield(2);
				    wattrset(qtcwin, line_normal);
				    mvwprintw(qtcwin, activefield+1, 4, "%s", qtclist.qtclines[0].qtc);
				    break;
			    case 4 ... 12:
				    show_help_msg(activefield);
				    wattrset(qtcwin, line_normal);
				    mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[(activefield-3)].qtc);
				    activefield--;
				    wattrset(qtcwin, line_currinverted);
				    mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[(activefield-3)].qtc);
				    break;
			}
		    }
		    break;
	  case 153:		// down
		    if (direction == RECV) {
			if (activefield < 30) {	// last line serial field idx
			    if (activefield == 0) {
				activefield = 1;
				showfield(0);
			    }
			    else if (activefield == 1 || activefield == 2) {
				activefield += 2;
				showfield(1);
				showfield(2);
			    }
			    else {
				activefield += 3;
				showfield(activefield-3);
			    }
			}
			showfield(activefield);
			curpos = 0;
		    }
		    if (direction == SEND && ((trxmode != DIGIMODE && qtclist.totalsent < *qtccount) || (trxmode == DIGIMODE && qtclist.totalsent == *qtccount))) {
			switch (activefield) {
			    case 0: activefield = 2;
				    showfield(0);
				    showfield(2);
				    break;
			    //case 2: activefield = 3;
				    //showfield(2);
				    //break;
			    case 2: activefield = 3;
				    show_help_msg(activefield);
				    showfield(2);
				    wattrset(qtcwin, line_currinverted);
				    mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[0].qtc);
				    break;
			    case 3 ... 11:
				    show_help_msg(activefield);
				    wattrset(qtcwin, line_normal);
				    mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[(activefield-3)].qtc);
				    activefield++;
				    wattrset(qtcwin, line_currinverted);
				    mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[(activefield-3)].qtc);
				    break;
			    case 12:
				    activefield = 0;
				    wattrset(qtcwin, line_normal);
				    mvwprintw(qtcwin, 14, 4, "%s", qtclist.qtclines[9].qtc);
				    showfield(0);
				    break;
			}
		    }
		    break;
	  case 155:	// left
		    if (DIRCLAUSE) {
			if (curpos < curfieldlen) {	// curpos is a shift, means lenght - position
			    curpos++;
			    showfield(activefield);
			}
		    }
		    break;
	  case 154:	// right
		    if (DIRCLAUSE) {
			if (curpos > 0) {
			    curpos--;
			    showfield(activefield);
			}
		    }
		    break;
	  case 10:  		// ENTER
		    if (activefield > 2) {
			if (direction == RECV) {
			    currqtc = ((activefield-3)/3);
			    //if ((activefield-3)%3 == 2) {
			    // TODO - is it need to restrict only for last field to allow the ENTER?
			    if (1) {
				if (qtcreclist.qtclines[currqtc].status == 0 &&
				    strlen(qtcreclist.qtclines[currqtc].time) == 4 &&
				    strlen(qtcreclist.qtclines[currqtc].callsign) > 0 &&
				    strlen(qtcreclist.qtclines[currqtc].serial) > 0
				) {
				    get_time();
				    tempc[0] = '\0';
				    strftime(tempc, 40, "%d-%b-%y %H:%M", time_ptr);
				    strncpy(qtcreclist.qtclines[currqtc].receivedtime, tempc, 15);
				    qtcreclist.qtclines[currqtc].receivedtime[15] = '\0';
				    qtcreclist.qtclines[currqtc].status = 2;
				    show_status(currqtc);
				    if (currqtc < *qtccount) {
					if (trxmode == DIGIMODE) {
					    qtcreclist.qtclines[currqtc].confirmed = 1; // compatibility for other modes
					    qtcreclist.confirmed++;
					    activefield+=3;	// go to next line exch field
					    showfield(activefield-3);
					}
					else {
					    if (qtcreclist.qtclines[currqtc].confirmed == 0) {
						qtcreclist.qtclines[currqtc].confirmed = 1;
						qtcreclist.confirmed++;
						if (trxmode == CWMODE) {
						    sendmessage(qtc_recv_msgs[2]);
						}
						if (trxmode == SSBMODE) {
						    play_file(qtc_phrecv_message[2]);
						}
					    }
					    tfi = (activefield-3)%3;
					    //activefield++;	// go to next line time field
					    activefield += (3-tfi);
					    showfield(activefield-(3-tfi));
					}
					showfield(activefield);
				    }
				}
				else if (qtcreclist.qtclines[currqtc].status == 1 && qtcreclist.qtclines[currqtc].confirmed != 1) {
				    if (trxmode == CWMODE) {
					sendmessage(qtc_recv_msgs[7]);
				    }
				    if (trxmode == SSBMODE) {
					play_file(qtc_phrecv_message[7]);
				    }
				    if (trxmode == DIGIMODE) {
					char *str = g_strdup_printf("%s %02d %02d\n", g_strchomp(qtc_recv_msgs[7]), currqtc+1, currqtc+1);
					sendmessage(str);
					g_free(str);
				    }
				}

				if (*qtccount > 0 && qtcreclist.confirmed == *qtccount) {
				    if (qtcreclist.sentcfmall == 0) {
					qtcreclist.sentcfmall = 1;
					if (log_recv_qtc_to_disk(nr_qsos) == 0) {
					    // TODO
					    // send 'CFM all' to station
					  if (qtcrec_record == 1 && record_run > -1) {
					      strcpy(reccommand, "pkill -SIGINT -n ");
					      strcat(reccommand, qtcrec_record_command_shutdown);
					      system(reccommand);
					      record_run = -1;
					  }
					  if (trxmode == DIGIMODE || trxmode == CWMODE) {
						sendmessage(qtc_recv_msgs[9]);
					    }
					    if (trxmode == SSBMODE) {
						play_file(qtc_phrecv_message[9]);
					    }
					    // TODO
					}
				    }
				    x = 27;	// close the window
				}
			    }
			}
			if (direction == SEND && trxmode != DIGIMODE) {
			    if (qtclist.qtclines[activefield-3].sent == 0) {
				qtclist.qtclines[activefield-3].sent = 1;
				get_time();
				tempc[0] = '\0';
				strftime(tempc, 40, "%d-%b-%y %H:%M", time_ptr);
				strncpy(qtclist.qtclines[activefield-3].senttime, tempc, 15);
				qtclist.qtclines[activefield-3].senttime[15] = '\0';
				qtclist.totalsent++;
			    }
			    tempc[0] = '\0';
			    replace_spaces(qtclist.qtclines[activefield-3].qtc, tempc);
			    data_ready = 1;
			    if (trxmode == CWMODE) {
				sendmessage(tempc);
			    }

			    mvwprintw(qtcwin, activefield, 30, "*");
			    qtclist.qtclines[activefield-3].flag = 1;
			    // scroll down if not at end of qtclist:
			    if (activefield-3 < *qtccount-1) {
				wattrset(qtcwin, line_normal);
				mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[(activefield-3)].qtc);
				activefield++;
				wattrset(qtcwin, line_currinverted);
				mvwprintw(qtcwin, activefield, 4, "%s", qtclist.qtclines[(activefield-3)].qtc);
			    }
			    if (*qtccount > 0 && qtclist.totalsent == *qtccount) {
				wattrset(qtcwin, line_inverted);
				mvwprintw(qtcwin, 2, 11, "CTRL+S to SAVE!");
				refreshp();
				show_help_msg(6);
				// //if (log_send_qtc_to_disk(nr_qsos) == 0) {
				    // //qtcreclist.sentcfmall = 1;
				    // TODO
				    // send 'CFM all' to station
				    // // if (trxmode == DIGIMODE || trxmode == CWMODE) {
					// // sendmessage(qtc_send_msgs[9]);
				    // //}
				    // TODO
				// //}
				// // x = 27;	// close the window
			    }
			}
		    }
		    if (activefield == 2) {
			if (direction == RECV &&
			    strlen(qtcreclist.callsign) > 0 &&
			    qtcreclist.serial > 0 &&
			    qtcreclist.count > 0 &&
			    qtcreclist.confirmed == 0
			) {
			    if (trxmode == CWMODE) {
				sendmessage(qtc_recv_msgs[1]);
			    }
			    if (trxmode == SSBMODE) {
				play_file(qtc_phrecv_message[1]);
			    }
			    if (qtcrec_record == 1 && record_run < 0) {
				strcpy(reccommand, qtcrec_record_command[0]);
				get_time();
				strftime(tempc, 60, "%y%m%d%H%M%S.wav", time_ptr);
				strcat(reccommand, tempc);
				strcat(reccommand, qtcrec_record_command[1]);
				record_run = system(reccommand);
			    }
			    activefield++;
			    showfield(activefield);
			}
		    }
		    if (trxmode == DIGIMODE) {
			if (direction == SEND && (activefield == 0 || activefield == 2) && qtclist.totalsent == 0) {
			    if (qtc_send_msgs[1][strlen(qtc_send_msgs[1])-1] == 10) {
				qtc_send_msgs[1][strlen(qtc_send_msgs[1])-1] = '\0';
			    }
			    tlen = strlen(qtc_send_msgs[1])-5; // len("sr/nr") = 5
			    char tmess[300], timec[40];
			    int ql;
			    tmess[0] = '\0';
			    if (tlen > 0 && strncmp(qtc_send_msgs[1] + tlen, "sr/nr", 5) == 0) {
				tempc[0] = '\0';
				strncpy(tempc, qtc_send_msgs[1], tlen-1);
				tempc[tlen-1] = '\0';
				sprintf(tmess, "%s %d/%d %s %d/%d\n", tempc, qtclist.serial, *qtccount, tempc, qtclist.serial, *qtccount);
			    }
			    timec[0] = '\0';
			    get_time();
			    strftime(timec, 40, "%d-%b-%y %H:%M", time_ptr);

			    for(ql = 0; ql < *qtccount; ql++) {
				qtclist.qtclines[ql].sent = 1;
				strncpy(qtclist.qtclines[ql].senttime, timec, 15);
				qtclist.qtclines[ql].senttime[15] = '\0';
				qtclist.totalsent++;

				tempc[0] = '\0';
				replace_spaces(qtclist.qtclines[ql].qtc, tempc);
				strcat(tmess, tempc);
				mvwprintw(qtcwin, ql+3, 30, "*");
				qtclist.qtclines[ql].flag = 1;

			    }
			    data_ready = 1;
			    strncpy(wkeyerbuffer, tmess, strlen(tmess));
			    write_keyer();
			    wattrset(qtcwin, line_inverted);
			    mvwprintw(qtcwin, 2, 11, "CTRL+S to SAVE!");
			    refreshp();
			    show_help_msg(6);

			    showfield(activefield);
			}
			else {
			  if (direction == SEND) {
				tempc[0] = '\0';
				replace_spaces(qtclist.qtclines[activefield-3].qtc, tempc);
				sendmessage(tempc);
			    }
			}
		    }
		    break;
	  case 19:	// CTRL-S - save QTC
		    if (*qtccurrdiretion == SEND && *qtccount > 0 && qtclist.totalsent == *qtccount) {
			if (log_sent_qtc_to_disk(nr_qsos) == 0) {
			    wattrset(qtcwin, line_inverted);
			    mvwprintw(qtcwin, 2, 11, "QTC's have been saved!");
			    prevqtccall[0] = '\0';
			    qtccallsign[0] = '\0';
			    refreshp();
			    sleep(1);
			    x = 27;	// close the window
			}
			//data_ready = 1;
		    }
		    if (*qtccurrdiretion == RECV && trxmode == DIGIMODE) {
			qtc_ry_capture = 1;
			wattr_get(qtcwin, &attributes, &cpair, NULL);
			wattrset(qtcwin, line_inverted);
			mvwprintw(qtcwin, 2, 11, "CAPTURE ON ");
			wattrset(qtcwin, attributes);
			showfield(activefield);
			refreshp();
			show_rtty_lines();
		    }
		    break;
	  case 5:	// CTRL-e  -  end capture
		    if (*qtccurrdiretion == RECV && trxmode == DIGIMODE) {
			qtc_ry_capture = 0;
			wattr_get(qtcwin, &attributes, &cpair, NULL);
			wattrset(qtcwin, line_inverted);
			mvwprintw(qtcwin, 2, 11, "CAPTURE OFF");
			wattrset(qtcwin, attributes);
			showfield(activefield);
			refreshp();
		    }
		    break;
	  case 129 ... 138:			/* F1.. F10 */
		    if (trxmode == CWMODE || trxmode == DIGIMODE) {
			if(direction == RECV) {
			    sendmessage(qtc_recv_msgs[x - 129]);
			    if (trxmode == CWMODE && qtcrec_record == 1 && strncmp(qtc_recv_msgs[x - 129], "QRV", 3) == 0 && record_run < 0) {
				strcpy(reccommand, qtcrec_record_command[0]);
				get_time();
				strftime(tempc, 60, "%y%m%d%H%M%S.wav", time_ptr);
				strcat(reccommand, tempc);
				strcat(reccommand, qtcrec_record_command[1]);
				record_run = system(reccommand);
			    }
			    if (trxmode == CWMODE && qtcrec_record == 1 && strncmp(qtc_recv_msgs[x - 129], "QSL ALL", 7) == 0 && record_run > -1) {
				strcpy(reccommand, "pkill -SIGINT -n ");
				strcat(reccommand, qtcrec_record_command_shutdown);
				system(reccommand);
				record_run = -1;
			    }
			    if (trxmode == DIGIMODE && strncmp(qtc_recv_msgs[x - 129], "QRV", 3) == 0) {
				qtc_ry_capture = 1;
				wattr_get(qtcwin, &attributes, &cpair, NULL);
				wattrset(qtcwin, line_inverted);
				mvwprintw(qtcwin, 2, 11, "CAPTURE ON ");
				wattrset(qtcwin, attributes);
				showfield(activefield);
				refreshp();
				show_rtty_lines();
			    }
			}
			if(direction == SEND && strlen(qtc_send_msgs[x - 129]) > 0) {
			    if (qtc_send_msgs[x - 129][strlen(qtc_send_msgs[x - 129])-1] == '\n') {
				qtc_send_msgs[x - 129][strlen(qtc_send_msgs[x - 129])-1] = 0;
			    }
			    tlen = strlen(qtc_send_msgs[x - 129])-5; // len("sr/nr") = 5
			    char tmess[40];
			    tmess[0] = '\0';
			    if (tlen > 0 && strncmp(qtc_send_msgs[x - 129] + tlen, "sr/nr", 5) == 0) {
				tempc[0] = '\0';
				strncpy(tempc, qtc_send_msgs[x - 129], tlen-1);
				tempc[tlen-1] = '\0';
				sprintf(tmess, "%s %d/%d ", tempc, qtclist.serial, *qtccount);
				sendmessage(tmess);
			    }
			    else if ((activefield-3) >= 0) {
				if (x-129 == 4) {	// F5, TIME
				    strncpy(tmess, qtclist.qtclines[activefield-3].qtc, 5);
				    tmess[5] = '\0';
				    sendmessage(tmess);
				}
				if (x-129 == 5) {	// F6, CALLSIGN
				    strncpy(tmess, qtclist.qtclines[activefield-3].qtc+5, 13);
				    for(i=12; tmess[i] == ' '; i--);
				    tmess[i+1] = ' ';
				    tmess[i+2] = '\0';
				    sendmessage(tmess);
				}
				if (x-129 == 6) {	// F7, SERIAL
				    for(i=strlen(qtclist.qtclines[activefield-3].qtc); qtclist.qtclines[activefield-3].qtc[i] != ' '; i--);
				    i++;
				    strncpy(tmess, qtclist.qtclines[activefield-3].qtc+i, strlen(qtclist.qtclines[activefield-3].qtc)-i);
				    i = strlen(qtclist.qtclines[activefield-3].qtc)-i;
				    tmess[i] = ' ';
				    tmess[i+1] = '\0';
				    sendmessage(tmess);
				}
			    }
			    else {
				sendmessage(qtc_send_msgs[x - 129]);
			    }
			}

		    }
		    if (trxmode == SSBMODE) {
			if (direction == RECV) {
			    if (qtcrec_record == 1 && x == 130 && record_run < 0) { // 130 -> F2, "QRV"
				strcpy(reccommand, qtcrec_record_command[0]);
				get_time();
				strftime(tempc, 60, "%y%m%d%H%M%S.wav", time_ptr);
				strcat(reccommand, tempc);
				strcat(reccommand, qtcrec_record_command[1]);
				record_run = system(reccommand);
			    }
			    if (qtcrec_record == 1 && x == 138 && record_run > -1) { // 138 -> F10, "QSL ALL"
				strcpy(reccommand, "pkill -SIGINT -n ");
				strcat(reccommand, qtcrec_record_command_shutdown);
				system(reccommand);
				record_run = -1;
			    }
			    play_file(qtc_phrecv_message[x - 129]);
			}
			if (direction == SEND) {
			    play_file(qtc_phsend_message[x - 129]);
			}
		    }

		    break;
	/*case 130:		// F2
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
		    showfield(activefield);
		    break;*/
	  case 161:  		// DELETE
		    if (DIRCLAUSE) {
			delete_from_field(0);
		    }
		    if (direction == SEND && activefield > 2 && trxmode != DIGIMODE) {
			qtclist.qtclines[activefield-3].sent = 0;
			qtclist.qtclines[activefield-3].flag = 0;
			wattrset(qtcwin, line_currnormal);
			mvwprintw(qtcwin, activefield, 30, " ");
			qtclist.totalsent--;
		    }
		    break;
	  /*case 138:			// SHIFT + Fn
		    x = onechar();
		    if (x == 81) {	// shift + F2
			for(j=0; j<qtcreclist.count; j++) {
			    if (qtcreclist.qtclines[j].status == 2) {
				qtcreclist.confirmed--;
			    }
			    qtcreclist.qtclines[j].status = 0;
			    show_status(j);
			}
			showfield(activefield);
		    }
		    break;*/
	  case 9:		// TAB
		  if (direction == RECV) {
		      if (trxmode == DIGIMODE) {
			  if (activefield == 32) {
			      activefield = 0;
			      showfield(32);
			  }
			  else {
			      activefield++;
			      showfield(activefield-1);
			  }
		      }
		      else {
			  if ((activefield < 3) || ((activefield-3)%3 >= 0 && (activefield-3)%3 < 2)) {
			      activefield++;
			      showfield(activefield-1);
			  }
			  else {
			      if ((activefield-3)%3 == 2) {
				  activefield -= 2;
				  showfield(activefield+2);
			      }
			  }
		      }
		      showfield(activefield);
		  }
		    if (direction == SEND) {
			switch (activefield) {
			    case 0:	activefield = 2;
				    showfield(0);
				    showfield(2);
				    break;
			    case 2:	activefield = 0;
				    showfield(2);
				    showfield(0);
				    break;
			    default:
				    break;
			}
		    }
		    curpos = 0;
		    break;
	  case 90:		// SHIFT + TAB
		    if (direction == RECV) {
			if (trxmode == DIGIMODE) {
			    if (activefield == 0) {
				activefield = 32;
				showfield(0);
			    }
			    else {
				activefield--;
				showfield(activefield+1);
			    }
			}
			else {
			    if ((activefield > 0 && activefield < 3) || ((activefield-3)%3 > 0)) {
				activefield--;
				showfield(activefield+1);
			    }
			    else {
				if ((activefield-3)%3 == 0) {
				    activefield += 2;
				    showfield(activefield-2);
				}
			    }
			}
			showfield(activefield);
		    }
		    if (direction == SEND) {
			switch (activefield) {
			    case 0:	activefield = 2;
				    showfield(0);
				    showfield(2);
				    break;
			    case 2:	activefield = 0;
				    showfield(2);
				    showfield(0);
				    break;
			    default:
				    break;
			}
		    }
		    curpos = 0;
		    break;
	  case 32:	// space
		    if (DIRCLAUSE) {
		        if (x == ' ' && direction == RECV && activefield > 2) {	// space at RECV mode
			      if (activefield%3 == 2) {
				  activefield -= 2;
				  showfield(activefield+2);
			      }
			      else {
				  activefield++;
				  showfield(activefield-1);
			      }
			      showfield(activefield);
			}
			else {
			    modify_field(x);
			}
		    }
		    break;
	  case 48 ... 57:	// numbers
		    /* if (trxmode == DIGIMODE && direction == RECV && activefield > 0 && activefield < 3 && *qtccount == 0 && capturing == 0) {
			capturing = 1;
			wattrset(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
			mvwprintw(qtcwin, 1, 19, "CAPTURE ON ");
		    } */
		    if (DIRCLAUSE) {
			modify_field(x);
		    }
		    break;
	  /*case 65 ... 90:	// letters
		    modify_field(x);
		    break;*/
	  case 97 ... 122:	// letters
		    if (DIRCLAUSE) {
			modify_field(x-32);
		    }
		    break;
	  case 47:	// '/' sign
		    if (DIRCLAUSE) {
			modify_field(x);
		    }
		    break;
	  case 63:	// '?' sign
		    if (DIRCLAUSE) {
			modify_field(x);
		    }
		    break;
	  case 127:	// backspace
		    if (DIRCLAUSE) {
		      delete_from_field(1);
		    }
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
	case 44:		// , keyer
	case 11:		// ctrl-k
	    {
		mvprintw(5, 0, "");
		keyer();
		x = 0;
		break;
	    }
	}
	refreshp();
	if (x != 27) {
	    x = 0;
	}
    }
    hide_panel(qtc_panel);
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
	    sprintf(fieldval, "%s", qtccallsign);
	    winrow = 1;
	    posidx = 0;
	    qtc_temp_obj = qtc_get(qtccallsign);
	    put_qtc();
	}
	else if (fidx == 1) {
	    sprintf(fieldval, "%4d", qtcreclist.serial);
	    winrow = 2;
	    posidx = 1;
	}
	else if (fidx == 2) {
	    sprintf(fieldval, "%d", *qtccount);
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
	if (fidx == activefield) {
	    wattrset(qtcwin, line_currinverted);
	}
	else {
	    wattrset(qtcwin, line_normal);
	}

	mvwprintw(qtcwin, winrow, pos[posidx][0], filled);
	mvwprintw(qtcwin, winrow, pos[posidx][0], "%s", fieldval);
	if (fidx == activefield) {
	    show_help_msg(posidx);
	    if (pos[posidx][0] == pos[posidx][1]) {
		wmove(qtcwin, winrow, (pos[posidx][1] + strlen(fieldval))-curpos);
	    }
	    else {
		wmove(qtcwin, winrow, pos[posidx][1]-curpos);
	    }
	}

	return 0;
}

int modify_field(int pressed) {
	char fieldval[16];
	int fi, winrow, qtcrow, posidx, stridx;

	posidx = 0;
	if (activefield == 0 && (isalnum(pressed) || pressed == '/') && strlen(qtccallsign) < pos[0][2]-1) {
	    shift_right(qtccallsign);
	    qtccallsign[strlen(qtccallsign)-curpos] = pressed;
	    qtccallsign[strlen(qtccallsign)+1] = '\0';
	    if (curpos > 0) {
		curpos--;
	    }
	    /*if (*qtccurrdiretion == SEND) {
		if (strlen(qtccallsign) > 0 && strcmp(qtccallsign, prevqtccall) != 0) {
		    qtc_temp_obj = qtc_get(qtccallsign);
		    *qtccount = genqtclist(qtccallsign, (10-(qtc_temp_obj->total)));
		    show_sendto_lines();
		    showfield(2);
		    put_qtc();
		}
	    }
	    strncpy(prevqtccall, qtccallsign, strlen(qtccallsign));*/
	    recalc_qtclist();
	    showfield(0);
	}
	else if (activefield == 1 && isdigit(pressed)) {
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
	else if (activefield == 2 && isdigit(pressed)) {
	    sprintf(fieldval, "%d", (*qtccount)*10);
	    shift_right(fieldval);
	    fieldval[strlen(fieldval)-(1+curpos)] = pressed;
	    fieldval[strlen(fieldval)] = '\0';
	    if (curpos > 0) {
		curpos--;
	    }
	    if(strlen(fieldval) <= pos[2][2] && atoi(fieldval) <= 10) {
	      qtc_temp_obj = qtc_get(qtccallsign);
	      if (*qtccurrdiretion == SEND) {
		    if (*qtccount != atoi(fieldval)) {
			if ((atoi(fieldval) + (qtc_temp_obj->total)) >= 10) {
			    sprintf(fieldval, "%d", (10 - (qtc_temp_obj->total)));
			}
			*qtccount = genqtclist(qtccallsign, atoi(fieldval));
			show_sendto_lines();
			put_qtc();
		    }
	      }
	      if (*qtccurrdiretion == RECV) {
		  if ((atoi(fieldval) + (qtc_temp_obj->total)) >= 10) {
		      sprintf(fieldval, "%d", (10 - (qtc_temp_obj->total)));
		  }
		  *qtccount = atoi(fieldval);
	      }
	      number_fields();
	      showfield(2);
	    }
	}
	else if (activefield >= 3 && activefield <= 32) {
	    fi = activefield-3;
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
		showfield(activefield);
		if (stridx == 0 && strlen(fieldval) == pos[posidx][2] && activefield <= 32) {	// auto TAB if curr field is QTC time, and len is 4
		    curpos = 0;
		    activefield++;
		    showfield(activefield-1);
		    showfield(activefield);
		}
	    }

	}
	return 0;
}

int delete_from_field(int dir) {

	char fieldval[16];
	int fi, winrow, qtcrow, stridx;

	if (activefield == 0) {
	    if (strlen(qtccallsign) > 0) {
		sprintf(fieldval, "%s", qtccallsign);
		shift_left(fieldval, dir);
		strncpy(qtccallsign, fieldval, strlen(fieldval));
		qtccallsign[strlen(fieldval)] = '\0';
		recalc_qtclist();
		showfield(0);
	    }
	}
	else if (activefield == 1) {
	    sprintf(fieldval, "%d", qtcreclist.serial);
	    if(strlen(fieldval) > 0) {
	      //sprintf(fieldval, "%d", qtcreclist.serial/10);
	      shift_left(fieldval, dir);
	      qtcreclist.serial = atoi(fieldval);
	      showfield(1);
	    }
	}
	else if (activefield == 2) {
	    sprintf(fieldval, "%d", *qtccount);
	    if(strlen(fieldval) > 0) {
	      //sprintf(fieldval, "%d", qtcreclist.count/10);
	      shift_left(fieldval, dir);
	      *qtccount = atoi(fieldval);
	      showfield(2);
	    }
	}
	else {
	    fi = activefield-3;
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
		showfield(activefield);
	    }

	}
	return 0;
}

int shift_right(char * fieldval) {
	int i;

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

	status = 0;
	if (idx < *qtccount) {
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
	    if (trxmode == DIGIMODE && qtcreclist.qtclines[idx].status == 0) {
		flag = '+';
	    }
	    wattrset(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
	    mvwprintw(qtcwin, idx+3, 30, "%c", flag);
	}
	return 0;
}

int number_fields() {
    int i;

    init_pair(QTCRECVBG,      COLOR_BLUE,   COLOR_CYAN);
    wattrset(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
    for(i=0;i<10;i++) {
	mvwprintw(qtcwin, i+3, 1, "  ");
    }
    for(i=0;i<*qtccount;i++) {
	mvwprintw(qtcwin, i+3, 1, "%2d", i+1);
    }
    return 0;
}

int clear_help_block() {
    int i;


    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;

    wattrset(qtcwin, line_inverted);
    for(i=1;i<13;i++) {
	mvwprintw(qtcwin, i, 36, "                                      ");
    }
    return 0;
}

int show_help_msg(msgidx) {
    clear_help_block();
    int i = 0, j = 0;
    char buff[80];
    int currqtc;

    //init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCRECVCURRLINE,COLOR_YELLOW, COLOR_MAGENTA);
    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);

    int line_currinverted = COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD;
    //int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;
    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;

    wattrset(qtcwin, line_currinverted);
    if (*qtccurrdiretion == RECV) {
        if (trxmode == DIGIMODE && *qtccurrdiretion == RECV) {
	    currqtc = ((activefield-3)/3);
	    if (qtcreclist.qtclines[currqtc].status == 0 &&
		strlen(qtcreclist.qtclines[currqtc].time) == 4 &&
		strlen(qtcreclist.qtclines[currqtc].callsign) > 0 &&
		strlen(qtcreclist.qtclines[currqtc].serial) > 0
	    ) {
		mvwprintw(qtcwin, ++j, 36, "Press ENTER to mark as RCVD");
	    }
	}
	else {
	    mvwprintw(qtcwin, ++j, 36, help_rec_msgs[msgidx]);
	}
    }
    if (*qtccurrdiretion == SEND) {
        if (msgidx > 2 && msgidx < 6) {
	    msgidx = 3;
	}
	mvwprintw(qtcwin, ++j, 36, help_send_msgs[msgidx]);
    }
    wattrset(qtcwin, line_inverted);
    mvwprintw(qtcwin, ++j, 36, "PgUP/PgDW: QRQ/QRS");
    if (*qtccurrdiretion == RECV) {
	mvwprintw(qtcwin, ++j, 36, "ENTER: R & next OR AGN");
    }
    if (*qtccurrdiretion == SEND) {
	mvwprintw(qtcwin, ++j, 36, "ENTER: send QTC");
    }
    for(i=0; i<12 && j < 12; i++) {
	if (*qtccurrdiretion == RECV) {
	    if (strlen(qtc_recv_msgs[i]) > 0) {
		strncpy(buff, qtc_recv_msgs[i], strlen(qtc_recv_msgs[i])-1);
		buff[strlen(qtc_recv_msgs[i])-1] = '\0';
		mvwprintw(qtcwin, ++j, 36, "F%-2d: %s", (i+1), buff);
	    }
	}
	if (*qtccurrdiretion == SEND) {
	    if (strlen(qtc_send_msgs[i]) > 0) {
		strncpy(buff, qtc_send_msgs[i], strlen(qtc_send_msgs[i])-1);
		buff[strlen(qtc_send_msgs[i])-1] = '\0';
		mvwprintw(qtcwin, ++j, 36, "F%-2d: %s", (i+1), buff);
	    }
	}
    }
    if (trxmode == DIGIMODE && *qtccurrdiretion == RECV) {
        mvwprintw(qtcwin, ++j, 36, "CTRL-T: Terminal window");
	mvwprintw(qtcwin, ++j, 36, "CTRL-S: Start capture");
	mvwprintw(qtcwin, ++j, 36, "CTRL-E: End capture");
    }

    return 0;
}

int parse_ry_line(char * line) {
    int j, i = 0, t = -1, f;
    char lline[40], *token, *wtoken, ttoken[50], tokens[5][15] = {"", "", "", "", ""};	// 5 members, cause possible callsign will AB/CD1EF/P, which will use 3
    char *saveptr1, *saveptr2;
    char sep[6] = "";
    int tactivefield;
    static int nr_parsed_line = 0;
    int serial_nr = 0;
    int maxlength[] = {4, 14, 4};

    if (*qtccount == 0 || qtcreclist.serial == 0) {
	nr_parsed_line = 0;
    }
    if ((*qtccount > 0 || qtcreclist.serial > 0) && nr_parsed_line == 0) {
	nr_parsed_line = 1;
    }

    strncpy(lline, line, strlen(line));
    tactivefield = activefield;
    if (nr_parsed_line == 0) {		// 1st line: SERIAL/NR
	wtoken = strtok_r(lline, " ", &saveptr1);
	while(wtoken != NULL && nr_parsed_line == 0) {
	    serial_nr = 0;
	    strcpy(ttoken, wtoken);
	    sep[0] = '/';
	    sep[1] = '\0';
	    token = strtok_r(ttoken, sep, &saveptr2);
	    t = -1;
	    while(token != NULL && t < 2)
	    {
		t++;
		strcpy(tokens[t], token);
		token = strtok_r(NULL, sep, &saveptr2);
	    }
	    f = 0;
	    for(i=0; i<=t; i++) {
		f = 1;
		for(j=0; j<strlen(tokens[i]); j++) {
		    if (! isdigit(tokens[i][j])) {
			f = 0;
		    }
		}
		if (f == 1) {
		    tactivefield = activefield;
		    activefield = i+1;
		    showfield(tactivefield);
		    for(j=0;j<strlen(tokens[i]); j++) {
			modify_field(tokens[i][j]);
		    }
		    showfield(activefield);
		    serial_nr++;
		}
	    }
	    if (serial_nr == 2) {
		nr_parsed_line++;
	    }
	    wtoken = strtok_r(NULL, " ", &saveptr1);
	}
    }
    else {
	strcpy(sep, "/;-: ");
	token = strtok(lline, sep);
	while(token != NULL && t < 4)
	{
	    t++;
	    strcpy(tokens[t], token);
	    token = strtok(NULL, sep);
	}

	while (t > 2) {
	    strcat(tokens[1], "/");
	    strcat(tokens[1], tokens[2]);
	    strcpy(tokens[t-1], tokens[t]);
	    tokens[t][0] = '\0';
	    t--;
	}

	for(i=0; i<=t; i++) {
	    f = 1;
	    for(j=0; j<strlen(tokens[i]); j++) {
		if ((i == 0 || i == 2) && ! isdigit(tokens[i][j])) {
		    tokens[i][j] = '?';
		}
		if (i == 1 && (!isalnum(tokens[i][j]) && tokens[i][j] != '/')) {
		    tokens[i][j] = '?';
		}
	    }
	    if (strlen(tokens[i]) > maxlength[i]) {
		tokens[i][maxlength[i]] = '\0';
	    }
	    if (f == 1) {
		tactivefield = activefield;
		activefield = (nr_parsed_line*3)+i;
		showfield(tactivefield);
		for(j=0;j<strlen(tokens[i]); j++) {
		    modify_field(tokens[i][j]);
		}
		showfield(activefield);
	    }
	}
	for(i=((t>0)?t:0); i<2; i++) {
	    tactivefield = activefield;
	    activefield = (nr_parsed_line*3)+i;
	    showfield(tactivefield);
	    modify_field('?');
	    showfield(activefield);
	}
	nr_parsed_line++;
	qtc_ry_copied++;
    }
    tactivefield = activefield;

    return tactivefield;
}

/* print one line with its status */
int print_rtty_line(t_qtc_ry_line qtc_ry_line, int row) {
    char currline[50];

    sprintf(currline, "%-38s", qtc_ry_line.content);
    currline[38] = '\0';
    if (qtc_ry_line.attr == 1) {
	currline[36] = ' ';
	currline[37] = '*';
    }
    mvwprintw(ry_win, row, 1, "%-38s", currline);
    return 0;
}

/* RTTY terminal to helps to capture the RTTY content */
int show_rtty_lines() {
    extern int miniterm;

    int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;
    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
    int line_currnormal = COLOR_PAIR(QTCRECVCURRLINE) | A_NORMAL;
    char boxhead[38];
    int prevline;
    char currline[50] = "", firstline[50] = "";
    int x, j;
    static int actline = 1;
    int oactivefield, tactivefield;

    oactivefield = activefield;
    tactivefield = activefield;

    show_panel(ry_panel);
    top_panel(ry_panel);
    werase(ry_win);
    show_panel(ry_help_panel);
    top_panel(ry_help_panel);
    werase(ry_help_win);

    sprintf(boxhead, "HELP");
    wnicebox(ry_help_win, 0, 0, 3, 73, "HELP");
    wbkgd(ry_help_win, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVWINBG)));
    wattrset(ry_help_win, line_inverted);
    //                           "1234567890123456789012345678901234567890123456789012345678901234567890123"
    mvwprintw(ry_help_win, 1, 1, "CTRL-R: Reset lines  -  CTRL-S: Start capture  -  CTRL-E: End capture    ");
    mvwprintw(ry_help_win, 2, 1, "ENTER: add current line to QTC  -  Up/Down: move                         ");
    mvwprintw(ry_help_win, 3, 1, "ESC: hide RTTY window                                                    ");

    strcpy(boxhead, "RTTY");
    wnicebox(ry_win, 0, 0, 12, 38, boxhead);
    wattrset(ry_win, line_normal);
    for(j=1; j<13; j++) {
	mvwprintw(ry_win, j, 1, "                                      ");
    }
    refreshp();

    if (qtc_ry_capture == 1) {
	mvwprintw(qtcwin, 2, 11, "CAPTURE ON ");
    }
    else {
	mvwprintw(qtcwin, 2, 11, "CAPTURE OFF");
    }

    x = -1; j = 1;
    prevline = -1;
    curs_set(0);
    while(x != 27) {

	while (x < 1) {

	    usleep(1000);
	    time_update();
	    if (miniterm == 1 && trxmode == DIGIMODE && (keyerport == GMFSK
	           || keyerport == MFJ1278_KEYER)) {
		    show_rtty();
	    }
	    if ((qtc_ry_currline < 11 && prevline != qtc_ry_currline-1) || qtc_ry_currline == 11) {
		if (actline > 1 && strcmp(firstline, qtc_ry_lines[0].content) != 0) {
		  actline--;
		  strcpy(firstline, qtc_ry_lines[0].content);
		}
		if (actline == 1 && qtc_ry_currline > 0) {
		    strcpy(firstline, qtc_ry_lines[0].content);
		}
		for(j=0; j<qtc_ry_currline; j++) {
		    if (j+1 == actline) {
			wattrset(ry_win, line_currnormal);
		    }
		    print_rtty_line(qtc_ry_lines[j], j+1);
		    wattrset(ry_win, line_normal);
		}
		prevline = qtc_ry_currline-1;
	    }
	    if (miniterm == 1 || miniterm == 0) {
		strncpy(currline, qtc_ry_lines[qtc_ry_currline].content, 38);
		currline[39] = '\0';
		mvwprintw(ry_win, qtc_ry_currline+1, 1, "%-38s", currline);
	    }
	    refreshp();
	    x = key_poll();
	}

	switch(x) {
	    case 152:	// cursor up
		      wattrset(ry_win, line_normal);
		      print_rtty_line(qtc_ry_lines[actline-1], actline);
		      if (actline > 1) {
			  actline--;
		      }
		      else {
			  if (qtc_ry_currline > 0) {
			      actline = qtc_ry_currline;
			  }
		      }
		      wattrset(ry_win, line_currnormal);
		      print_rtty_line(qtc_ry_lines[actline-1], actline);
		      wattrset(ry_win, line_normal);
		      break;
	    case 153:	// cursor down
		      wattrset(ry_win, line_normal);
		      print_rtty_line(qtc_ry_lines[actline-1], actline);
		      if (actline < qtc_ry_currline) {
			  actline++;
		      }
		      else {
			  actline = 1;
		      }
		      wattrset(ry_win, line_currnormal);
		      print_rtty_line(qtc_ry_lines[actline-1], actline);
		      wattrset(ry_win, line_normal);
		      break;
	    case 18:	// CTRL-r
		      for(j=0; j<12; j++) {
			  qtc_ry_lines[j].content[0] = '\0';
			  qtc_ry_lines[j].attr = 0;
			  mvwprintw(ry_win, (j+1), 1, "                                      ");
		      }
		      prevline = -1;
		      actline = 1;
		      qtc_ry_currline = 0;
		      qtc_ry_copied = 0;
		      break;
	    case 19:	// CTRL-s  -  start capture
		      qtc_ry_capture = 1;
		      mvwprintw(qtcwin, 2, 11, "CAPTURE ON ");
		      break;
	    case 5:	// CTRL-e  -  end capture
		      qtc_ry_capture = 0;
		      mvwprintw(qtcwin, 2, 11, "CAPTURE OFF");
		      break;
	    case 10:	// ENTER  -  add to qtc
		      if (qtc_ry_lines[actline-1].attr == 0) {
			  parse_ry_line(qtc_ry_lines[actline-1].content);
			  qtc_ry_lines[actline-1].attr = 1;
		      }
		      wattrset(ry_win, line_currnormal);
		      print_rtty_line(qtc_ry_lines[actline-1], actline);
		      wattrset(ry_win, line_normal);
		      break;
	}
	refreshp();
	if (x != 27) {
	    x = -1;
	}
	if (*qtccount > 0 && qtc_ry_copied == *qtccount) {
	    qtc_ry_capture = 0;
	    mvwprintw(qtcwin, 2, 11, "CAPTURE OFF");
	}
    }
    hide_panel(ry_panel);
    hide_panel(ry_help_panel);
    //mvwprintw(qtcwin, 2, 11, "           ");

    curs_set(1);
    tactivefield = activefield;
    activefield = oactivefield;
    showfield(tactivefield);
    showfield(oactivefield);

    return 0;
}

int put_qtc() {

    init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;
    char qtcdirstring[3][10] = {"", "Received", "Sent"};

    wattrset(qtcwin, line_normal);
    if (*qtccurrdiretion == RECV || *qtccurrdiretion == SEND) {
	mvwprintw(qtcwin, 1, 19, "%s %2d QTC", qtcdirstring[*qtccurrdiretion], qtc_temp_obj->total);
    }
    else {
	mvwprintw(qtcwin, 1, 19, "Total: %2d QTC (R: %d, S: %d)", qtc_temp_obj->total, qtc_temp_obj->received, qtc_temp_obj->sent);
    }
    return 0;
}

int replace_spaces(char * src, char * tempc) {
      int tsp, tdp;

      tsp = 0;
      tdp = 0;
      while(src[tsp] != '\0') {
	  if (src[tsp] != ' ') {
	      tempc[tdp] = src[tsp];
	      tdp++;
	  }
	  else {
	      if (trxmode == DIGIMODE) {
		  tempc[tdp] = '-';
	      }
	      else {
		  tempc[tdp] = ' ';
	      }
	      tdp++;
	      while(src[tsp+1] == ' ') {
		  tsp++;
	      }
	  }
	  tsp++;
      }
      tempc[tdp] = '\n';
      tdp++;
      tempc[tdp] = '\0';

      return 0;
}

int show_sendto_lines() {
    int i;
    int line_inverted = COLOR_PAIR(QTCRECVINVLINE) | A_BOLD;
    int line_normal = COLOR_PAIR(QTCRECVLINE) | A_NORMAL;

    wattrset(qtcwin, line_inverted);
    for(i=0; i<10; i++) {
	mvwprintw(qtcwin, i+3, 1, "                                 ");
    }
    wattrset(qtcwin, line_normal);
    for(i=0; i<qtclist.count; i++) {
	mvwprintw(qtcwin, i+3, 4, "%s", qtclist.qtclines[i].qtc);
	if (qtclist.qtclines[i].sent == 1) {
	    mvwprintw(qtcwin, i+3, 30, "*");
	}
    }
    wattrset(qtcwin, line_normal);
    for(i=qtclist.count; i<10; i++) {
	mvwprintw(qtcwin, i+3, 4, "                        ");
    }
    number_fields();
    return 0;
}

int recalc_qtclist() {
	if (*qtccurrdiretion == SEND) {
	    if (strlen(qtccallsign) > 0 && strcmp(qtccallsign, prevqtccall) != 0) {
		qtc_temp_obj = qtc_get(qtccallsign);
		*qtccount = genqtclist(qtccallsign, (10-(qtc_temp_obj->total)));
		show_sendto_lines();
		showfield(2);
		put_qtc();
	    }
	}
	strncpy(prevqtccall, qtccallsign, strlen(qtccallsign));

	return 0;
}

/* int move_cursor(int dir) {
	int fi, winrow;

	if (activefield == 0 || activefield == 1) {
	    winrow = 1;
	}
	else {
	    fi = activefield-2;
	    winrow = (fi/3)+2;
	}
	wmove(qtcwin, winrow, pos[posidx][1]);
} */

