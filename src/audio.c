/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 *                              2012-2022 Thomas Beierlein <tb@forth-ev.de>
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
#include <fnmatch.h>
#include <glib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <wordexp.h>

#include "clear_display.h"
#include "err_utils.h"
#include "globalvars.h"
#include "ignore_unused.h"
#include "keystroke_names.h"
#include "libgen.h"
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

static int sr_listfiles();

static char *expand_directory(const char *dir);
static char *prepare_playback_command(char *filename);
static void stop_command(char *string);
static void vk_do_record(int message_nr);
static void sr_start(void);
void sr_stop();

void sound_setup_default(void) {
    if (vk_record_cmd) g_free (vk_record_cmd);
    vk_record_cmd = g_strdup("rec -r 8000 $1 -q");

    if (vk_play_cmd) g_free (vk_play_cmd);
    vk_play_cmd = g_strdup("play_vk $1");

    if (soundlog_record_cmd) g_free (soundlog_record_cmd);
    soundlog_record_cmd = g_strdup("soundlog");

    if (soundlog_play_cmd) g_free (soundlog_play_cmd);
    soundlog_play_cmd = g_strdup("play -q $1 2> /dev/null");

    if (soundlog_dir) g_free (soundlog_dir);
    soundlog_dir = g_strdup("./soundlogs");
}


/* check if Sound Recorder got started and is still running
 *
 * For simplicity check if ~/.VRlock is present. It gets created during start
 * of SR and removed when recording gets stopped
 */
bool is_sr_running() {
    char *lockfile = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S,
		".VRlock", NULL);
    bool exists = (access(lockfile, F_OK) == 0);
    g_free(lockfile);
    return exists;
}


static void recordmenue(void) {
    int j;

    attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (j = 0; j <= LINES - 1; j++)
	clear_line(j);

    mvaddstr(1, 20, "--- TLF SOUND RECORDER UTILITY ---");
    mvaddstr(6, 20, "F1 ... F12, S, C: Record Messages");

    mvprintw(9, 20, "1; %s contest recorder",
	    is_sr_running() ? "Stop" : "Start");

    mvaddstr(10, 20, "2: List and Play contest file");
    mvaddstr(12, 20, "ESC: Exit sound recorder function");

    refreshp();

}


/*--------------------------------------------------------------------------*/
void record(void) {

    int key;
    bool run = true;
    char playbackfile[40];

    recordmenue();

    while (run) {

	key = key_poll();

	/* Look for F1-F12, s|S, c|C, 1-4 */
	switch (key) {

	    /* Record voice keyer messages, F1-F12, s|S, c|C. */
	    case KEY_F(1):
		vk_do_record(0);
		run = false;
		break;
	    case KEY_F(2):
		vk_do_record(1);
		run = false;
		break;
	    case KEY_F(3):
		vk_do_record(2);
		run = false;
		break;
	    case KEY_F(4):
		vk_do_record(3);
		run = false;
		break;
	    case KEY_F(5):
		vk_do_record(4);
		run = false;
		break;
	    case KEY_F(6):
		vk_do_record(5);
		run = false;
		break;
	    case KEY_F(7):
		vk_do_record(6);
		run = false;
		break;
	    case KEY_F(8):
		vk_do_record(7);
		run = false;
		break;
	    case KEY_F(9):
		vk_do_record(8);
		run = false;
		break;
	    case KEY_F(10):
		vk_do_record(9);
		run = false;
		break;
	    case KEY_F(11):
		vk_do_record(10);
		run = false;
		break;
	    case KEY_F(12):
		vk_do_record(11);
		run = false;
		break;
	    case 's':
	    case 'S':
		vk_do_record(SP_TU_MSG);
		run = false;
		break;
	    case 'c':
	    case 'C':
		vk_do_record(CQ_TU_MSG);
		run = false;
		break;

	    /* Contest recording and playback. */

	    // Start/Stop contest recording.
	    case '1':
		if (is_sr_running()) {
		    sr_stop();

		    mvaddstr(15, 20, "Contest recording stopped...");
		    refreshp();
		    sleep(1);
		} else {
		    sr_start();

		    mvaddstr(15, 20, "Contest recording started...");
		    refreshp();
		    sleep(1);
		}
		run = false;
		break;


	    case '2':
		// List contest recordings
		if (sr_listfiles() == -1) {
		    mvprintw(LINES - 1, 1, "Press ESC to exit this screen");
		    break;
		}

		// and play play back one recorded file.
		mvprintw(17, 20, "Play back file (ddhhmm): ");
		refreshp();

		echo();
		getnstr(playbackfile, 8);
		noecho();

		char *command = prepare_playback_command(playbackfile);

		mvprintw(18, 20, "Use Ctrl-C to stop and return to TLF");
		refreshp();

		IGNORE(system(command));
		g_free(command);

		run = false;
		break;

	    case ESCAPE:
		run = false;
	}
    }
}

/* common tools */

/* helper function to filter file by valid soundmode ending */
static int is_soundfile(const struct dirent *entry) {
    return !fnmatch("*.au", entry->d_name, 0);
}

/* expand ~ character for home directory if present in dir */
static char *expand_directory(const char *dir) {
    wordexp_t p;
    char **w;
    char *expanded;

    wordexp(dir, &p, 0);

    w = p.we_wordv;
    expanded = g_strdup(w[0]);

    wordfree(&p);

    return expanded;
}

/* strip audio file suffix */
static char* strip_suffix(char * filename) {
    GRegex *regex = g_regex_new("\\.au$", 0, 0, NULL);
    char *stripped_name = g_regex_replace(regex, filename, -1 , 0,
	    "", 0, NULL);
    g_regex_unref(regex);
    return stripped_name;
}

/* show list of audio file from soundlog directory */
static int sr_listfiles() {
    struct dirent **nameList;
    char *expanded_dir = expand_directory(soundlog_dir);

    int n = scandir(expanded_dir, &nameList,
		    is_soundfile, alphasort);

    if (n == -1) {
	mvprintw(LINES - 2, 1, "%s: %s", strerror(errno), expanded_dir);
	g_free(expanded_dir);
	return -1;
    }

    g_free(expanded_dir);

    for (int i = 4; i < 15; i++)
	clear_line(i);


    if (n > 48) n = 48;	    /* limit number of files to display */

    int i = 10;
    int j = 4;
    for (int k = 0; k < n; k ++) {

	char *printname = strip_suffix(nameList[k] -> d_name);
	mvprintw(j, i, "%s", printname);
	g_free(printname);

	i += 10;
    	if (i > 60) {
	    i = 10;
	    j++;
	}
    }
    refreshp();

    return 0;
}


/* kill process (first command token in string) with SIGTERM */
static void stop_command(char *string) {
    gchar **vector = g_strsplit_set(string, " \t", 2);
    char *program = basename(vector[0]);
    char *command = g_strconcat("pkill -SIGTERM -n ", program, NULL);
    g_strfreev(vector);

    IGNORE(system(command));
    g_free(command);
}


/* voice recorder handling - recording and play back */
static void sr_start(void) {
    IGNORE(system("echo " " > ~/.VRlock"));

    char *command = g_strconcat("mkdir -p ", soundlog_dir, "; ",
	    "cd ", soundlog_dir, "; ",
	    soundlog_record_cmd, " >/dev/null 2>/dev/null &", NULL);

    IGNORE(system(command));
    g_free(command);
}


void sr_stop() {
    IGNORE(system("rm ~/.VRlock"));
    stop_command(soundlog_record_cmd);
}


/* substitute filename with appended file type suffix in play command and
 * prepare for running the command in the soundlog directory
 */
static char *prepare_playback_command(char *filename) {
    char *file = g_strconcat(filename, ".au", NULL);

    GRegex *regex = g_regex_new("\\$1", 0, 0 , NULL);
    char *play_command = g_regex_replace(regex, soundlog_play_cmd, -1, 0,
	    file, 0, NULL);
	g_regex_unref(regex);
    g_free(file);

    char *full_command = g_strconcat("cd ", soundlog_dir, "; ",
		play_command, NULL);
    g_free (play_command);

    return full_command;
}

/* voice keyer handling - recording and playback */
/*--------------------------------------------------------------------------*/
static void vk_do_record(int message_nr) {

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
	    stop_command(vk_record_cmd);
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
    g_free(ptr);

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

    gchar *file = g_strdup(audiofile);	/* has to be freed in play_thread */

    /* play sound in separate thread so it can be killed from the main one */
    if (pthread_create(&vk_thread, NULL, play_thread, (void *)file) != 0) {
	    g_free(file);
	    TLF_LOG_INFO("could not start sound thread!");
    }
}


void vk_stop() {
    stop_command(vk_play_cmd);
}


/* check if playing VK message is finished */
bool is_vk_finished() {
	return (vk_running == false);
}


