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

/* Splitscreen telnet client */
/* Uses ncurses, so hopefully compatible with much */
/* dave brown n2rjt (dcb@vectorbd.com) wrote most of this */
/* TLF integration and TNC interface by PA0RCT   07/30/2002 */

#define VERSIONSPLIT "V1.4.1 5/18/96 - N2RJT"

#include "splitscreen.h"
#include "bandmap.h"
#include "get_time.h"
#include <pthread.h>

pthread_mutex_t spot_ptr_mutex = PTHREAD_MUTEX_INITIALIZER;

int currow;
int curcol;
int maxtln_loglines = DEFAULTTLN_LOGLINES;
int attr[3];
int color[3][2];
int curattr = 0;
int insmode = TRUE;
char outbuffer[82];
char lanflag;
int lanspotflg = 0;

int tln_loglines = 0;

int view_state = STATE_EDITING;
char tln_input_buffer[2 * BUFFERSIZE];

void addlog(char *s)
{
    extern char lastwwv[];
    extern struct tln_logline *loghead;
    extern struct tln_logline *logtail;
    extern struct tln_logline *viewing;
    extern char spot_ptr[MAX_SPOTS][82];
    extern char lastmsg[];
    extern int ptr;
    extern int clusterlog;

    int len;
    FILE *fp;
    struct tln_logline *temp;

    pthread_mutex_lock (&spot_ptr_mutex);

    for (len = 0; len < strlen(s); len += 80) {

	strncpy(spot_ptr[ptr], s + len, 82);
	if (strlen(spot_ptr[ptr]) > 5) {
	    lastmsg[0] = '\0';
	    strncat(lastmsg, spot_ptr[ptr], 82);
	}

	if (clusterlog == 1) {
	    if ((fp = fopen("clusterlog", "a")) == NULL) {
		mvprintw(24, 0, "Opening clusterlog not possible.");
	    } else {
		if (strlen(lastmsg) > 20) {
		    fputs(lastmsg, fp);
		    fputs("\n", fp);
		}

		fclose(fp);
	    }

	}
	ptr++;

	if (ptr > MAX_SPOTS - 1) {
	    for (ptr = 10; ptr <= MAX_SPOTS - 1; ptr++)
		strncpy(spot_ptr[ptr - 10], spot_ptr[ptr], 82);
	    for (ptr = MAX_SPOTS - 1; ptr >= MAX_SPOTS - 11; ptr--)
		spot_ptr[ptr][0] = '\0';
	    ptr = MAX_SPOTS - 10;
	}
    }

    pthread_mutex_unlock (&spot_ptr_mutex);

    // \todo drop it later tb mar11
    bm_add(s);

    if ((strncmp( s, "WWV", 3) == 0) || strncmp ( s, "WCY", 3) == 0)
	strncpy (lastwwv, s, 82);

    if (tln_loglines >= maxtln_loglines) {
	temp = loghead;
	loghead = loghead->next;
	loghead->prev = NULL;
	if (viewing == temp) {
	    viewing = loghead;
	}
	free(temp->text);
    } else {
	temp = (struct tln_logline *) malloc(sizeof(struct tln_logline));
	tln_loglines++;
    }
    temp->next = NULL;
    temp->text = strdup(s);
    temp->attr = curattr;
    if (loghead) {
	logtail->next = temp;
	temp->prev = logtail;
    } else {
	loghead = temp;
	temp->prev = NULL;
    }
    logtail = temp;
}

int logattr(void)
{
    extern struct tln_logline *viewing;

    if (!viewing)
	return 0;
    else
	return viewing->attr;
}

char *firstlog(void)
{
    extern struct tln_logline *loghead;
    extern struct tln_logline *viewing;

    viewing = loghead;
    view_state = STATE_VIEWING;
    return viewing->text;
}

char *lastlog(void)
{
    extern struct tln_logline *logtail;
    extern struct tln_logline *viewing;

    viewing = logtail;
    view_state = STATE_VIEWING;
    return viewing->text;
}

char *nextlog(void)
{
    extern struct tln_logline *loghead;
    extern struct tln_logline *logtail;
    extern struct tln_logline *viewing;

    if (view_state == STATE_EDITING)
	viewing = loghead;
    else if (viewing)
	viewing = viewing->next;
    if (viewing) {
	view_state = STATE_VIEWING;
	return viewing->text;
    } else {
	viewing = logtail;
	return NULL;
    }
}

char *prevlog(void)
{
    extern struct tln_logline *loghead;
    extern struct tln_logline *logtail;
    extern struct tln_logline *viewing;

    if (view_state == STATE_EDITING)
	viewing = logtail;
    else if (viewing)
	viewing = viewing->prev;
    if (viewing) {
	view_state = STATE_VIEWING;
	return viewing->text;
    } else {
	viewing = loghead;
	return NULL;
    }
}

void start_editing(void)
{
    extern struct tln_logline *viewing;
    extern WINDOW *entwin;

    werase(entwin);
    currow = curcol = 0;
    viewing = NULL;
    view_state = STATE_EDITING;
}

void delete_prev_char(void)
{
    extern WINDOW *entwin;

    int i, j;
    int c, cc;
    if (currow != 0 || curcol != 0) {
	if (curcol-- == 0) {
	    curcol = COLS - 1;
	    currow--;
	}
	c = ' ';
	for (i = ENTRYROWS - 1, j = COLS - 1; i > currow || j >= curcol;
	     j--) {
	    if (j < 0) {
		j = COLS - 1;
		i--;
	    }
	    cc = mvwinch(entwin, i, j);
	    waddch(entwin, c);
	    c = cc;
	}
	wmove(entwin, currow, curcol);
    }
}

void right_arrow(void)
{
    extern WINDOW *entwin;

    if (++curcol >= COLS) {
	curcol = 0;
	if (++currow >= ENTRYROWS) {
	    currow = ENTRYROWS - 1;
	    curcol = COLS - 1;
	}
    }
    wmove(entwin, currow, curcol);
}

void left_arrow(void)
{
    extern WINDOW *entwin;

    if (--curcol < 0) {
	curcol = COLS - 1;
	if (--currow < 0) {
	    currow = curcol = 0;
	}
    }
    wmove(entwin, currow, curcol);
}

void move_eol(void)
{
    extern WINDOW *entwin;

    currow = ENTRYROWS - 1;
    curcol = COLS - 1;
    while ((A_CHARTEXT & mvwinch(entwin, currow, curcol)) == ' ') {
	curcol--;
	if (curcol < 0) {
	    if (currow > 0) {
		currow--;
		curcol = COLS - 1;
	    } else {
		break;
	    }
	}
    }
    right_arrow();
}

void gather_input(char *s)
{
    extern WINDOW *entwin;
    int l = 0;
    int i, j;
    for (i = j = 0; i < ENTRYROWS && j < 81; j++) {
	if (j >= COLS) {
	    j = 0;
	    i++;
	}
	if (i < ENTRYROWS)
	    s[l++] = A_CHARTEXT & mvwinch(entwin, i, j);
    }
    while (--l >= 0) {
	if (s[l] != ' ')
	    break;
	else
	    s[l] = '\0';
    }
    /*s[++l] = '\n'; */
}

int attop = 0;
int walkup(void)
{
    int i;
    if (attop)
	return 0;
    attop = TRUE;
    for (i = 0; i < LINES - ENTRYROWS - 1; i++) {
	if (prevlog() == NULL) {	/* Not enough to view .. you already see it all */
	    beep();
	    return 1;
	}
    }
    return 0;
}

int walkdn(void)
{
    int i;
    if (!attop)
	return 0;
    attop = FALSE;
    for (i = 0; i < LINES - ENTRYROWS - 1; i++) {
	if (nextlog() == NULL) {	/* Not enough to view */
	    beep();
	    return 1;
	}
    }
    return 0;
}

int pageup(int lines)
{
    extern WINDOW *sclwin;

    int i;
    char *s = NULL;
    walkup();
    for (i = 0; i < lines; i++) {
	wmove(sclwin, 0, 0);
	s = prevlog();
	if (s == NULL) {
	    beep();
	    break;
	}
	winsertln(sclwin);
	wattrset(sclwin, logattr());
	mvwaddstr(sclwin, 0, 0, s);
    }
    wrefresh(sclwin);
    return (s != NULL);
}

int pagedn(int lines)
{
    extern WINDOW *sclwin;

    int i;
    char *s = NULL;
    walkdn();
    for (i = 0; i < lines; i++) {
	s = nextlog();
	if (s == NULL) {
	    beep();
	    break;
	}
	scroll(sclwin);
	wattrset(sclwin, logattr());
	mvwprintw(sclwin, LINES - ENTRYROWS - 1, 0, "%s", s);
    }
    wrefresh(sclwin);
    return (s != NULL);
}

char entry_text[BUFFERSIZE];

void viewbottom(void)
{
    extern WINDOW *sclwin;

    int i;
    char *s;

    for (i = 0; i < LINES - ENTRYROWS; i++) {
	if (i == 0)
	    s = lastlog();
	else
	    s = prevlog();
	if (s == NULL)
	    break;
    }
    werase(sclwin);
    while (1) {
	s = nextlog();
	if (s == NULL)
	    break;
	wattrset(sclwin, logattr());
	wprintw(sclwin, "%s\n", s);
    }
    if (strlen(tln_input_buffer) > 0) {
	wprintw(sclwin, "%s", tln_input_buffer);
    }
    wrefresh(sclwin);
    attop = FALSE;
}

void viewtop(void)
{
    extern WINDOW *sclwin;

    int i;
    char *s;

    werase(sclwin);
    for (i = 0; i < LINES - ENTRYROWS; i++) {
	if (i == 0)
	    s = firstlog();
	else
	    s = nextlog();
	if (s == NULL)
	    break;
	wattrset(sclwin, logattr());
	wprintw(sclwin, "%s\n", s);
    }
    wrefresh(sclwin);
    attop = FALSE;
}

void resume_editing(void)
{
    extern WINDOW *entwin;
    extern WINDOW *sclwin;
    extern struct tln_logline *viewing;

    viewbottom();
    wattrset(sclwin, curattr);
    werase(entwin);
    mvwprintw(entwin, 0, 0, entry_text);
    wmove(entwin, currow, curcol);
    viewing = NULL;
    view_state = STATE_EDITING;
    wrefresh(entwin);
}

void viewlog(void)
{
    extern WINDOW *entwin;
    extern struct tln_logline *viewing;

    attop = FALSE;
    if (walkup()) {
	view_state = STATE_EDITING;
	viewing = NULL;
	return;
    }
    gather_input(entry_text);
    werase(entwin);
    mvwprintw(entwin, 0, 0, "Viewing data... hit ENTER to return\n");
    pageup(SCROLLSIZE);
}

int litflag = FALSE;
int edit_line(int c)
{
    extern WINDOW *entwin;
    extern WINDOW *sclwin;

    if (view_state != STATE_EDITING) {
	if (c == '\n')
	    resume_editing();
	else if (c == KEY_PPAGE)
	    pageup(SCROLLSIZE);
	else if (c == KEY_NPAGE)
	    pagedn(SCROLLSIZE);
	else if (c == KEY_UP)
	    pageup(1);
	else if (c == KEY_DOWN)
	    pagedn(1);
	else if (c == KEY_HOME)
	    viewtop();
	else if (c == KEY_END)
	    viewbottom();
    } else {
	if (litflag) {
	    wprintw(sclwin, "Keycode = %o octal\n", c);
	    wrefresh(sclwin);
	    litflag = FALSE;
	    return 0;
	}
	if (c == erasechar() || c == KEY_BACKSPACE)
	    delete_prev_char();
	else if (c == killchar() || c == KEY_CLEAR)
	    start_editing();
	else if (c == '\n')
	    return 1;
	else if (c == KEY_IC)
	    insmode = TRUE;
	else if (c == KEY_EIC)
	    insmode = FALSE;
	else if (c == KEY_F(10))
	    insmode = !insmode;
	else if (c == KEY_DOWN) {
	    if (currow < ENTRYROWS)
		currow++;
	    wmove(entwin, currow, curcol);
	} else if (c == KEY_UP) {
	    if (currow > 0)
		currow--;
	    wmove(entwin, currow, curcol);
	} else if (c == KEY_PPAGE) {
	    viewlog();
	} else if (c == KEY_LEFT) {
	    left_arrow();
	} else if (c == KEY_RIGHT) {
	    right_arrow();
	} else if (c == KEY_HOME || c == '\001') {
	    currow = curcol = 0;
	    wmove(entwin, currow, curcol);
	} else if (c == KEY_END || c == '\005') {
	    move_eol();
	} else if (c == KEY_DC) {
	    right_arrow();
	    delete_prev_char();
	} else if (c == KEY_EOS) {
	    wclrtobot(entwin);
	} else if (c == KEY_EOL) {
	    wclrtoeol(entwin);
	} else if (c == KEY_F(9)) {
	    litflag = TRUE;
	} else if (c == (c & A_CHARTEXT)) {
	    if (insmode) {
		if (currow < ENTRYROWS - 1) {
		    int i;
		    i = A_CHARTEXT & mvwinch(entwin, currow, COLS - 1);
		    mvwinsch(entwin, currow + 1, 0, i);
		}
		mvwinsch(entwin, currow, curcol, c);
	    } else {
		waddch(entwin, c);
	    }
	    curcol++;
	    if (curcol == COLS && currow < ENTRYROWS - 1) {
		curcol = 0;
		currow++;
	    }
	    wmove(entwin, currow, curcol);
	}
    }
    return 0;
}

void sanitize(char *s)
{
    char *t;
    for (t = s; *s != '\0'; s++) {
	if (*s == '\007')
	    beep();
	else if (*s == '\015');
	else if (*s < 0)
	    *t++ = *s - 128;
	else
	    *t++ = *s;
    }
    *t = '\0';
}

void addtext(char *s)
{
    extern int use_rxvt;
    extern WINDOW *sclwin;
    extern int in_packetclient;
    extern int lan_active;
    extern char call[];
    extern char hiscall[];
    extern struct tm *time_ptr;
    extern char talkarray[5][62];

    char lan_out[256];
    static char convers_calls[50][6];
    static int ci, cl = 0, cc;
    static char spotline[160];
    static int m = 0, t = 0;
    static char dxtext[160];
    static char spottime[40];
    static float freq;
    static char *spotpointer;

    int i, l;

    l = strlen(tln_input_buffer);
    if (view_state == STATE_EDITING) {

	if (s[0] == '<' && s[1] != '<') {	// must be convers
	    for (ci = 0; ci <= cl; ci++) {
		if (strncmp(convers_calls[ci], s, 5) == 0)
		    break;
	    }

	    if (ci <= cl) {	// found
		cc = ci;
		while (cc > 5)
		    cc -= 5;
	    } else {
		if (cl < 48)
		    cl++;

		strncpy(convers_calls[cl], s, 5);
		cc = cl;

		while (cc > 5)
		    cc -= 5;
	    }
	    cc += 1;

	    if (use_rxvt == 0)
		wattron(sclwin, COLOR_PAIR(cc) | A_BOLD);
	    else
		wattron(sclwin, COLOR_PAIR(cc));

	}

	if (strncmp(s, "***", 3) == 0) {
	    cc = 1;
	    wattron(sclwin, COLOR_PAIR(cc));
	    cc = 0;
	}			// end convers

	wprintw(sclwin, "%s", s);

	wattroff(sclwin, A_BOLD);

	if (in_packetclient == 1)
	    wrefresh(sclwin);
    }
// Cluster private spotting interface

    if (strncmp(s, call, strlen(call) - 1) == 0
	&& strlen(s) < 81 && strchr(s, '>') == NULL) {

	mvprintw(24, 0,
		 "                                                                                ");

	if ((strlen(s) + strlen(call) + 3) < 80) {
	    strcpy(dxtext, s + strlen(call) + 3);
	    if (dxtext[strlen(dxtext) - 1] == '\n')
		dxtext[strlen(dxtext) - 1] = '\0';	// remove the newline
	    mvprintw(24, 0, dxtext);
	    mvprintw(12, 29, hiscall);
	}
	refreshp();

	spotpointer = strchr(dxtext, ':');

	if (spotpointer != NULL && strncmp(spotpointer, ": DX ", 5) == 0) {
	    spotline[0] = '\0';
	    s[0] = '\0';
	    strcat(spotline, "DX de ");
	    strncat(spotline, dxtext, spotpointer - dxtext);
	    strcat(spotline, ":                                ");
	    freq = atof(spotpointer + 5);
	    if (freq > 1800.0 && freq < 30000.0) {

		if (freq >= 100000.0) {
		    sprintf(spotline + 16, "%5.1f  ", freq);
		    sprintf(spotline + 26, "%s", spotpointer + 14);
		} else if (freq >= 10000.0) {
		    sprintf(spotline + 17, "%5.1f  ", freq);
		    sprintf(spotline + 26, "%s", spotpointer + 13);
		} else if (freq >= 1000.0) {
		    sprintf(spotline + 18, "%5.1f  ", freq);
		    sprintf(spotline + 26, "%s", spotpointer + 12);
		} else {
		    sprintf(spotline + 19, "%5.1f  ", freq);
		    sprintf(spotline + 26, "%s", spotpointer + 11);
		}

		for (m = 25; m < 38; m++) {
		    if (spotline[m] > 96)
			spotline[m] -= 32;
		}

		strcat(spotline,
		       "                                           ");
		get_time();
//                                              strftime(spottime, 80, "%H%MZ", time_ptr);      ### bug fix
		strftime(spottime, sizeof(spottime), "%H%MZ", time_ptr);
		strcpy(spotline + 70, spottime);
		spotline[75] = '\0';
		strcat(spotline, " <<\n");
		strcpy(s, spotline);
	    }
	} else {
	    for (t = 0; t < 4; t++)
		strcpy(talkarray[t], talkarray[t + 1]);
	    if (s[strlen(s) - 1] == '\n')
		s[strlen(s) - 1] = '\0';	// remove the newline
	    talkarray[4][0] = '\0';
	    strncat(talkarray[4], s + 8, 60);

	    if (s[strlen(s) - 1] != '\n')
		strcat(s, "\n");	// we need to restore newline here...

	}

    }
// end cluster private spotting interface

    for (i = 0; i < strlen(s); i++) {
	if (s[i] == '\n' || i + l + 1 >= COLS) {

	    addlog(tln_input_buffer);

	    if (lan_active == 1 && lanspotflg == 0) {
		if ((strlen(tln_input_buffer) > 0)
		    && (tln_input_buffer[0] > 32)
		    && (tln_input_buffer[0] < 126)) {
		    strncpy(lan_out, tln_input_buffer, 78);
		    strcat(lan_out, "\n");
		    if (strlen(s) == 0)
			tln_input_buffer[0] = '\0';
		    send_lan_message(CLUSTERMSG, lan_out);
		    lan_out[0] = '\0';
		}
	    }

	    l = strlen(tln_input_buffer);
	    l = -i - 1;
	} else {
	    if (s[i] != '\0')
		tln_input_buffer[i + l] = s[i];
	}

	tln_input_buffer[i + l + 1] = '\0';

    }
}

/* =========================================
=
=             This initializes the packet windows
=
===========================================*/

int init_packet(void)
{

    extern int prsock;
    extern int portnum;
    extern char pr_hostaddress[];
    extern SCREEN *packetscreen;
    extern WINDOW *sclwin;
    extern WINDOW *entwin;
    extern char spot_ptr[MAX_SPOTS][82];
    extern int tncport;
    extern int fdSertnc;
    extern int fdFIFO;
    extern int packetinterface;
    extern int tnc_serial_rate;
    extern int verbose;
    extern int use_rxvt;
    extern char tncportname[];

    struct termios termattribs;

    int addrarg;
    int iptr = 0;
    mode_t mode = 0666;
    static int initialized = 0;

    tln_input_buffer[0] = '\0';
    attr[NORMAL_ATTR] = A_NORMAL;
    if (use_rxvt == 0) {
	attr[MINE_ATTR] = A_BOLD;
	attr[ENTRY_ATTR] = A_BOLD;
    } else {
	attr[MINE_ATTR] = A_NORMAL;
	attr[ENTRY_ATTR] = A_NORMAL;

    }
    addrarg = 0;

    if (initialized == 0) {

	initialized = 1;
	packetscreen = newterm(NULL, stdout, stdin);
	start_color();

	init_pair(0, 0, 0);
	init_pair(1, 1, 0);
	init_pair(2, 2, 0);
	init_pair(3, 3, 0);
	init_pair(4, 4, 0);
	init_pair(5, 5, 0);
	init_pair(6, 6, 0);
	init_pair(7, 7, 0);

	sclwin = newwin(LINES - ENTRYROWS, COLS, 0, 0);
	entwin = newwin(ENTRYROWS, COLS, LINES - ENTRYROWS, 0);
	scrollok(sclwin, TRUE);
	scrollok(entwin, FALSE);
	keypad(entwin, TRUE);
	intrflush(entwin, FALSE);
	wattrset(sclwin, attr[NORMAL_ATTR]);
	wattrset(entwin, attr[ENTRY_ATTR]);

	noecho();
	cbreak();
	wtimeout(entwin, 30);
	wrefresh(sclwin);
	start_editing();
	curattr = attr[NORMAL_ATTR];
    }

    if (packetinterface == TELNET_INTERFACE) {

	wprintw(sclwin, "Trying %s:%d ... \n", pr_hostaddress, portnum);
	wrefresh(sclwin);

	if ((prsock = startcli()) < 0) {

	    wprintw(sclwin,
		    "\n\nconnect  failed... please check network connectivity !!\n");
	    wrefresh(sclwin);
	    sleep(2);

	    return (-1);
	} else {

	    wprintw(sclwin, "connected.\n");
	}
	socktimeout(30);

    } else if (packetinterface == TNC_INTERFACE) {

	if (strlen(tncportname) > 2) {
	    tncportname[strlen(tncportname) - 1] = '\0';	// remove '\n'
	    if ((fdSertnc = open(tncportname, O_RDWR | O_NONBLOCK)) < 0) {
		wprintw(sclwin, "open of %s failed!!!\n", tncportname);
		wrefresh(sclwin);
		sleep(2);
		return (-1);
	    }
	} else {
	    if (tncport == 1) {
		if ((fdSertnc =
		     open("/dev/ttyS0", O_RDWR | O_NONBLOCK)) < 0) {
		    wprintw(sclwin, "open of /dev/ttyS0 failed!!!\n");
		    wrefresh(sclwin);
		    sleep(2);
		    return (-1);
		}
	    } else if (tncport == 2) {

		if ((fdSertnc =
		     open("/dev/ttyS1", O_RDWR | O_NONBLOCK)) < 0) {
		    wprintw(sclwin, "open of /dev/ttyS1 failed!!!\n");
		    wrefresh(sclwin);
		    sleep(2);
		    return (-1);
		}
	    }
	}

	termattribs.c_iflag = IGNBRK | IGNPAR | IMAXBEL | IXOFF;
	termattribs.c_oflag = 0;
	termattribs.c_cflag = CS8 | CSTOPB | CREAD | CLOCAL;
	termattribs.c_lflag = 0;	/* Set some term flags */

	/*  The ensure there are no read timeouts (possibly writes?) */
	termattribs.c_cc[VMIN] = 1;
	termattribs.c_cc[VTIME] = 0;

	switch (tnc_serial_rate) {

	case 1200:{
		cfsetispeed(&termattribs, B1200);	/* Set input speed */
		cfsetospeed(&termattribs, B1200);	/* Set output speed */
		break;
	    }

	case 2400:{
		cfsetispeed(&termattribs, B2400);	/* Set input speed */
		cfsetospeed(&termattribs, B2400);	/* Set output speed */
		break;
	    }

	case 4800:{
		cfsetispeed(&termattribs, B4800);	/* Set input speed */
		cfsetospeed(&termattribs, B4800);	/* Set output speed */
		break;
	    }

	case 9600:{
		cfsetispeed(&termattribs, B9600);	/* Set input speed */
		cfsetospeed(&termattribs, B9600);	/* Set output speed */
		break;
	    }
	default:{

		cfsetispeed(&termattribs, B9600);	/* Set input speed */
		cfsetospeed(&termattribs, B9600);	/* Set output speed */
	    }
	}

	tcsetattr(fdSertnc, TCSANOW, &termattribs);	/* Set the serial port */

	wprintw(sclwin, "ttyS%d opened...\n", (tncport - 1));
	wrefresh(sclwin);
    } else if (packetinterface == FIFO_INTERFACE) {

	wprintw(sclwin, "Trying to open input FIFO \n");
	wrefresh(sclwin);

	if ((mkfifo("clfile", mode)) < 0) {
	    wprintw(sclwin, "FIFO clfile exists...\n");
	    wrefresh(sclwin);
	    if (verbose == 1)
		sleep(1);
	} else {
	    wprintw(sclwin, "FIFO clfile made\n");
	    wrefresh(sclwin);
	    if (verbose == 1)
		sleep(1);

	}

	if ((fdFIFO = open("clfile", O_RDONLY | O_NONBLOCK)) < 0) {
	    wprintw(sclwin, "Open FIFO failed\n");
	    wrefresh(sclwin);
	    sleep(1);
	} else {
	    wprintw(sclwin, "FIFO clfile open\n\n");
	    wrefresh(sclwin);
	    if (verbose == 1)
		sleep(1);

	}
    }

    wprintw(sclwin, "\n Use \":\" to go to tlf !! \n");
    wrefresh(sclwin);

    pthread_mutex_lock (&spot_ptr_mutex);
    for (iptr = 0; iptr < MAX_SPOTS; iptr++)
	spot_ptr[iptr][0] = '\0';
    pthread_mutex_unlock (&spot_ptr_mutex);

    return (0);
}

/* =========================================
=
=          Throw away the sockets and windows ...
=
=
===========================================*/

int cleanup_telnet(void)
{

    extern int prsock;
    extern WINDOW *entwin;
    extern WINDOW *sclwin;
    extern SCREEN *mainscreen;
    extern int packetinterface;
    extern int fdSertnc;
    extern int fdFIFO;

    if (packetinterface == TELNET_INTERFACE) {

	wattrset(entwin, A_NORMAL);
	wclear(entwin);
	wmove(entwin, 1, 0);
	wrefresh(entwin);
	vidattr(A_NORMAL);
	delwin(entwin);
	wclear(sclwin);
	wrefresh(sclwin);
	delwin(sclwin);

	endwin();

	if (prsock > 0)
	    close_s(prsock);

    } else if (packetinterface == TNC_INTERFACE) {

	if (fdSertnc > 0)
	    close(fdSertnc);

    } else if (packetinterface == FIFO_INTERFACE) {

	if (fdFIFO > 0)
	    close(fdFIFO);

	remove("./clfile");
    }

    set_term(mainscreen);

    return (0);
}

/* =========================================
=
=         Basic packet loop
=
=
===========================================*/

int packet()
{

    extern SCREEN *mainscreen;
    extern SCREEN *packetscreen;
    extern WINDOW *sclwin;
    extern WINDOW *entwin;
    extern int prsock;
    extern int fdSertnc;
    extern int fdFIFO;
    extern int packetinterface;
    extern int in_packetclient;
    extern char clusterlogin[];

    char line[BUFFERSIZE];

    int i = 0;
    int c, count;
    static int sent_login = 0;

    in_packetclient = 1;
    sleep(1);
    set_term(packetscreen);

    wclear(sclwin);
    wclear(entwin);

    count = 0;

    if ((tln_loglines == 0) && (packetinterface == TELNET_INTERFACE)) {
	addtext("Welcome to TLF telnet\n\n");
	if ((sent_login == 0) && (strlen(clusterlogin) > 0)
	    && (packetinterface == TELNET_INTERFACE) && (prsock > 0)) {
	    usputs(prsock, clusterlogin);
	    sent_login = 1;
	    addtext("logged into cluster...\n\n");
	    sleep(1);
	    endwin();
	    set_term(mainscreen);
	    clear();
	    clear_display();
	    in_packetclient = 0;
	    return (0);
	}

    }
    if ((tln_loglines == 0) && (packetinterface == TNC_INTERFACE))
	addtext("Welcome to TLF tnc terminal\n\n");

    while (1) {
	wrefresh(entwin);

	while ((c = wgetch(entwin)) == -1) {

	    if (packetinterface == TELNET_INTERFACE) {

		if (prsock > 0) {

		    line[0] = '\0';
		    i = 0;

		    i = recvline(&prsock, line, BUFFERSIZE - 1);

		} else {
		    wprintw(sclwin,
			    "There is no connection.... going back to tlf !!\n");
		    wrefresh(sclwin);
		    sleep(2);
		    i = -1;
		}

		if (i == -1) {
		    cleanup_telnet();
		    return (-1);

		} else if (i != -2) {
		    line[i] = '\0';
		    sanitize(line);
		    addtext(line);
		}

	    }

	    else if (packetinterface == TNC_INTERFACE) {

		if (fdSertnc > 0) {

		    i = read(fdSertnc, line, BUFFERSIZE - 1);

		    if (i > 0) {
			line[i] = '\0';
			sanitize(line);
			addtext(line);
		    }
		}
	    } else if (packetinterface == FIFO_INTERFACE) {

		if (fdFIFO > 0) {
		    i = read(fdFIFO, line, BUFFERSIZE - 1);

		    if (i > 0) {
			line[i] = '\0';
			sanitize(line);
			addtext(line);
		    }

		}
	    }
	}

	if (edit_line(c)) {
	    gather_input(line);

	    if (strcmp(line, ":exit") == 0) {
		endwin();
		exit(1);
	    }
	    if (strcmp(line, " :exit") == 0) {
		endwin();
		exit(1);
	    }
	    if (strcmp(line, ":") == 0)
		break;
	    if (strcmp(line, " :") == 0)
		break;

	    curattr = attr[MINE_ATTR];
	    wattrset(sclwin, curattr);
	    strcat(line, "\n");
	    addtext(line);

	    if ((packetinterface == TELNET_INTERFACE) && (prsock > 0)) {
		usputs(prsock, line);
	    }

	    if (packetinterface == TNC_INTERFACE) {
		int rc;
		line[strlen(line) - 1] = 13;
		line[strlen(line)] = '\0';
		rc = write(fdSertnc, line, strlen(line));
	    }

	    curattr = attr[NORMAL_ATTR];
	    wattrset(sclwin, curattr);
	    wrefresh(sclwin);
	    start_editing();
	}
    }
    endwin();
    set_term(mainscreen);
    clear();
    clear_display();

    in_packetclient = 0;

    return (0);
}

/* ======================================================

					Receive info from  cluster

========================================================
*/

int receive_packet(void)
{

    extern int prsock;
    extern int packetinterface;
    extern int fdSertnc;
    extern int fdFIFO;
    extern int in_packetclient;

    char line[BUFFERSIZE];
    int i = 0;

    if (in_packetclient == 1)
	return (0);

    if (packetinterface == TELNET_INTERFACE) {
	if (prsock > 0) {
	    i = recvline(&prsock, line, BUFFERSIZE - 1);

	    view_state = STATE_EDITING;
	    if (i == -1) {
		cleanup_telnet();
		return (-1);
	    } else if (i != -2) {
		line[i] = '\0';
		sanitize(line);
		addtext(line);

	    }
	}
    } else if (packetinterface == TNC_INTERFACE) {

	if (fdSertnc > 0) {

	    i = read(fdSertnc, line, BUFFERSIZE - 1);

	    if (i > 0) {
		line[i] = '\0';
		sanitize(line);
		addtext(line);

	    }
	}
    } else if (packetinterface == FIFO_INTERFACE) {

	if (fdFIFO > 0) {

	    i = read(fdFIFO, line, BUFFERSIZE - 1);

	    if (i > 0) {
		line[i] = '\0';
		sanitize(line);
		addtext(line);
	    }

	}
    }

    return (0);

}

/* ======================================================

					  send command to cluster

========================================================
*/
#define MAX_CMD_LEN 60

int send_cluster(void)
{
    extern int fdSertnc;
    extern int packetinterface;
    extern int prsock;
    extern int cluster;
    extern int showscore_flag;

    char line[MAX_CMD_LEN+2] = "";

    cluster = CLUSTER;
    showscore_flag = 0;
    mvprintw(24, 0,
	     "                                                                           ");
    mvprintw(24, 0, ">");
    refreshp();
    echo();
    getnstr(line, MAX_CMD_LEN);
    noecho();

    if (strlen(line) > 0) {
	strcat(line, "\n");

	if (packetinterface == TNC_INTERFACE) {
	    int rc;
	    line[strlen(line) - 1] = '\r';
	    line[strlen(line)] = '\0';	/* not needed */

	    rc = write(fdSertnc, line, strlen(line));
	}
	else if ((packetinterface == TELNET_INTERFACE) && (prsock > 0))
	    usputs(prsock, line);
    }

    attron(COLOR_PAIR(C_HEADER) | A_STANDOUT);

    mvprintw(24, 0,
	     "                                                                           ");
    refreshp();
    line[0] = '\0';	/* not needed */

    return (0);
}
