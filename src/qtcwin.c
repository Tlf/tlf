/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013-2015 Ervin Hegedüs - HA2OS <airween@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------
 *        Handle QTC panel, receive and send QTC, write to log
 *
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "callinput.h"
#include "cw_utils.h"
#include "genqtclist.h"
#include "get_time.h"
#include "ignore_unused.h"
#include "keyer.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "nicebox.h"		// Includes curses.h
#include "qtc_log.h"
#include "qtcutil.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "rtty.h"
#include "sendbuf.h"
#include "speedupndown.h"
#include "time_update.h"
#include "tlf_panel.h"
#include "ui_utils.h"
#include "write_keyer.h"
/* check direction clause macro
 * direction should be RECV (1) or SEND (2), see tlf.h
 */
#define DIRCLAUSE (direction == RECV) || (direction == SEND && (activefield == 0 || activefield == 2))


void init_qtc_panel();
void draw_qtc_panel();
void start_qtc_recording();
void stop_qtc_recording();
void clear_help_block();
void show_help_msg();
void showfield(int fidx);
void modify_field(int pressed);
void delete_from_field(int dir);
void shift_left(char *fieldval, int shift);
void show_status(int idx);
void number_fields();
void put_qtc();
void replace_spaces(char *src, char *tempc);
void show_sendto_lines();
void recalc_qtclist();
void show_rtty_lines();
void fill_qtc_times(char *time);

extern char hiscall[];
extern char lastcall[];
extern int trxmode;
extern int digikeyer;
extern int nr_qsos;

static int record_run = -1;		/* was recording already started? */

t_qtclist qtclist;
t_qtcreclist qtcreclist;

t_qtc_ry_line qtc_ry_lines[QTC_RY_LINE_NR];
int qtc_ry_currline = 0;
int qtc_ry_capture;
int qtc_ry_copied;

struct t_qtc_store_obj *qtc_temp_obj;  /* temp object to help to maintain
					  the QTC hash */


enum {
    QTCRECVWINBG = 32,
    QTCRECVLINE,
    QTCRECVINVLINE,
    QTCRECVCURRLINE,
    QTCRECVBG,
};

#define LINE_INVERTED  		(COLOR_PAIR(QTCRECVINVLINE) | A_BOLD)
#define LINE_CURRINVERTED 	(COLOR_PAIR(QTCRECVCURRLINE) | A_BOLD)
#define LINE_CURRNORMAL  	(COLOR_PAIR(QTCRECVCURRLINE) | A_NORMAL)
#define LINE_NORMAL  		(COLOR_PAIR(QTCRECVLINE) | A_NORMAL)

WINDOW *qtcwin;
PANEL *qtc_panel;
WINDOW *ry_win;
PANEL *ry_panel;
WINDOW *ry_help_win;
PANEL *ry_help_panel;


int activefield;
// array values: hor position, cursor position, field len to fill with spaces
int pos[6][3] = {{3, 3, 15}, {3, 6, 4}, {8, 8, 2}, {3, 3, 4}, {8, 8, 14}, {24, 24, 4}};
int curpos = 0;
int curfieldlen = 0;
static char prevqtccall[15] = "";

char help_rec_msgs[7][26] = {
    "Enter callsign",
    "Enter the QTC serial",
    "Enter the QTC number",
    "Enter the time",
    "Enter the CALL",
    "Enter the SERIAL",
    ""
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

char *qtccallsign;
int *qtccount;
int qtccurrdirection;
char qtchead[32];


/* Init qtc panels and windows if needed, set used colorpairs */
void init_qtc_panel() {
    static int qtcpanel_initialized = 0;

    if (!qtcpanel_initialized) {
	qtcwin = newwin(14, 75, 9, 2);
	qtc_panel = new_panel(qtcwin);
	hide_panel(qtc_panel);

	ry_win = newwin(14, 40, 9, 37);
	ry_panel = new_panel(ry_win);
	hide_panel(ry_panel);

	ry_help_win = newwin(5, 75, 4, 2);
	ry_help_panel = new_panel(ry_help_win);
	hide_panel(ry_help_panel);

	qtcpanel_initialized = 1;
    }

    init_pair(QTCRECVWINBG,   COLOR_BLUE,   COLOR_GREEN);
    init_pair(QTCRECVLINE,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(QTCRECVINVLINE, COLOR_YELLOW, COLOR_CYAN);
    init_pair(QTCRECVCURRLINE, COLOR_YELLOW, COLOR_MAGENTA);
    init_pair(QTCRECVBG,      COLOR_BLUE,   COLOR_CYAN);
}

/* draw the panel content */
void draw_qtc_panel(int direction) {
    int i, j;

    show_panel(qtc_panel);
    top_panel(qtc_panel);

    werase(qtcwin);

    wnicebox(qtcwin, 0, 0, 12, 33, qtchead);

    sprintf(qtchead, "HELP");
    wnicebox(qtcwin, 0, 35, 12, 38, qtchead);

    wbkgd(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVWINBG)));

    wattrset(qtcwin, LINE_INVERTED);
    mvwaddstr(qtcwin, 1, 1, "                                 ");
    /* the first visible and used line is the qtc serial and count
     * it differs in RECV and SEND direction, these two lines sets them
     */
    if (direction == RECV) {
	mvwaddstr(qtcwin, 2, 1, "      /                          ");
    }
    if (direction == SEND) {
	mvwprintw(qtcwin, 2, 1, "     %d/%2d                        ",
		  qtclist.serial, qtclist.count);
    }

    showfield(0);	// QTC CALL
    if (direction == RECV) {
	showfield(1);	// QTC serial
    }
    showfield(2);	// QTC nr of row

    clear_help_block();

    /* if the direction is RECV, set the fields on the opened QTC window */
    if (direction == RECV) {
	for (i = 0; i < QTC_LINES; i++) {
	    wattrset(qtcwin, LINE_INVERTED);
	    mvwaddstr(qtcwin, i + 3, 1, "                                 ");
	    for (j = 0; j < 3; j++) {
		showfield((i * 3) + j + 3);	// QTC fields...
	    }
	}
	number_fields();
    }
    /* same function, but it's a littlebit more complex, therefore that's in an external function */
    if (direction == SEND) {
	show_sendto_lines();
    }

    /* if the direction is RECV, and mode is DIGIMODE,
     * show the current CAPTURE MODE
     */
    if (qtccurrdirection == RECV) {
	wattrset(qtcwin, LINE_INVERTED);

	if (trxmode == DIGIMODE) {
	    if (qtc_ry_capture == 0) {
		mvwaddstr(qtcwin, 2, 11, "CAPTURE OFF");
	    } else {
		mvwaddstr(qtcwin, 2, 11, "CAPTURE ON ");
		show_rtty_lines();
	    }
	} else {
	    if (qtcrec_record) {
		mvwaddstr(qtcwin, 2, 11, "RECORD OFF  ");
	    }
	}
	wattrset(qtcwin, LINE_NORMAL);
    }

    showfield(activefield);
    curpos = 0;

    refreshp();
}


/* start recording */
void start_qtc_recording() {
    char reccommand[100] = "";
    char tempc[40];

    if (record_run < 0 && qtcrec_record
	    && strlen((char *)qtcrec_record_command) > 0) {
	strcpy(reccommand, qtcrec_record_command[0]);
	format_time(tempc, sizeof(tempc), "%y%m%d%H%M%S.wav");
	strcat(reccommand, tempc);
	strcat(reccommand, qtcrec_record_command[1]);
	record_run = system(reccommand);
	if (record_run > -1 && qtcrec_record) {
	    mvwaddstr(qtcwin, 2, 11, "RECORD ON   ");
	}
    }
}

/* stop recording */
void stop_qtc_recording() {
    char reccommand[100] = "";

    if (qtcrec_record && strlen((char *)qtcrec_record_command_shutdown) > 0) {
	strcpy(reccommand, "pkill -SIGINT -n ");
	strcat(reccommand, qtcrec_record_command_shutdown);
	IGNORE(system(reccommand));;
	record_run = -1;
	if (qtcrec_record) {
	    mvwaddstr(qtcwin, 2, 11, "RECORD OFF  ");
	}
    }
}

void fill_qtc_callsign(int direction, char *tcall) {
    if (direction == RECV) {
	strcpy(qtcreclist.callsign, tcall);
    }
    if (direction == SEND) {
	strcpy(qtclist.callsign, tcall);
    }
}

void fill_qtc_times(char *time) {
    int i;
    int len;
    int afield;

    if (trxmode != DIGIMODE && qtccurrdirection == RECV) {
	afield = (activefield - 3) / 3;
	if (strlen(time) >= 2) {
	    len = 2;
	} else {
	    len = strlen(time);
	}
	for (i = afield + 1; i < *qtccount; i++) {
	    strncpy(qtcreclist.qtclines[i].time, time, len);
	    qtcreclist.qtclines[i].time[len] = '\0';
	    showfield(3 + (i * 3));
	}
    }
}

void fill_lazy_line(t_qtcrecline *recline) {
    int i;

    // fill time field
    for (i = strlen(recline->time); i < 4; i++) {
	recline->time[i + 1] = '\0';
	recline->time[i] = '?';
    }

    // fill callsign field
    for (i = strlen(recline->callsign); i < 3; i++) {
	recline->callsign[i + 1] = '\0';
	recline->callsign[i] = '?';
    }

    // fill serial field
    for (i = strlen(recline->serial); i < 1; i++) {
	recline->serial[i + 1] = '\0';
	recline->serial[i] = '?';
    }

}

/* prepare data for RECV operation */
void prepare_for_recv() {
    int i;

    /* if the callsign field is empty, or the callsign field has
     * been changed in main window, then it needs to clean up
     * all received QTC info
     */
    if (strcmp(qtcreclist.callsign, prevqtccall) != 0 ||
	    strlen(qtcreclist.callsign) == 0) {
	qtcreclist.count = 0;
	qtcreclist.serial = 0;
	qtcreclist.confirmed = 0;
	qtcreclist.sentcfmall = 0;
	for (i = 0; i < QTC_LINES; i++) {
	    qtcreclist.qtclines[i].status = 0;
	    qtcreclist.qtclines[i].time[0] = '\0';
	    qtcreclist.qtclines[i].callsign[0] = '\0';
	    qtcreclist.qtclines[i].serial[0] = '\0';
	    qtcreclist.qtclines[i].receivedtime[0] = '\0';
	    qtcreclist.qtclines[i].confirmed = 0;
	}
	activefield = 0;
	qtc_ry_copied = 0;
	stop_qtc_recording();
    }

    if (qtcreclist.count == 0) {
	activefield = 0;
    }

    /* save the previous qtc callsign */
    g_strlcpy(prevqtccall, qtcreclist.callsign, sizeof(prevqtccall));
    qtccallsign = qtcreclist.callsign;

    /* save the address of the counter of receive QTC block */
    qtccount = &qtcreclist.count;

    /* set the QTC win header */
    sprintf(qtchead, "QTC receive");
}


/* prepare data for SEND operation */
void prepare_for_send() {

    /* if the callsign field in main window had been changed, it needs
     * to clean up the qtc structure */
    if (strcmp(qtclist.callsign, prevqtccall) != 0 ||
	    strlen(qtclist.callsign) == 0 || qtclist.count == 0) {

	qtc_temp_obj = qtc_get(qtclist.callsign);
	genqtclist(qtclist.callsign, (10 - (qtc_temp_obj->total)));

	activefield = 0;
    }

    /* save the current callsign to previous call variable */
    g_strlcpy(prevqtccall, qtclist.callsign, sizeof(prevqtccall));
    qtccallsign = qtclist.callsign;

    /* save the address of the counter of SEND qtc structure */
    qtccount = &qtclist.count;

    /* set the QTC win header */
    sprintf(qtchead, "QTC send");
}

void qtc_main_panel(int direction) {
    char tempc[40];
    int i, x;
    int tfi, tlen = 0;
    int currqtc = -1;
    attr_t attributes;
    short cpair;


    /* fill the callsign fields of the current qtc direction structure
     * with hiscall or the last call if hiscall is empty
     */
    if (strlen(hiscall) > 0) {
	fill_qtc_callsign(direction, hiscall);
    } else if (strlen(lastcall) > 0) {
	fill_qtc_callsign(direction, lastcall);
    }

    /* save the value of the direction variable */
    qtccurrdirection = direction;

    /* here are the mandatory steps, if the direction is RECV */
    if (direction == RECV) {
	prepare_for_recv();
    }

    /* here are the mandatory steps, if the direction is SEND */
    if (direction == SEND) {
	prepare_for_send();
    }

    init_qtc_panel();
    draw_qtc_panel(direction);

    i = 1;

    x = -1;
    /* main loop */
    while (x != ESCAPE) {

	while (x < 1) {

	    usleep(5000);
	    time_update();
	    if (trxmode == DIGIMODE && digikeyer != NO_KEYER) {
		show_rtty();
	    }
	    x = key_poll();

	}

	switch (x) {
	    // Ctrl-T (^T)
	    case CTRL_T:
		if (trxmode == DIGIMODE) {
		    show_rtty_lines();
		}

	    // Up Arrow
	    case KEY_UP:
		if (direction == RECV) {
		    if (activefield > 0) {	// nr of QTC record field idx
			if (activefield == 1 || activefield == 2) {
			    activefield = 0;
			    showfield(1);
			    showfield(2);
			} else if (activefield == 3 || activefield == 4) {
			    activefield -= 2;
			    showfield(activefield + 2);
			} else {
			    activefield -= 3;
			    showfield(activefield + 3);
			}
		    }
		    showfield(activefield);
		    curpos = 0;
		}
		if (direction == SEND && ((trxmode != DIGIMODE
					   && qtclist.totalsent < *qtccount) || (trxmode == DIGIMODE
						   && qtclist.totalsent == *qtccount))) {
		    switch (activefield) {
			case 0:
			    activefield = 12;
			    show_help_msg(activefield);
			    showfield(0);
			    wattrset(qtcwin, LINE_CURRINVERTED);
			    mvwaddstr(qtcwin, activefield, 4, qtclist.qtclines[9].qtc);
			    break;
			case 2:
			    activefield = 0;
			    showfield(2);
			    showfield(0);
			    break;
			case 3:
			    activefield = 2;
			    showfield(2);
			    wattrset(qtcwin, LINE_NORMAL);
			    mvwaddstr(qtcwin, activefield + 1, 4, qtclist.qtclines[0].qtc);
			    break;
			case 4 ... 12:
			    show_help_msg(activefield);
			    wattrset(qtcwin, LINE_NORMAL);
			    mvwprintw(qtcwin, activefield, 4, "%s",
				      qtclist.qtclines[(activefield - 3)].qtc);
			    activefield--;
			    wattrset(qtcwin, LINE_CURRINVERTED);
			    mvwprintw(qtcwin, activefield, 4, "%s",
				      qtclist.qtclines[(activefield - 3)].qtc);
			    break;
		    }
		}
		break;

	    // Down Arrow
	    case KEY_DOWN:
		if (direction == RECV) {
		    if (activefield < 30) {	// last line serial field idx
			if (activefield == 0) {
			    activefield = 1;
			    showfield(0);
			} else if (activefield == 1 || activefield == 2) {
			    activefield += 2;
			    showfield(1);
			    showfield(2);
			} else {
			    activefield += 3;
			    showfield(activefield - 3);
			}
		    }
		    showfield(activefield);
		    curpos = 0;
		}
		if (direction == SEND && ((trxmode != DIGIMODE
					   && qtclist.totalsent < *qtccount) || (trxmode == DIGIMODE
						   && qtclist.totalsent == *qtccount))) {
		    switch (activefield) {
			case 0:
			    activefield = 2;
			    showfield(0);
			    showfield(2);
			    break;
			case 2:
			    activefield = 3;
			    show_help_msg(activefield);
			    showfield(2);
			    wattrset(qtcwin, LINE_CURRINVERTED);
			    mvwaddstr(qtcwin, activefield, 4, qtclist.qtclines[0].qtc);
			    break;
			case 3 ... 11:
			    show_help_msg(activefield);
			    wattrset(qtcwin, LINE_NORMAL);
			    mvwprintw(qtcwin, activefield, 4, "%s",
				      qtclist.qtclines[(activefield - 3)].qtc);
			    activefield++;
			    wattrset(qtcwin, LINE_CURRINVERTED);
			    mvwprintw(qtcwin, activefield, 4, "%s",
				      qtclist.qtclines[(activefield - 3)].qtc);
			    break;
			case 12:
			    activefield = 0;
			    wattrset(qtcwin, LINE_NORMAL);
			    mvwaddstr(qtcwin, 14, 4, qtclist.qtclines[9].qtc);
			    showfield(0);
			    break;
		    }
		}
		break;

	    // Left Arrow
	    case KEY_LEFT:
		if (DIRCLAUSE) {
		    if (curpos < curfieldlen) {	// curpos is a shift, means lenght - position
			curpos++;
			showfield(activefield);
		    }
		}
		break;

	    // Right Arrow
	    case KEY_RIGHT:
		if (DIRCLAUSE) {
		    if (curpos > 0) {
			curpos--;
			showfield(activefield);
		    }
		}
		break;

	    // <Enter>
	    case '\n':
	    case KEY_ENTER:
		if (activefield > 2) {
		    if (direction == RECV) {
			currqtc = ((activefield - 3) / 3);
			if ((qtcreclist.qtclines[currqtc].status == 0 &&
				strlen(qtcreclist.qtclines[currqtc].time) == 4 &&
				strlen(qtcreclist.qtclines[currqtc].callsign) > 0 &&
				strlen(qtcreclist.qtclines[currqtc].serial) > 0)
				|| qtc_recv_lazy
			   ) {
			    if (qtc_recv_lazy) {
				fill_lazy_line(&qtcreclist.qtclines[currqtc]);
				showfield(3 * (currqtc + 1));
				showfield((3 * (currqtc + 1)) + 1);
				showfield((3 * (currqtc + 1)) + 2);
			    }

			    format_time(tempc, sizeof(tempc), DATE_TIME_FORMAT);
			    g_strlcpy(qtcreclist.qtclines[currqtc].receivedtime, tempc, 16);
			    qtcreclist.qtclines[currqtc].status = 2;
			    show_status(currqtc);
			    if (currqtc < *qtccount) {
				if (trxmode == DIGIMODE) {
				    qtcreclist.qtclines[currqtc].confirmed = 1; // compatibility for other modes
				    qtcreclist.confirmed++;
				    activefield += 3;	// go to next line exch field
				    showfield(activefield - 3);
				} else {
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
				    tfi = (activefield - 3) % 3;
				    //activefield++;	// go to next line time field
				    activefield += (3 - tfi);
				    showfield(activefield - (3 - tfi));
				}
				showfield(activefield);
			    }
			} else if (qtcreclist.qtclines[currqtc].status == 1 &&
				   qtcreclist.qtclines[currqtc].confirmed != 1) {
			    if (trxmode == CWMODE) {
				sendmessage(qtc_recv_msgs[7]);
			    }
			    if (trxmode == SSBMODE) {
				play_file(qtc_phrecv_message[7]);
			    }
			    if (trxmode == DIGIMODE) {
				char *str = g_strdup_printf("%s %02d %02d\n",
							    g_strchomp(qtc_recv_msgs[7]),
							    currqtc + 1, currqtc + 1);
				sendmessage(str);
				g_free(str);
			    }
			}

			if (*qtccount > 0 && qtcreclist.confirmed == *qtccount) {
			    if (qtcreclist.sentcfmall == 0) {
				qtcreclist.sentcfmall = 1;
				log_recv_qtc_to_disk(nr_qsos);
				if (record_run > -1) {
				    stop_qtc_recording();
				}
				if (trxmode == DIGIMODE || trxmode == CWMODE) {
				    sendmessage(qtc_recv_msgs[9]);
				}
				if (trxmode == SSBMODE) {
				    play_file(qtc_phrecv_message[9]);
				}
			    }
			    x = ESCAPE;	// <Escape> close the window
			}
		    }
		    if (direction == SEND && trxmode != DIGIMODE) {
			if (qtclist.qtclines[activefield - 3].sent == 0) {
			    qtclist.qtclines[activefield - 3].sent = 1;
			    format_time(tempc, sizeof(tempc), DATE_TIME_FORMAT);
			    g_strlcpy(qtclist.qtclines[activefield - 3].senttime, tempc, 16);
			    qtclist.qtclines[activefield - 3].senttime[15] = '\0';
			    qtclist.totalsent++;
			}
			tempc[0] = '\0';
			replace_spaces(qtclist.qtclines[activefield - 3].qtc, tempc);
			if (trxmode == CWMODE) {
			    sendmessage(tempc);
			}

			mvwaddstr(qtcwin, activefield, 30, "*");
			qtclist.qtclines[activefield - 3].flag = 1;
			// scroll down if not at end of qtclist:
			if (activefield - 3 < *qtccount - 1) {
			    wattrset(qtcwin, LINE_NORMAL);
			    mvwprintw(qtcwin, activefield, 4, "%s",
				      qtclist.qtclines[(activefield - 3)].qtc);
			    activefield++;
			    wattrset(qtcwin, LINE_CURRINVERTED);
			    mvwprintw(qtcwin, activefield, 4, "%s",
				      qtclist.qtclines[(activefield - 3)].qtc);
			}
			if (*qtccount > 0 && qtclist.totalsent == *qtccount) {
			    wattrset(qtcwin, LINE_INVERTED);
			    mvwaddstr(qtcwin, 2, 11, "CTRL+S to SAVE!");
			    refreshp();
			    show_help_msg(6);
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
			start_qtc_recording();
			activefield++;
			showfield(activefield);
		    }
		}
		if (trxmode == DIGIMODE) {
		    if (direction == SEND && (activefield == 0 || activefield == 2)
			    && qtclist.totalsent == 0) {
			tlen = strlen(qtc_send_msgs[1]) - 5; // len("sr/nr") = 5
			char tmess[300], timec[40];
			int ql;
			tmess[0] = '\0';
			if (tlen > 0 && strncmp(qtc_send_msgs[1] + tlen, "sr/nr", 5) == 0) {
			    g_strlcpy(tempc, qtc_send_msgs[1], tlen);
			    sprintf(tmess, "%s %d/%d %s %d/%d\n",
				    tempc, qtclist.serial, *qtccount, tempc,
				    qtclist.serial, *qtccount);
			}
			format_time(timec, sizeof(timec), DATE_TIME_FORMAT);

			for (ql = 0; ql < *qtccount; ql++) {
			    qtclist.qtclines[ql].sent = 1;
			    g_strlcpy(qtclist.qtclines[ql].senttime, timec, 16);
			    qtclist.totalsent++;

			    tempc[0] = '\0';
			    replace_spaces(qtclist.qtclines[ql].qtc, tempc);
			    strcat(tmess, tempc);
			    mvwaddstr(qtcwin, ql + 3, 30, "*");
			    qtclist.qtclines[ql].flag = 1;

			}
			keyer_append(tmess);
			write_keyer();
			wattrset(qtcwin, LINE_INVERTED);
			mvwaddstr(qtcwin, 2, 11, "CTRL+S to SAVE!");
			refreshp();
			show_help_msg(6);

			showfield(activefield);
		    } else {
			if (direction == SEND) {
			    tempc[0] = '\0';
			    replace_spaces(qtclist.qtclines[activefield - 3].qtc, tempc);
			    sendmessage(tempc);
			}
		    }
		}
		if (activefield == 0) {
		    start_qtc_recording();
		}
		if (activefield < 2) {
		    if (direction == RECV) {
			activefield++;
			showfield(activefield - 1);
			showfield(activefield);
		    }
		}
		break;

	    // Ctrl-S (^S), save QTC
	    case CTRL_S:
		if (qtccurrdirection == SEND && *qtccount > 0
			&& qtclist.totalsent == *qtccount) {
		    log_sent_qtc_to_disk(nr_qsos);
		    wattrset(qtcwin, LINE_INVERTED);
		    mvwaddstr(qtcwin, 2, 11, "QTCs have been saved!");
		    prevqtccall[0] = '\0';
		    qtccallsign[0] = '\0';
		    refreshp();
		    sleep(1);
		    x = ESCAPE;	// <Escape> close the window

		}
		if (qtccurrdirection == RECV && trxmode == DIGIMODE) {
		    qtc_ry_capture = 1;
		    wattr_get(qtcwin, &attributes, &cpair, NULL);
		    wattrset(qtcwin, LINE_INVERTED);
		    mvwaddstr(qtcwin, 2, 11, "CAPTURE ON ");
		    wattrset(qtcwin, attributes);
		    showfield(activefield);
		    refreshp();
		    show_rtty_lines();
		}
		break;

	    // Ctrl-E (^E), end capture
	    case CTRL_E:
		if (qtccurrdirection == RECV && trxmode == DIGIMODE) {
		    qtc_ry_capture = 0;
		    wattr_get(qtcwin, &attributes, &cpair, NULL);
		    wattrset(qtcwin, LINE_INVERTED);
		    mvwaddstr(qtcwin, 2, 11, "CAPTURE OFF");
		    wattrset(qtcwin, attributes);
		    showfield(activefield);
		    refreshp();
		}
		break;

	    // F1...F10
	    case KEY_F(1) ... KEY_F(10):
		if (trxmode == CWMODE || trxmode == DIGIMODE) {
		    if (direction == RECV) {
			sendmessage(qtc_recv_msgs[x - KEY_F(1)]); /* 0-9 */

			/* start recording */
			if (trxmode == CWMODE && strncmp(qtc_recv_msgs[x - KEY_F(1)], "QRV", 3) == 0) {
			    start_qtc_recording();
			}

			/* stop recording */
			if (trxmode == CWMODE
				&& strncmp(qtc_recv_msgs[x - KEY_F(1)], "QSL ALL", 7) == 0 && record_run > -1) {
			    stop_qtc_recording();
			}

			if (trxmode == DIGIMODE
				&& strncmp(qtc_recv_msgs[x - KEY_F(1)], "QRV", 3) == 0) {
			    qtc_ry_capture = 1;
			    wattr_get(qtcwin, &attributes, &cpair, NULL);
			    wattrset(qtcwin, LINE_INVERTED);
			    mvwaddstr(qtcwin, 2, 11, "CAPTURE ON ");
			    wattrset(qtcwin, attributes);
			    showfield(activefield);
			    refreshp();
			    show_rtty_lines();
			}
		    }

		    if (direction == SEND && strlen(qtc_send_msgs[x - KEY_F(1)]) > 0) {
			tlen = strlen(qtc_send_msgs[x - KEY_F(1)]) - 5; // len("sr/nr") = 5
			char tmess[60];
			tmess[0] = '\0';
			if (tlen > 0 && strncmp(qtc_send_msgs[x - KEY_F(1)] + tlen, "sr/nr", 5) == 0) {
			    g_strlcpy(tempc, qtc_send_msgs[x - KEY_F(1)], tlen);
			    sprintf(tmess, "%s %d/%d ", tempc, qtclist.serial, *qtccount);
			    sendmessage(tmess);
			} else if ((activefield - 3) >= 0) {
			    if (x - KEY_F(1) == 4) {	// F5, TIME
				strncpy(tmess, qtclist.qtclines[activefield - 3].qtc, 5);
				tmess[5] = '\0';
				sendmessage(tmess);
			    }
			    if (x - KEY_F(1) == 5) {	// F6, CALLSIGN
				strncpy(tmess, qtclist.qtclines[activefield - 3].qtc + 5, 13);
				for (i = 12; tmess[i] == ' '; i--)
				    ;
				tmess[i + 1] = ' ';
				tmess[i + 2] = '\0';
				sendmessage(tmess);
			    }
			    if (x - KEY_F(1) == 6) {	// F7, SERIAL
				strncpy(tmess, qtclist.qtclines[activefield - 3].qtc + 20, 4);
				tmess[4] = ' ';
				tmess[5] = '\0';
				sendmessage(tmess);
			    }
			} else {
			    sendmessage(qtc_send_msgs[x - KEY_F(1)]);
			}
		    }

		}
		if (trxmode == SSBMODE) {
		    if (direction == RECV) {
			if (x == KEY_F(2)) { // F2, "QRV"
			    start_qtc_recording();
			}
			if (x == KEY_F(10) && record_run > -1) { // F10, "QSL ALL"
			    stop_qtc_recording();
			}
			play_file(qtc_phrecv_message[x - KEY_F(1)]);
		    }
		    if (direction == SEND) {
			play_file(qtc_phsend_message[x - KEY_F(1)]);
		    }
		}

		break;

	    // <Delete>, Keypad <Del>
	    case KEY_DC:
		if (DIRCLAUSE) {
		    delete_from_field(0);
		}
		if (direction == SEND && activefield > 2 && trxmode != DIGIMODE) {
		    qtclist.qtclines[activefield - 3].sent = 0;
		    qtclist.qtclines[activefield - 3].flag = 0;
		    wattrset(qtcwin, LINE_CURRNORMAL);
		    mvwaddstr(qtcwin, activefield, 30, " ");
		    qtclist.totalsent--;
		}
		break;

	    // <Tab>
	    case TAB:
		if (direction == RECV) {
		    if (trxmode == DIGIMODE) {
			if (activefield == 32) {
			    activefield = 0;
			    showfield(32);
			} else {
			    activefield++;
			    showfield(activefield - 1);
			}
		    } else {
			if ((activefield < 3) || (activefield - 3) % 3 < 2) {
			    activefield++;
			    showfield(activefield - 1);
			} else {	/* (activefield-3)%3 == 2*/
			    activefield -= 2;
			    showfield(activefield + 2);
			}
		    }
		    showfield(activefield);
		}
		if (direction == SEND) {
		    switch (activefield) {
			case 0:
			    activefield = 2;
			    showfield(0);
			    showfield(2);
			    break;
			case 2:
			    activefield = 0;
			    showfield(2);
			    showfield(0);
			    break;
			default:
			    break;
		    }
		}
		curpos = 0;
		break;

	    // Shift <Tab>
	    case KEY_BTAB:
		if (direction == RECV) {
		    if (trxmode == DIGIMODE) {
			if (activefield == 0) {
			    activefield = 32;
			    showfield(0);
			} else {
			    activefield--;
			    showfield(activefield + 1);
			}
		    } else {
			if ((activefield > 0 && activefield < 3) ||
				((activefield - 3) % 3 > 0)) {
			    activefield--;
			    showfield(activefield + 1);
			} else {		/* (activefield-3)%3 == 0) */
			    activefield += 2;
			    showfield(activefield - 2);
			}
		    }
		    showfield(activefield);
		}
		if (direction == SEND) {
		    switch (activefield) {
			case 0:
			    activefield = 2;
			    showfield(0);
			    showfield(2);
			    break;
			case 2:
			    activefield = 0;
			    showfield(2);
			    showfield(0);
			    break;
			default:
			    break;
		    }
		}
		curpos = 0;
		break;

	    // <Space>
	    case SPACE:
		if (DIRCLAUSE) {
		    if (direction == RECV) {
			if (activefield > 2) {
			    if (activefield % 3 == 2) {
				activefield -= 2;
				showfield(activefield + 2);
			    } else {
				activefield++;
				showfield(activefield - 1);
			    }
			    showfield(activefield);
			}
			if (activefield == 2) {
			    if (strlen(qtcreclist.callsign) > 0 &&
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
				start_qtc_recording();
				activefield++;
				showfield(activefield);
			    }
			}
			if (activefield < 2) {
			    if (activefield == 0) {
				start_qtc_recording();
			    }
			    activefield++;
			    showfield(activefield - 1);
			    showfield(activefield);
			}
		    }
		}
		break;

	    // Numerals
	    case '0' ... '9':
		if (DIRCLAUSE) {
		    modify_field(x);
		}
		break;

	    /*case 65 ... 90:	// letters
	    	modify_field(x);
	    	break;*/

	    // Convert lower case to upper case
	    case 'a' ... 'z':
		if (DIRCLAUSE) {
		    modify_field(x - 32);
		}
		break;

	    // Slash
	    case '/':
		if (DIRCLAUSE) {
		    modify_field(x);
		}
		break;

	    // Query
	    case '?':
		if (DIRCLAUSE) {
		    modify_field(x);
		}
		break;

	    // <Backspace>
	    case KEY_BACKSPACE:
		if (DIRCLAUSE) {
		    delete_from_field(1);
		}
		break;

	    // <Page-Up>
	    case KEY_PPAGE:
		speedup();
		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 14, "%2u", GetCWSpeed());
		break;

	    // <Page-Down>
	    case KEY_NPAGE:
		speeddown();
		attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);
		mvprintw(0, 14, "%2u", GetCWSpeed());
		break;

	    // Comma or Ctrl-K (^K), keyboard window
	    case ',':
	    case 11:
		move(5, 0);
		keyer();
		x = 0;
		break;

	    // Ctrl-L (^L), mark callsign for late QTC
	    case CTRL_L:
		if (strlen(g_strstrip(qtccallsign)) > 3) {
		    qtc_inc(g_strstrip(qtccallsign), QTC_LATER);
		    sprintf(tempc, "%s;L\n", qtccallsign);
		    send_lan_message(QTCFLAG, tempc);
		}
		break;

	    // Ctrl-N (^N), mark callsign for explicit NO QTC
	    case CTRL_N:
		if (strlen(g_strstrip(qtccallsign)) > 3) {
		    qtc_inc(g_strstrip(qtccallsign), QTC_NO);
		    sprintf(tempc, "%s;N\n", qtccallsign);
		    send_lan_message(QTCFLAG, tempc);
		}
		break;

	    // Ctrl-F (^F), fill time fields with first 2 chars
	    case CTRL_F:
		if (activefield > 2) {
		    fill_qtc_times(qtcreclist.qtclines[(activefield - 3) / 3].time);
		    showfield(activefield);
		}
		break;

	    // Ctrl-R (^R), start/stop recording
	    case CTRL_R:
		if (direction == RECV) {
		    if (record_run < 0) {
			start_qtc_recording();
			showfield(activefield);
		    } else {
			stop_qtc_recording();
		    }
		}
	}
	refreshp();
	if (x != ESCAPE) {
	    x = 0;
	}
    }
    hide_panel(qtc_panel);
}

void showfield(int fidx) {

    char fieldval[20], filled[20];
    int qtcrow, winrow, fi, posidx, i;

    fieldval[0] = '\0';

    posidx = 0;
    if (fidx == 0) {
	sprintf(fieldval, "%s", qtccallsign);
	winrow = 1;
	posidx = 0;
	qtc_temp_obj = qtc_get(qtccallsign);
	put_qtc();
    } else if (fidx == 1) {
	sprintf(fieldval, "%4d", qtcreclist.serial);
	winrow = 2;
	posidx = 1;
    } else if (fidx == 2) {
	sprintf(fieldval, "%d", *qtccount);
	winrow = 2;
	posidx = 2;
    } else {
	fi = fidx - 3;
	winrow = (fi / 3) + 3;
	qtcrow = winrow - 3;
	switch (fi % 3) {
	    case 0:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].time);
		posidx = 3;
		break;
	    case 1:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].callsign);
		posidx = 4;
		break;
	    case 2:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].serial);
		posidx = 5;
		break;
	}
	show_status(qtcrow);
    }
    curfieldlen = strlen(fieldval);

    for (i = 0; i < pos[posidx][2]; i++) {
	filled[i] = ' ';
    }
    filled[i] = '\0';
    if (fidx == activefield) {
	wattrset(qtcwin, LINE_CURRINVERTED);
    } else {
	wattrset(qtcwin, LINE_NORMAL);
    }

    mvwaddstr(qtcwin, winrow, pos[posidx][0], filled);
    mvwaddstr(qtcwin, winrow, pos[posidx][0], fieldval);
    if (fidx == activefield) {
	show_help_msg(posidx);
	if (pos[posidx][0] == pos[posidx][1]) {
	    wmove(qtcwin, winrow, (pos[posidx][1] + strlen(fieldval)) - curpos);
	} else {
	    wmove(qtcwin, winrow, pos[posidx][1] - curpos);
	}
    }
}

void modify_field(int pressed) {
    char fieldval[16];
    int fi, winrow, qtcrow, posidx, stridx;

    memset(fieldval, '\0', 15);

    posidx = 0;
    if (activefield == 0 && (isalnum(pressed) || pressed == '/')
	    && strlen(qtccallsign) < pos[0][2] - 1) {
	qtccallsign[strlen(qtccallsign) - curpos] = pressed;
	if (curpos > 0) {
	    curpos--;
	}
	recalc_qtclist();
	showfield(0);
    } else if (activefield == 1 && isdigit(pressed)) {
	sprintf(fieldval, "%d", (qtcreclist.serial) * 10);
	fieldval[strlen(fieldval) - (1 + curpos)] = pressed;
	if (curpos > 0) {
	    curpos--;
	}
	if (strlen(fieldval) <= pos[1][2]) {
	    qtcreclist.serial = atoi(fieldval);
	    showfield(1);
	}
    } else if (activefield == 2 && isdigit(pressed)) {
	sprintf(fieldval, "%d", (*qtccount) * 10);
	fieldval[strlen(fieldval) - (1 + curpos)] = pressed;
	if (curpos > 0) {
	    curpos--;
	}
	if (strlen(fieldval) <= pos[2][2] && atoi(fieldval) <= 10) {
	    qtc_temp_obj = qtc_get(qtccallsign);
	    if (qtccurrdirection == SEND) {
		if (*qtccount != atoi(fieldval)) {
		    if ((atoi(fieldval) + (qtc_temp_obj->total)) >= 10) {
			sprintf(fieldval, "%d", (10 - (qtc_temp_obj->total)));
		    }
		    *qtccount = genqtclist(qtccallsign, atoi(fieldval));
		    show_sendto_lines();
		    put_qtc();
		}
	    }
	    if (qtccurrdirection == RECV) {
		if ((atoi(fieldval) + (qtc_temp_obj->total)) >= 10) {
		    sprintf(fieldval, "%d", (10 - (qtc_temp_obj->total)));
		}
		*qtccount = atoi(fieldval);
	    }
	    number_fields();
	    showfield(2);
	}
    } else if (activefield >= 3 && activefield <= 32) {
	fi = activefield - 3;
	winrow = (fi / 3) + 3;
	qtcrow = winrow - 3;
	stridx = fi % 3;
	switch (stridx) {
	    case 0:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].time);
		posidx = 3;
		break;
	    case 1:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].callsign);
		posidx = 4;
		break;
	    case 2:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].serial);
		posidx = 5;
		break;
	}
	if (pressed == '?') {
	    qtcreclist.qtclines[qtcrow].status = 1;	// set incomplete the qtc status
	    show_status(qtcrow);
	}
	if ((((stridx == 0 || stridx == 2) && (isdigit(pressed) || pressed == '?'))
		|| stridx == 1) && strlen(fieldval) < pos[posidx][2]) {

	    fieldval[strlen(fieldval) - curpos] = pressed;
	    switch (stridx) {
		case 0:
		    strcpy(qtcreclist.qtclines[qtcrow].time, fieldval);
		    if (qtc_auto_filltime) {
			fill_qtc_times(fieldval);
		    }
		    break;
		case 1:
		    strcpy(qtcreclist.qtclines[qtcrow].callsign, fieldval);
		    break;
		case 2:
		    strcpy(qtcreclist.qtclines[qtcrow].serial, fieldval);
		    break;
	    }
	    if (curpos > 0) {
		curpos--;
	    }
	    showfield(activefield);
	    if (stridx == 0 && strlen(fieldval) == pos[posidx][2]
		    && activefield <= 32) {	// auto TAB if curr field is QTC time, and len is 4
		curpos = 0;
		activefield++;
		showfield(activefield - 1);
		showfield(activefield);
	    }
	}
    }
}

void delete_from_field(int dir) {

    char fieldval[16];
    int fi, winrow, qtcrow, stridx;

    fieldval[0] = '\0';

    if (activefield == 0) {
	if (strlen(qtccallsign) > 0) {
	    sprintf(fieldval, "%s", qtccallsign);
	    shift_left(fieldval, dir);
	    g_strlcpy(qtccallsign, fieldval, QTC_CALL_SIZE);
	    qtccallsign[strlen(fieldval)] = '\0';
	    recalc_qtclist();
	    showfield(0);
	}
    } else if (activefield == 1) {
	sprintf(fieldval, "%d", qtcreclist.serial);
	if (strlen(fieldval) > 0) {
	    //sprintf(fieldval, "%d", qtcreclist.serial/10);
	    shift_left(fieldval, dir);
	    qtcreclist.serial = atoi(fieldval);
	    showfield(1);
	}
    } else if (activefield == 2) {
	sprintf(fieldval, "%d", *qtccount);
	if (strlen(fieldval) > 0) {
	    //sprintf(fieldval, "%d", *qtccount/10);
	    shift_left(fieldval, dir);
	    *qtccount = atoi(fieldval);
	    showfield(2);
	}
    } else {
	fi = activefield - 3;
	winrow = (fi / 3) + 3;
	qtcrow = winrow - 3;
	stridx = fi % 3;
	switch (stridx) {
	    case 0:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].time);
		break;
	    case 1:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].callsign);
		break;
	    case 2:
		sprintf(fieldval, "%s", qtcreclist.qtclines[qtcrow].serial);
		break;
	}

	if (strlen(fieldval) > 0) {
	    shift_left(fieldval, dir);
	    fieldval[strlen(fieldval)] = '\0';
	    switch (stridx) {
		case 0:
		    strcpy(qtcreclist.qtclines[qtcrow].time, fieldval);
		    break;
		case 1:
		    strcpy(qtcreclist.qtclines[qtcrow].callsign, fieldval);
		    break;
		case 2:
		    strcpy(qtcreclist.qtclines[qtcrow].serial, fieldval);
		    break;
	    }
	    showfield(activefield);
	}
    }
}

void shift_left(char *fieldval, int shift) {
    int i;

    for (i = strlen(fieldval) - (curpos + shift); i < strlen(fieldval); i++) {
	fieldval[i] = fieldval[i + 1];
    }
    if (shift == 0) {
	curpos--;
    }
}

void show_status(int idx) {

    char flag = ' ';
    int i, status = 0;

    status = 0;
    if (idx < *qtccount) {
	if (strlen(qtcreclist.qtclines[idx].time) != 4) {
	    status = 1;
	} else {
	    for (i = 0; i < strlen(qtcreclist.qtclines[idx].time); i++) {
		if (qtcreclist.qtclines[idx].time[i] == '?'
			|| ! isdigit(qtcreclist.qtclines[idx].time[i])) {
		    status = 1;
		    break;
		}
	    }
	}
	if (strlen(qtcreclist.qtclines[idx].callsign) < 3) {
	    status = 1;
	} else {
	    for (i = 0; i < strlen(qtcreclist.qtclines[idx].callsign); i++) {
		if (qtcreclist.qtclines[idx].callsign[i] == '?') {
		    status = 1;
		    break;
		}
	    }
	}
	if (strlen(qtcreclist.qtclines[idx].serial) == 0) {
	    status = 1;
	}
	for (i = 0; i < strlen(qtcreclist.qtclines[idx].serial); i++) {
	    if (qtcreclist.qtclines[idx].serial[i] == '?'
		    || ! isdigit(qtcreclist.qtclines[idx].serial[i])) {
		status = 1;
		break;
	    }
	}
	if (status == 1) {
	    qtcreclist.qtclines[idx].status = 1;
	} else if (qtcreclist.qtclines[idx].status !=
		   2) {	// unset incomplete mark if not marked as complete
	    qtcreclist.qtclines[idx].status = 0;
	}

	switch (qtcreclist.qtclines[idx].status) {
	    case 0:
		flag = ' ';
		break;
	    case 1:
		flag = '?';
		break;
	    case 2:
		flag = '*';
		break;
	}
	if (trxmode == DIGIMODE && qtcreclist.qtclines[idx].status == 0) {
	    flag = '+';
	}
	wattrset(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
	mvwprintw(qtcwin, idx + 3, 30, "%c", flag);
    }
}

void number_fields() {
    int i;

    wattrset(qtcwin, (chtype)(A_NORMAL | COLOR_PAIR(QTCRECVBG)));
    for (i = 0; i < QTC_LINES; i++) {
	mvwaddstr(qtcwin, i + 3, 1, "  ");
    }
    for (i = 0; i < *qtccount; i++) {
	mvwprintw(qtcwin, i + 3, 1, "%2d", i + 1);
    }
}

void clear_help_block() {
    int i;

    wattrset(qtcwin, LINE_INVERTED);
    for (i = 1; i < 13; i++) {
	mvwaddstr(qtcwin, i, 36, "                                      ");
    }
}

void show_help_msg(int msgidx) {
    int i = 0, j = 0;
    char buff[80];
    int currqtc;

    clear_help_block();

    wattrset(qtcwin, LINE_CURRINVERTED);
    if (qtccurrdirection == RECV) {
	if (trxmode == DIGIMODE && qtccurrdirection == RECV) {
	    currqtc = ((activefield - 3) / 3);
	    if (qtcreclist.qtclines[currqtc].status == 0 &&
		    strlen(qtcreclist.qtclines[currqtc].time) == 4 &&
		    strlen(qtcreclist.qtclines[currqtc].callsign) > 0 &&
		    strlen(qtcreclist.qtclines[currqtc].serial) > 0
	       ) {
		mvwaddstr(qtcwin, ++j, 36, "Press ENTER to mark as RCVD");
	    }
	} else {
	    mvwaddstr(qtcwin, ++j, 36, help_rec_msgs[msgidx]);
	}
    }
    if (qtccurrdirection == SEND) {
	if (msgidx > 2 && msgidx < 6) {
	    msgidx = 3;
	}
	mvwaddstr(qtcwin, ++j, 36, help_send_msgs[msgidx]);
    }
    wattrset(qtcwin, LINE_INVERTED);
    mvwaddstr(qtcwin, ++j, 36, "PgUP/PgDW: QRQ/QRS      CTRL-N: NO QTC");
    if (qtccurrdirection == RECV) {
	mvwaddstr(qtcwin, ++j, 36, "ENTER: R & next OR AGN  CTRL-L: LATER ");
    }
    if (qtccurrdirection == SEND) {
	mvwaddstr(qtcwin, ++j, 36, "ENTER: send QTC         CTRL-L: LATER ");
    }
    for (i = 0; i < 12 && j < 12; i++) {
	if (qtccurrdirection == RECV) {
	    if (strlen(qtc_recv_msgs[i]) > 0) {
		g_strlcpy(buff, qtc_recv_msgs[i], sizeof(buff));
		mvwprintw(qtcwin, ++j, 36, "F%-2d: %s", (i + 1), buff);
	    }
	    if (i == 1) {
		mvwaddstr(qtcwin, j - 1, 56, "CTRL-F: FILL TIMES");
	    }
	    if (i == 2) {
		mvwaddstr(qtcwin, j - 1, 56, "CTRL-R: RECORD");
	    }
	}
	if (qtccurrdirection == SEND) {
	    if (strlen(qtc_send_msgs[i]) > 0) {
		g_strlcpy(buff, qtc_send_msgs[i], sizeof(buff));
		mvwprintw(qtcwin, ++j, 36, "F%-2d: %s", (i + 1), buff);
	    }
	}
    }
    if (trxmode == DIGIMODE && qtccurrdirection == RECV) {
	mvwaddstr(qtcwin, ++j, 36, "CTRL-T: Terminal window");
	mvwaddstr(qtcwin, ++j, 36, "CTRL-S: Start capture");
	mvwaddstr(qtcwin, ++j, 36, "CTRL-E: End capture");
    }
}

int parse_ry_line(char *line) {
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

    g_strlcpy(lline, line, sizeof(lline));
    tactivefield = activefield;
    if (nr_parsed_line == 0) {		// 1st line: SERIAL/NR
	wtoken = strtok_r(lline, " ", &saveptr1);
	while (wtoken != NULL && nr_parsed_line == 0) {
	    serial_nr = 0;
	    strcpy(ttoken, wtoken);
	    sep[0] = '/';
	    sep[1] = '\0';
	    token = strtok_r(ttoken, sep, &saveptr2);
	    t = -1;
	    while (token != NULL && t < 2) {
		t++;
		strcpy(tokens[t], token);
		token = strtok_r(NULL, sep, &saveptr2);
	    }
	    f = 0;
	    for (i = 0; i <= t; i++) {
		f = 1;
		for (j = 0; j < strlen(tokens[i]); j++) {
		    if (! isdigit(tokens[i][j])) {
			f = 0;
		    }
		}
		if (f == 1) {
		    tactivefield = activefield;
		    activefield = i + 1;
		    showfield(tactivefield);
		    for (j = 0; j < strlen(tokens[i]); j++) {
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
    } else {
	strcpy(sep, "/;-: ");
	token = strtok(lline, sep);
	while (token != NULL && t < 4) {
	    t++;
	    strcpy(tokens[t], token);
	    token = strtok(NULL, sep);
	}

	while (t > 2) {
	    strcat(tokens[1], "/");
	    strcat(tokens[1], tokens[2]);
	    strcpy(tokens[t - 1], tokens[t]);
	    tokens[t][0] = '\0';
	    t--;
	}

	for (i = 0; i <= t; i++) {
	    f = 1;
	    for (j = 0; j < strlen(tokens[i]); j++) {
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
		activefield = (nr_parsed_line * 3) + i;
		showfield(tactivefield);
		for (j = 0; j < strlen(tokens[i]); j++) {
		    modify_field(tokens[i][j]);
		}
		showfield(activefield);
	    }
	}
	for (i = ((t > 0) ? t : 0); i < 2; i++) {
	    tactivefield = activefield;
	    activefield = (nr_parsed_line * 3) + i;
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
void show_rtty_lines() {
    extern int miniterm;

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
    wattrset(ry_help_win, LINE_INVERTED);
    //                           "1234567890123456789012345678901234567890123456789012345678901234567890123"
    mvwprintw(ry_help_win, 1, 1,
	      "CTRL-R: Reset lines  -  CTRL-S: Start capture  -  CTRL-E: End capture    ");
    mvwprintw(ry_help_win, 2, 1,
	      "ENTER: add current line to QTC  -  Up/Down: move                         ");
    mvwprintw(ry_help_win, 3, 1,
	      "ESC: hide RTTY window                                                    ");

    strcpy(boxhead, "RTTY");
    wnicebox(ry_win, 0, 0, 12, 38, boxhead);
    wattrset(ry_win, LINE_NORMAL);
    for (j = 1; j < 13; j++) {
	mvwaddstr(ry_win, j, 1, "                                      ");
    }
    refreshp();

    if (qtc_ry_capture == 1) {
	mvwaddstr(qtcwin, 2, 11, "CAPTURE ON ");
    } else {
	mvwaddstr(qtcwin, 2, 11, "CAPTURE OFF");
    }

    x = -1; j = 1;
    prevline = -1;
    curs_set(0);
    while (x != ESCAPE) {

	while (x < 1) {

	    usleep(1000);
	    time_update();

	    if (miniterm == 1 && trxmode == DIGIMODE && digikeyer != NO_KEYER) {
		show_rtty();
	    }

	    if ((qtc_ry_currline < 11 && prevline != qtc_ry_currline - 1)
		    || qtc_ry_currline == 11) {
		if (actline > 1 && strcmp(firstline, qtc_ry_lines[0].content) != 0) {
		    actline--;
		    strcpy(firstline, qtc_ry_lines[0].content);
		}
		if (actline == 1 && qtc_ry_currline > 0) {
		    strcpy(firstline, qtc_ry_lines[0].content);
		}
		for (j = 0; j < qtc_ry_currline; j++) {
		    if (j + 1 == actline) {
			wattrset(ry_win, LINE_CURRNORMAL);
		    }
		    print_rtty_line(qtc_ry_lines[j], j + 1);
		    wattrset(ry_win, LINE_NORMAL);
		}
		prevline = qtc_ry_currline - 1;
	    }

	    strncpy(currline, qtc_ry_lines[qtc_ry_currline].content, 38);
	    currline[39] = '\0';
	    mvwprintw(ry_win, qtc_ry_currline + 1, 1, "%-38s", currline);

	    refreshp();
	    x = key_poll();
	}

	switch (x) {

	    // Up arrow
	    case KEY_UP:
		wattrset(ry_win, LINE_NORMAL);
		print_rtty_line(qtc_ry_lines[actline - 1], actline);
		if (actline > 1) {
		    actline--;
		} else {
		    if (qtc_ry_currline > 0) {
			actline = qtc_ry_currline;
		    }
		}
		wattrset(ry_win, LINE_CURRNORMAL);
		print_rtty_line(qtc_ry_lines[actline - 1], actline);
		wattrset(ry_win, LINE_NORMAL);
		break;

	    // Down arrow
	    case KEY_DOWN:
		wattrset(ry_win, LINE_NORMAL);
		print_rtty_line(qtc_ry_lines[actline - 1], actline);
		if (actline < qtc_ry_currline) {
		    actline++;
		} else {
		    actline = 1;
		}
		wattrset(ry_win, LINE_CURRNORMAL);
		print_rtty_line(qtc_ry_lines[actline - 1], actline);
		wattrset(ry_win, LINE_NORMAL);
		break;

	    // Ctrl-R (^R)
	    case CTRL_R:
		for (j = 0; j < 12; j++) {
		    qtc_ry_lines[j].content[0] = '\0';
		    qtc_ry_lines[j].attr = 0;
		    mvwaddstr(ry_win, (j + 1), 1, "                                      ");
		}
		prevline = -1;
		actline = 1;
		qtc_ry_currline = 0;
		qtc_ry_copied = 0;
		break;

	    // Ctrl-S (^S), start capture
	    case CTRL_S:
		qtc_ry_capture = 1;
		mvwaddstr(qtcwin, 2, 11, "CAPTURE ON ");
		break;

	    // Ctrl-E (^E), end capture
	    case CTRL_E:
		qtc_ry_capture = 0;
		mvwaddstr(qtcwin, 2, 11, "CAPTURE OFF");
		break;

	    // <Enter>, add to qtc
	    case LINEFEED:
		if (qtc_ry_lines[actline - 1].attr == 0) {
		    parse_ry_line(qtc_ry_lines[actline - 1].content);
		    qtc_ry_lines[actline - 1].attr = 1;
		}
		wattrset(ry_win, LINE_CURRNORMAL);
		print_rtty_line(qtc_ry_lines[actline - 1], actline);
		wattrset(ry_win, LINE_NORMAL);
		break;
	}
	refreshp();
	if (x != ESCAPE) {
	    x = -1;
	}
	if (*qtccount > 0 && qtc_ry_copied == qtcreclist.count) {
	    qtc_ry_capture = 0;
	    mvwaddstr(qtcwin, 2, 11, "CAPTURE OFF");
	}
    }
    hide_panel(ry_panel);
    hide_panel(ry_help_panel);

    curs_set(1);
    tactivefield = activefield;
    activefield = oactivefield;
    showfield(tactivefield);
    showfield(oactivefield);
}

void put_qtc() {

    char qtcdirstring[3][10] = {"", "Received", "Sent"};

    wattrset(qtcwin, LINE_NORMAL);
    if (qtc_temp_obj->capable == -1 && qtc_temp_obj->total == 0) {
	mvwaddstr(qtcwin, 1, 19, "FLAG: NO QTC");
    } else {
	mvwprintw(qtcwin, 1, 19, "%s %2d QTC", qtcdirstring[qtccurrdirection],
		  qtc_temp_obj->total);
    }

}

void replace_spaces(char *src, char *tempc) {
    int tsp, tdp;

    tsp = 0;
    tdp = 0;
    g_strstrip(src);
    while (src[tsp] != '\0') {
	if (src[tsp] != ' ') {
	    tempc[tdp] = src[tsp];
	    tdp++;
	} else {
	    if (trxmode == DIGIMODE) {
		tempc[tdp] = '-';
	    } else {
		tempc[tdp] = ' ';
	    }
	    tdp++;
	    while (src[tsp + 1] == ' ' || src[tsp + 1] == '-') {
		tsp++;
	    }
	}
	tsp++;
    }
    tempc[tdp] = '\n';
    tdp++;
    tempc[tdp] = '\0';
}

void show_sendto_lines() {
    int i;

    wattrset(qtcwin, LINE_INVERTED);
    for (i = 0; i < QTC_LINES; i++) {
	mvwaddstr(qtcwin, i + 3, 1, "                                 ");
    }

    wattrset(qtcwin, LINE_NORMAL);
    for (i = 0; i < qtclist.count; i++) {
	mvwaddstr(qtcwin, i + 3, 4, qtclist.qtclines[i].qtc);
	if (qtclist.qtclines[i].sent == 1) {
	    mvwaddstr(qtcwin, i + 3, 30, "*");
	}
    }

    wattrset(qtcwin, LINE_NORMAL);
    for (i = qtclist.count; i < QTC_LINES; i++) {
	mvwaddstr(qtcwin, i + 3, 4, "                        ");
    }
    number_fields();
}

void recalc_qtclist() {
    if (qtccurrdirection == SEND) {
	if (strlen(qtccallsign) > 0 && strcmp(qtccallsign, prevqtccall) != 0) {
	    qtc_temp_obj = qtc_get(qtccallsign);
	    *qtccount = genqtclist(qtccallsign, (10 - (qtc_temp_obj->total)));
	    show_sendto_lines();
	    showfield(2);
	    put_qtc();
	}
    }
    g_strlcpy(prevqtccall, qtccallsign, sizeof(prevqtccall));
}
