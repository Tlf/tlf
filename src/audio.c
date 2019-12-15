/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 *                                   2012 Thomas Beierlein <tb@forth-ev.de>
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
 *      audio.c   soundcard input routine
 *
 *--------------------------------------------------------------*/


#include <dirent.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ignore_unused.h"
#include "keystroke_names.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"


extern char sc_device[];

/* -------------------main test routine ------- */
void recordmenue(void) {
    int j;

    attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (j = 0; j <= 24; j++)
	mvprintw(j, 0,
		 "                                                                                ");

    mvprintw(1, 20, "--- TLF SOUND RECORDER UTILITY ---");
    mvprintw(6, 20, "F1 ... F12, S, C: Record Messages");

    mvprintw(9, 20, "1.: Enable contest recorder");
    mvprintw(10, 20, "2.: Disable contest recorder");
    mvprintw(11, 20, "3.: Play back file");
    mvprintw(13, 20, "ESC: Exit sound recorder function");

    refreshp();

}

/*--------------------------------------------------------------------------*/
void do_record(int message_nr) {

    extern char ph_message[14][80];

    char commands[80] = "";

    mvprintw(15, 20, "recording %s", ph_message[message_nr]);
    mvprintw(16, 20, "ESC to exit");
    mvprintw(17, 20, "");
    refreshp();
    strcpy(commands, "rec -r 8000 ");	//G4KNO
    strcat(commands, ph_message[message_nr]);
    strcat(commands, " -q &");	//G4KNO
    IGNORE(system(commands));;
    //G4KNO: Loop until <esc> keypress
    while (1) {
	if (key_get() == ESCAPE) {
	    //kill process (SIGINT=Ctrl-C).
	    IGNORE(system("pkill -SIGINT -n rec"));;
	    break;
	}
    }
}

/*--------------------------------------------------------------------------*/
void record(void) {

    extern char ph_message[14][80];

    int runnit = 1, key, i = 0, j = 4;
    char commands[80] = "";
    char playbackfile[40];
    char printname[7];
    DIR *sounddir;
    struct dirent *soundfilename;

    recordmenue();

    while (runnit == 1) {

	key = key_poll();

	/* Look for F1-F12, s|S, c|C, 1-4 */
	switch (key) {

	    /* Record voice keyer messages, F1-F12, s|S, c|C. */
	    case KEY_F(1):
		do_record(0);
		runnit = 0;
		break;
	    case KEY_F(2):
		do_record(1);
		runnit = 0;
		break;
	    case KEY_F(3):
		do_record(2);
		runnit = 0;
		break;
	    case KEY_F(4):
		do_record(3);
		runnit = 0;
		break;
	    case KEY_F(5):
		do_record(4);
		runnit = 0;
		break;
	    case KEY_F(6):
		do_record(5);
		runnit = 0;
		break;
	    case KEY_F(7):
		do_record(6);
		runnit = 0;
		break;
	    case KEY_F(8):
		do_record(7);
		runnit = 0;
		break;
	    case KEY_F(9):
		do_record(8);
		runnit = 0;
		break;
	    case KEY_F(10):
		do_record(9);
		runnit = 0;
		break;
	    case KEY_F(11):
		do_record(10);
		runnit = 0;
		break;
	    case KEY_F(12):
		do_record(11);
		runnit = 0;
		break;
	    case 's':
	    case 'S':
		do_record(12);
		runnit = 0;
		break;
	    case 'c':
	    case 'C':
		do_record(13);
		runnit = 0;
		break;

	    /* Contest recording and playback. */

	    // Start contest recording.
	    case '1':
		IGNORE(system("echo " " > ~/.VRlock"));;

		IGNORE(system
		       ("cd ~/tlf/soundlogs; ./soundlog  > /dev/null 2> /dev/null &"));

		mvprintw(15, 20, "Contest recording enabled...");
		refreshp();
		sleep(1);
		runnit = 0;
		break;

	    // Stop contest recording.
	    case '2':
		mvprintw(15, 20, "Contest recording disabled...");
		refreshp();
		sleep(1);
		IGNORE(system("rm ~/.VRlock"));;
		IGNORE(system("pkill -f soundlogs > /dev/null 2> /dev/null "));;
		runnit = 0;
		break;

	    // List contest recordings.
	    case '3':
		sounddir = opendir("$HOME/tlf/soundlogs/");	// (W9WI)

		if (sounddir == NULL)
		    break;

		for (i = 4; i < 15; i++)
		    mvprintw(i, 0,
			     "                                                                                ");

		mvprintw(4, 10, "");

		for (i = 10; i < 81; i += 10) {
		    soundfilename = readdir(sounddir);
		    if (soundfilename == NULL)
			break;
		    else {
			if (strstr(soundfilename->d_name, ".au") != NULL) {
			    if (i > 60) {
				i = 10;
				j++;
			    }
			    g_strlcpy(printname, soundfilename->d_name, 7);
			    mvprintw(j, i, "%s", printname);
			    refreshp();

			} else if (i >= 10)
			    i -= 10;
		    }
		}
		closedir(sounddir);

	    // Play back contest recording.
	    case '4':
		mvprintw(15, 20, "Play back file (ddhhmmxx): ");
		refreshp();

		echo();
		getnstr(playbackfile, 8);
		noecho();
		strcpy(commands, "play -d ");
		strcat(commands, sc_device);
		strcat(commands, " ~/tlf/soundlogs/");
		if (strlen(playbackfile) > 6) {
		    strncat(commands, playbackfile, 6);
		    strcat(commands, ".au trim ");
		    strcat(commands, playbackfile + 6);
		} else if (strlen(playbackfile) < 5) {
		    strcat(commands, playbackfile);
		    strcat(commands, "00.au");
		} else {
		    strcat(commands, playbackfile);
		    strcat(commands, ".au");
		}
		mvprintw(16, 20, "Use Ctrl-c to stop and return to tlf");
		mvprintw(18, 20, "");
		refreshp();
		IGNORE(system(commands));;
		runnit = 0;
		break;
	    case ESCAPE:
		runnit = 0;
	}
    }
}
