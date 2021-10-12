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

char *vk_record_cmd;
char *vk_play_cmd;
char *soundlog_record_cmd;
char *soundlog_play_cmd;
char *soundlog_dir;


void vk_do_record(int message_nr);
void vr_start(void);
void vr_stop(void);


void sound_setup_default(void) {
    if (vk_record_cmd) g_free (vk_record_cmd);
    vk_record_cmd = g_strdup("rec -r 8000 $1 -q");

    if (vk_play_cmd) g_free (vk_play_cmd);
    vk_play_cmd = g_strdup("play_vk $1");

    if (soundlog_record_cmd) g_free (soundlog_record_cmd);
    soundlog_record_cmd = g_strdup("soundlog");

    if (soundlog_play_cmd) g_free (soundlog_play_cmd);
    soundlog_play_cmd = g_strdup("play -q ~/tlf/soundlogs/$1 trim $2");

    if (soundlog_dir) g_free (soundlog_dir);
    soundlog_dir = g_strdup("~/tlf/soundlogs");
}


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
		vk_do_record(0);
		runnit = 0;
		break;
	    case KEY_F(2):
		vk_do_record(1);
		runnit = 0;
		break;
	    case KEY_F(3):
		vk_do_record(2);
		runnit = 0;
		break;
	    case KEY_F(4):
		vk_do_record(3);
		runnit = 0;
		break;
	    case KEY_F(5):
		vk_do_record(4);
		runnit = 0;
		break;
	    case KEY_F(6):
		vk_do_record(5);
		runnit = 0;
		break;
	    case KEY_F(7):
		vk_do_record(6);
		runnit = 0;
		break;
	    case KEY_F(8):
		vk_do_record(7);
		runnit = 0;
		break;
	    case KEY_F(9):
		vk_do_record(8);
		runnit = 0;
		break;
	    case KEY_F(10):
		vk_do_record(9);
		runnit = 0;
		break;
	    case KEY_F(11):
		vk_do_record(10);
		runnit = 0;
		break;
	    case KEY_F(12):
		vk_do_record(11);
		runnit = 0;
		break;
	    case 's':
	    case 'S':
		vk_do_record(12);
		runnit = 0;
		break;
	    case 'c':
	    case 'C':
		vk_do_record(13);
		runnit = 0;
		break;

	    /* Contest recording and playback. */

	    // Start contest recording.
	    case '1':
		vr_start();

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

		vr_stop();

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
		IGNORE(system(commands));
		runnit = 0;
		break;
	    case ESCAPE:
		runnit = 0;
	}
    }
}


/* voice recorder handling - recording and play back */
void vr_start(void) {
    IGNORE(system("echo " " > ~/.VRlock"));

    char *command = g_strconcat("cd ", soundlog_dir, "; ",
	    soundlog_record_cmd, " >/dev/null 2>/dev/null &", NULL);

    IGNORE(system(command));
    g_free(command);
}


void vr_stop() {
    IGNORE(system("rm ~/.VRlock"));

    gchar **vector = g_strsplit_set(soundlog_record_cmd, " \t", 2);
    char *command = g_strconcat("pkill -SIGTERM -n ", vector[0], NULL);
    g_strfreev(vector);

    IGNORE(system(command));
    g_free(command);
}

/* voice keyer handling - recording and playback */
/*--------------------------------------------------------------------------*/
void vk_do_record(int message_nr) {

    extern char ph_message[14][80];

    mvprintw(15, 20, "recording %s", ph_message[message_nr]);
    mvprintw(16, 20, "ESC to exit");
    mvprintw(17, 20, "");
    refreshp();

    GRegex *regex = g_regex_new("\\$1", 0, 0 , NULL);
    char *command = g_regex_replace(regex, vk_record_cmd, -1, 0,
	    ph_message[message_nr], 0, NULL);
    g_regex_unref(regex);

    /* let the command run in background so we can stop recording by
     * <esc> key later */
    char *reccommand = g_strconcat( command, " &", NULL);

    IGNORE(system(reccommand));
    g_free(command);
    g_free(reccommand);

    /* Loop until <esc> key pressed */
    while (1) {
	if (key_get() == ESCAPE) {
	    /* kill process (first record command token) with SIGINT=Ctrl-C */
	    gchar **vector = g_strsplit_set(vk_record_cmd, " \t", 2);
	    char *command = g_strconcat("pkill -SIGINT -n ", vector[0], NULL);
	    g_strfreev(vector);

	    IGNORE(system(command));
	    g_free(command);
	    break;
	}
    }
}


/* playing recorded voice keyer messages in background */
void *play_thread(void *ptr) {
    char *audiofile = (char *)ptr;

    pthread_detach(pthread_self());

    vk_running=true;

    GRegex *regex = g_regex_new("\\$1", 0, 0, NULL);
    char *playcommand = g_regex_replace(regex, vk_play_cmd, -1, 0,
	    audiofile, 0, NULL);
    g_regex_unref(regex);

    /* CAT PTT wanted and available, use it. */
    if (rigptt == CAT_PTT_USE) {
	/* Request PTT On */
	rigptt |= CAT_PTT_ON;
    } else {		/* Fall back to netkeyer interface */
	netkeyer(K_PTT, "1");	// ptt on
    }

    usleep(txdelay * 1000);
    IGNORE(system(playcommand));
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

/* start playing voice keyer message file */
void vk_play_file(char *audiofile) {

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

void vk_stop() {
    gchar **vector = g_strsplit(vk_play_cmd, " \t", 2);
    char *command = g_strconcat("pkill -SIGTERM -n ", vector[0], NULL);
    g_strfreev(vector);

    IGNORE(system(command));
    g_free(command);
}


/* wait till VK message is finished or key pressed */
int vk_wait_finish() {
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


