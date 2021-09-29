/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 *                              2012-2021 Thomas Beierlein <tb@forth-ev.de>
 *                                   2021 Nate Bargman <n0nb@n0nb.us>
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
 *      audio.c   recording and playing audio files
 *
 *--------------------------------------------------------------*/


#include <dirent.h>
#include <errno.h>
#include <glib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "err_utils.h"
#include "globalvars.h"
#include "ignore_unused.h"
#include "keystroke_names.h"
#include "netkeyer.h"
#include "time_update.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"


static pthread_t vk_thread;
static atomic_bool vk_running = false;


void recordmenue(void) {
    int j;

    attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (j = 0; j <= 24; j++)
	mvprintw(j, 0, backgrnd_str);

    mvprintw(1, 20, "--- TLF SOUND RECORDER UTILITY ---");
    mvprintw(6, 20, "F1 ... F12, S, C: Record Messages");

    mvprintw(9, 20, "1.: Enable contest recorder");
    mvprintw(10, 20, "2.: Disable contest recorder");
    mvprintw(11, 20, "3.: List and Play contest file");
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
		       ("cd ~/tlf/soundlogs; soundlog  > /dev/null 2> /dev/null &"));

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
		errno = 0;

		/* Must query the environment for the value of $HOME
		 * and build the path to the soundlogs.
		 */
		char *path = g_strdup_printf("%s%s",
					     g_getenv("HOME"),
					     "/tlf/soundlogs");

		sounddir = opendir(path);

		if (sounddir == NULL) {
		    if (errno != 0) {
			mvprintw(22, 1, "%s: %s", strerror(errno), path);
			mvprintw(23, 1, "Press ESC to exit this screen");
		    }
		    break;
		}

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
		g_free(path);
		closedir(sounddir);

	    // Play back contest recording.
	    case '4':
		mvprintw(15, 20, "Play back file (ddhhmmxx): ");
		refreshp();

		echo();
		getnstr(playbackfile, 8);
		noecho();
		strcpy(commands, "play -q ");
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


/* playing recorded voice keyer messages */
void *play_thread(void *ptr) {
    char *audiofile = (char *)ptr;

    pthread_detach(pthread_self());

    vk_running=true;

    // use play_vk from current dir, if available
    // note: this overrides PATH setting
    bool has_local_play_vk = (access("./play_vk", X_OK) == 0);
    char *playcommand = g_strdup_printf("%s %s",
			(has_local_play_vk ? "./play_vk" : "play_vk"),
				audiofile);

    /* CAT PTT wanted and available, use it. */
    if (rigptt == CAT_PTT_USE) {
	/* Request PTT On */
	rigptt |= CAT_PTT_ON;
    } else {		/* Fall back to netkeyer interface */
	netkeyer(K_PTT, "1");	// ptt on
    }

    usleep(txdelay * 1000);
    IGNORE(system(playcommand));;
    g_free(playcommand);

    /* CAT PTT wanted, available, and active. */
    if (rigptt == (CAT_PTT_USE | CAT_PTT_ACTIVE)) {
	/* Request PTT Off */
	rigptt |= CAT_PTT_OFF;
    } else {		/* Fall back to netkeyer interface */
	netkeyer(K_PTT, "0");	// ptt off
    }

    vk_running= false;

    return NULL;
}

void play_file(char *audiofile) {

    /* do not play another message as long as the old one is still running */
    if (vk_running) {
	return;
    }

    if (audiofile == NULL || *audiofile == 0) {
	return;
    }

    if (access(audiofile, R_OK) != 0) {
	TLF_LOG_INFO("cannot open sound file %s!", audiofile);
	return;
    }

    /* play sound in separate thread so it can be killed from the main one */
    if (pthread_create(&vk_thread, NULL, play_thread, (void *)audiofile) != 0) {
	    TLF_LOG_INFO("could not start sound thread!");
    }
}

#define NO_KEY -1

void stop_vk() {
    IGNORE(system("pkill -SIGTERM -n play_vk"));
}


/* wait till VK message is finished or key pressed */
int wait_vk_finish() {
    while (1) {
	usleep(100000);
	time_update();
	if (vk_running == false) {
	    return NO_KEY;
	}
	const int inchar = key_poll();
	if (inchar > 0 && inchar != KEY_RESIZE) {
	    return inchar;
	}
    }
}


