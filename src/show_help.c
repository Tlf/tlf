/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Thomas Beierlein <tb@forth-ev.de>
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
	/*
	 * ------------------------------------------------------------ *
	 * Show help file *
	 * *--------------------------------------------------------------
	 */


#include <stdlib.h>
#include <unistd.h>

#include <glib/gstdio.h>

#include "clear_display.h"
#include "tlf_curses.h"

#define new_help 	/* new implementation */
#ifdef new_help

extern SCREEN *mainscreen;

/** \brief Show help file
 *
 * Read 'help.txt' and display it via 'less'.
 * Local 'help.txt' in actual directory overrides default file
 * in PKG_DATA_DIR
 */
int show_help(void) {
    int rc;
    char filename[] = "help.txt";
    char *helpfile;
    char *cmdstr;

    if (g_access(filename, R_OK) == 0) {
	helpfile = filename;
    } else {
	helpfile = g_strconcat(PACKAGE_DATA_DIR, G_DIR_SEPARATOR_S,
		filename, NULL);
	if (g_access(helpfile, R_OK) != 0) {
	    g_free(helpfile);
	    return -1;
	}
    }

    cmdstr = g_strdup_printf("less %s", helpfile);

    endwin();
    rc=system("clear");
    rc=system(cmdstr);
    rc=system("clear");

    g_free(helpfile);
    g_free(cmdstr);

    set_term(mainscreen);
    clear_display();
    return 0;
}

#else 			/* keep old implementation for now */
int
show_help(void)
{
    extern char     backgrnd_str[];

    char            helpinfo[9][85];
    int             i,
                    j;
    char            printbuffer[160];

    strcpy(helpinfo[0],
	   "SPOT    : Last spots        TONE   : Sidetone(0=OFF)   +       : CQ/S&P             ");
    strcpy(helpinfo[1],
	   "MAP     : Band spots        EDIT   : Edit Log          -       : Delete last qso    ");
    strcpy(helpinfo[2],
	   "CLOFF   : Cluster OFF       VIEW   : View  Log         ;       : Include note in log");
    strcpy(helpinfo[3],
	   "CLUSTER : Cluster info      Ctl-k ,: Keyboard          SPACE   : Check dupe         ");
    strcpy(helpinfo[4],
	   "SHORT   : Short exchange    TAB    : CALL/EXCHANGE .   DEMODE  : Toggle DE          ");
    strcpy(helpinfo[5],
	   "LONG    : Long Exchange     ESCAPE : Leave mode        CONTEST : Contest mode       ");
    strcpy(helpinfo[6],
	   "MESSAGE : Change Messages   F1-F11 : Play Message      FILTER  : Filter ON/OFF      ");
    strcpy(helpinfo[7],
	   "LIST    : List Messages     F12    : Auto CQ           SCORE   : Score window       ");
    strcpy(helpinfo[8],
	   "CHECK   : Call Check ON     NOCHECK: Call Check OFF    _ (Und.): Send prev. nr.     ");

    nicebox(13, 0, 9, 78, "Help(1)");

    for (i = 0; i <= 8; i++) {
	printbuffer[0] = '\0';
	strncat(printbuffer, helpinfo[i], 78);
	strncat(printbuffer, backgrnd_str, 4);
	printbuffer[78] = '\0';
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	mvprintw(i + 14, 1, "%s", printbuffer);
    }
    mvvline(14, 27, ACS_VLINE, 9);
    mvvline(14, 54, ACS_VLINE, 9);
    refreshp();

    attroff(A_STANDOUT);
    mvprintw(23, 30, "Press any key for more");

    (void)key_get();

    strcpy(helpinfo[0],
	   "MESSAGE : Change Messages   F1-F11 : Play Message      FILTER  : Filter ON/OFF      ");
    strcpy(helpinfo[1],
	   "LIST    : List Messages     F12    : Auto CQ           SCORE   : Score window       ");
    strcpy(helpinfo[2],
	   "CHECK   : Call Check ON     NOCHECK: Call Check OFF    _ (Und.): Send prev. nr.     ");
    strcpy(helpinfo[3],
	   ",       : keyboard on       @      : Edit last qso     =       : Cfm last call      ");
    strcpy(helpinfo[4],
	   "\\       : log qso w/o CW    ZONE   : Show Zones        CTY     : Show Countries     ");
    strcpy(helpinfo[5],
	   "TRX     : trxcontrol on/off RIT    : RIT clear on/off  Ctrl-g  : Grab DX spot       ");
    strcpy(helpinfo[6],
	   "14011   : fast qsy          #      : TRX -> MEM        Ctrl-p  : Show Propagation   ");
    strcpy(helpinfo[7],
	   "CWMODE  : CW                SSB    : SSB               DIGIMODE: Digital modes      ");
    strcpy(helpinfo[8],
	   "MODE    : CW/SSB/DIG        EXIT   : exit program      SET     : Set parameters     ");

    nicebox(13, 0, 9, 78, "Help(2)");

    for (i = 0; i <= 8; i++) {
	printbuffer[0] = '\0';
	strncat(printbuffer, helpinfo[i], 78);
	strncat(printbuffer, backgrnd_str, 4);
	printbuffer[78] = '\0';
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	mvprintw(i + 14, 1, "%s", printbuffer);
    }

    mvvline(14, 27, ACS_VLINE, 9);
    mvvline(14, 54, ACS_VLINE, 9);
    refreshp();

    attroff(A_STANDOUT);
    mvprintw(23, 30, "Press any key for more");
    (void)key_get();

    strcpy(helpinfo[0],
	   "                        ==== CT compatible commands ====                            ");
    strcpy(helpinfo[1],
	   "Alt-A   : Announce window   Alt-C  : Countries         Alt-G   : Talk (GAB)         ");
    strcpy(helpinfo[2],
	   "Alt-H   : Help              Alt-I  : Talk window       Alt-J   : TX freq. window    ");
    strcpy(helpinfo[3],
	   "Alt-K   : Keyboard on/off   Alt-M  : Multipliers       Alt-N   : Include note in log");
    strcpy(helpinfo[4],
	   "Alt-Q   : Quit tlf          Alt-R  : Rate display      Alt-S   : Summary display    ");
    strcpy(helpinfo[5],
	   "Alt-V   : Vary CW speed     Alt-X  : Exit tlf          Alt-Z   : Zones Display      ");
    strcpy(helpinfo[6],
	   "Alt-,   : Bandmap           Insert : Send F2           +       : Send F3 + Log ");
    strcpy(helpinfo[7],
	   "F1      : Send CQ           F2     : Send Exchange     F3      : Send QRZ msg ");
    strcpy(helpinfo[8],
	   "F4      : MyCall            F5     : HisCall           F6      : CL?     ");

    nicebox(13, 0, 9, 78, "Help(3, CT cmds)");

    for (i = 0; i <= 8; i++) {
	printbuffer[0] = '\0';
	strncat(printbuffer, helpinfo[i], 78);
	strncat(printbuffer, backgrnd_str, 4);
	printbuffer[78] = '\0';
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT);
	mvprintw(i + 14, 1, "%s", printbuffer);
    }
    mvvline(14, 27, ACS_VLINE, 9);
    mvvline(14, 54, ACS_VLINE, 9);
    refreshp();

    attroff(A_STANDOUT);
    mvprintw(23, 30, "Press any key to return to tlf");

    (void)key_get();
    clear_display();
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

    for (j = 13; j <= 23; j++) {
	mvprintw(j, 0, backgrnd_str);
    }
    refreshp();

    return (0);
}
#endif
