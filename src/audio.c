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


#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "audio.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "gettxinfo.h"

#ifdef __OpenBSD__
# include <soundcard.h>
#else
# include <sys/soundcard.h>
#endif


extern char sc_device[];

int afd;

#define ABUFSIZE          4000

void init_audio() {

    char afile[40];		/* Audio device name */
    int sndfmt;			/* Encoding of audio from */
    int channels;		/* Number of channels to record */
    int speed;			/* Sample rate of audio */

    sndfmt = AFMT_MU_LAW;
    channels = 1;
    speed = 8000;

    /* Audio device setup */

    strcpy(afile, sc_device);

    if ((afd = open(afile, O_RDONLY, 0)) == -1) {
	perror(afile);
	exit(errno);
    }

    if (ioctl(afd, SNDCTL_DSP_SETFMT, &sndfmt) == -1) {
	perror("mu law");
	exit(errno);
    }

    if (ioctl(afd, SNDCTL_DSP_CHANNELS, &channels) == -1) {
	perror("mono");
	exit(errno);
    }

    if (ioctl(afd, SNDCTL_DSP_SPEED, &speed) == -1) {
	perror("8000 sps");
	exit(errno);
    }
}

int close_audio() {

    close(afd);

    return (0);
}

/* ------------ rescale bar ----------*/

int rescale(int testvalue) {

    extern int scale_values[];

    if (testvalue > scale_values[0])
	testvalue = 19;
    else if (testvalue > scale_values[1])
	testvalue = 18;
    else if (testvalue > scale_values[2])
	testvalue = 17;
    else if (testvalue > scale_values[3])
	testvalue = 16;
    else if (testvalue > scale_values[4])
	testvalue = 15;
    else if (testvalue > scale_values[5])
	testvalue = 14;
    else if (testvalue > scale_values[6])
	testvalue = 13;
    else if (testvalue > scale_values[7])
	testvalue = 12;
    else if (testvalue > scale_values[8])
	testvalue = 11;
    else if (testvalue > scale_values[9])
	testvalue = 10;
    else if (testvalue > scale_values[10])
	testvalue = 9;
    else if (testvalue > scale_values[11])
	testvalue = 8;
    else if (testvalue > scale_values[12])
	testvalue = 7;
    else if (testvalue > scale_values[13])
	testvalue = 6;
    else if (testvalue > scale_values[14])
	testvalue = 5;
    else if (testvalue > scale_values[15])
	testvalue = 4;
    else if (testvalue > scale_values[16])
	testvalue = 3;
    else if (testvalue > scale_values[17])
	testvalue = 2;
    else if (testvalue > scale_values[18])
	testvalue = 1;
    else
	testvalue = 0;

    return (testvalue);
}

float get_audio_sample(void) {

    int rc = 0;			/* Return value from subs */
    static float avg = 128.0;
    static float maxval = 0;
    static float minval = 0;
    static float retval = 0.0;
    unsigned char abuf[ABUFSIZE];	/* Audio data buffer */
    int i, k;

    for (k = 0; k < 4; k++) {
	if ((rc = read(afd, abuf, ABUFSIZE)) == -1) {
	    perror("audio read");
	    exit(errno);
	}

	for (i = 0; i < rc; i++) {	// calculate average
	    if ((maxval - minval) != 0.0) {
		if (i > 2)
		    avg =
			avg + (abuf[i] + abuf[i - 1] + abuf[i - 2] +
			       abuf[i - 3]) / 5;
		else
		    avg =
			avg + (abuf[i] + abuf[i + 1] + abuf[i + 2] +
			       abuf[i + 3]) / 5;
	    }
	    if (avg >= 188.0)
		avg = 128.0;
	    if (avg <= 60.0)
		avg = 128.0;

	}
	for (i = 0; i < rc; i++) {	// calculate maximum avg value
	    if (abuf[i] >= avg) {
		if (i > 0)
		    maxval =
			((510 * maxval) + abuf[i] + abuf[i - 1]) / 512;
		else
		    maxval =
			((510 * maxval) + abuf[i] + abuf[i + 1]) / 512;

		if (maxval < avg)
		    maxval = avg + 1;

	    } else {
		if (i > 0)
		    minval =
			((510 * minval) + abuf[i] + abuf[i - 1]) / 512;
		else
		    minval =
			((510 * minval) + abuf[i] + abuf[i + 1]) / 512;

		if (minval >= avg)
		    minval = avg - 1;
	    }

	}
	retval = (maxval - minval);
	if (retval < 0.0)
	    retval = 0.0;
    }
    mvprintw(23, 5, "               ");

    mvprintw(23, 5, "%4.0f", retval);
    refreshp();

    retval = rescale(retval);

    return (retval);
}

/* ----------make bar for s meter ------------*/

int make_bar(int xpos, int ypos, int yheight, unsigned int value,
	     int bar_type) {

    int i;

    if (bar_type == S_BAR) {
	for (i = 0; i < yheight; i++)
	    mvprintw(ypos - i, xpos, " ");
    }

    /* Make the bar */

    if (bar_type == S_BAR || bar_type == PAN_BAR) {
	for (i = 0; i < value && i < 20; i++)
	    mvprintw(ypos - i, xpos, "#");
    } else
	mvprintw(21 - value, xpos, "@");

    refreshp();

    return (0);
}

/* ------------ draw noise bridge screen ----------*/

int draw_nb_screen(int xpos, int ypos, int yheight, int bar_type) {

    int i;

    attroff(A_BOLD);

    for (i = 0; i < 21; i++) {
	switch (i) {
	    case 0:
	    case 20:
		mvprintw(ypos + i, xpos,
			 "+---------+---------+---------+---------+---------+");
		break;
	    case 1 ... 4:
	    case 6 ... 9:
	    case 11 ... 14:
	    case 16 ... 19:
		mvprintw(ypos + i, xpos,
			 "|    |    |    |    |    |    |    |    |    |    |");
		break;
	    case 5:
	    case 10:
	    case 15:

		mvprintw(ypos + i, xpos,
			 "+----+----+----+----+----+----+----+----+----+----+");

	}
    }

    attron(A_BOLD);

    mvprintw(ypos + 1, 2, "%2d-", 0);	// 0
    mvprintw(ypos + 3, 2, "%2d-", 3);	// 3
    mvprintw(ypos + 5, 2, "%2d-", 6);	// 6
    mvprintw(ypos + 10, 2, "%2d-", 10);
    mvprintw(ypos + 15, 2, "%2d-", 20);	// 20
    mvprintw(ypos + 20, 2, "%2d-", 40);

    return (0);
}

/* ------------ draw scanner screen ----------*/

int drawscreen(int xpos, int ypos, int yheight, int bar_type) {

    int i;

    attroff(A_BOLD);

    for (i = 0; i < 21; i++) {
	switch (i) {
	    case 0:
	    case 20:
		mvprintw(ypos + i, xpos,
			 "+---------+---------+---------+---------+---------+");
		break;
	    case 1 ... 4:
	    case 6 ... 9:
	    case 11 ... 14:
	    case 16 ... 19:
		mvprintw(ypos + i, xpos,
			 "|    |    |    |    |    |    |    |    |    |    |");
		break;
	    case 5:
	    case 10:
	    case 15:

		mvprintw(ypos + i, xpos,
			 "+----+----+----+----+----+----+----+----+----+----+");

	}
    }

    attron(A_BOLD);

    mvprintw(ypos + 1, 2, "%2d-", 20);
    mvprintw(ypos + 3, 2, "%2d-", 10);
    mvprintw(ypos + 5, 2, "%2d-", 9);
    mvprintw(ypos + 7, 2, "%2d-", 8);
    mvprintw(ypos + 9, 2, "%2d-", 7);
    mvprintw(ypos + 11, 2, "%2d-", 6);
    mvprintw(ypos + 13, 2, "%2d-", 5);
    mvprintw(ypos + 15, 2, "%2d-", 4);
    mvprintw(ypos + 17, 2, "%2d-", 3);
    mvprintw(ypos + 19, 2, "%2d-", 2);

    return (0);
}

/* ------------ draw S meter screen ----------*/

int drawSmeter(int xpos, int ypos, int yheight, float testvalue) {

    int i;

    for (i = 0; i < 21; i++) {
	switch (i) {
	    case 0:
	    case 20:
		mvprintw(ypos + i, xpos, "+---+");
		break;
	    case 1 ... 4:
	    case 6 ... 19:
		mvprintw(ypos + i, xpos, "|   |");
		break;
	    case 5:
		attron(modify_attr(COLOR_PAIR(C_HEADER) | A_STANDOUT));
		mvprintw(ypos + i, xpos, ">   <");
		attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));
	}
    }

    mvprintw(ypos + 1, 3, "%2d", 20);
    mvprintw(ypos + 3, 3, "%2d", 10);
    mvprintw(ypos + 5, 3, "%2d", 9);
    mvprintw(ypos + 7, 3, "%2d", 8);
    mvprintw(ypos + 9, 3, "%2d", 7);
    mvprintw(ypos + 11, 3, "%2d", 6);
    mvprintw(ypos + 13, 3, "%2d", 5);
    mvprintw(ypos + 15, 3, "%2d", 4);
    mvprintw(ypos + 17, 3, "%2d", 3);
    mvprintw(ypos + 19, 3, "%2d", 2);

    make_bar(7, 20, 20, (int) testvalue, S_BAR);

    refreshp();

    return (0);
}

/* ------------------ Panoramic scan -------------------- */

int panscan(void) {

    int j, key = 0;
    float testvalue;
    float FromFrequency = 0.0;
    float FrequencyStep = 0.0;
    float frequencies[51];

    while (1) {
	key = 0;
	attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

	for (j = 0; j <= 24; j++)
	    mvprintw(j, 0,
		     "                                                                                ");

	drawscreen(5, 1, 21, 0);

	mvprintw(1, 60, "- PANORAMIC SCAN -");
	mvprintw(5, 60, "Frequency: ");
	refreshp();
	echo();
	scanf("%f", &FromFrequency);
	noecho();
	mvprintw(5, 72, "%5.1f", FromFrequency);
	mvprintw(22, 1, "%5.1f", FromFrequency);
	refreshp();
	mvprintw(7, 60, "Step (kHz): ");
	refreshp();
	scanf("%f", &FrequencyStep);
	mvprintw(7, 72, "%5.1f", FrequencyStep);
	refreshp();

	for (j = 0; j < 51; j++) {
	    frequencies[j] = FromFrequency + j * FrequencyStep;
	    switch (j) {
		case 10:
		case 20:
		case 30:
		case 40:
		case 50:
		    mvprintw(22, j + 1, "%5.1f", frequencies[j]);
		    break;
		default:
		    ;
	    }

	}
	refreshp();

	/* ------------- scan --------------------- */

	for (j = 0; j < 5; j++)
	    testvalue = get_audio_sample();

	for (j = 0; j < 51; j++) {

	    int i;

	    for (i = 0; i < 100; i++) {
		if (get_outfreq() == 0)
		    break;
		usleep(10 * 1000);
	    }

	    set_outfreq(frequencies[j] * 1000);

	    usleep(50 * 1000);
	    testvalue = get_audio_sample();

	    make_bar(5 + j, 20, 20, (int) testvalue, PAN_BAR);

	    key = key_poll();
	    if (key == 27 || key == '\n' || key == KEY_ENTER)
		break;

	}			// end for

	/* -----------end scan -------------------- */
	mvprintw(23, 60, "----   Key?  ----");

	j = key_get();
	if (j == 27)
	    break;
    }				// end while

    return (0);
}

/* ------------------ Noise bridge scan -------------------- */

int nbscan(void) {

    int j, key = 0;
    float testvalue;
    float FromFrequency = 0.0;
    float FrequencyStep = 0.0;
    float frequencies[51];
    float values[51];

    while (1) {
	key = 0;
	attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

	for (j = 0; j <= 24; j++)	// wipe the screen
	    mvprintw(j, 0,
		     "                                                                                ");

	draw_nb_screen(5, 1, 21, 0);	// draw the grid

	mvprintw(1, 60, "- NOISE BRIDGE -");	// get the parameters
	mvprintw(5, 60, "Frequency: ");
	refreshp();
	scanf("%f", &FromFrequency);
	mvprintw(5, 72, "%5.1f", FromFrequency);
	mvprintw(22, 1, "%5.1f", FromFrequency);
	refreshp();
	mvprintw(7, 60, "Step (kHz): ");
	refreshp();
	scanf("%f", &FrequencyStep);
	mvprintw(7, 72, "%5.1f", FrequencyStep);
	refreshp();

	for (j = 0; j < 51; j++) {	// draw the X scale
	    frequencies[j] = FromFrequency + j * FrequencyStep;
	    switch (j) {
		case 10:
		case 20:
		case 30:
		case 40:
		case 50:
		    mvprintw(22, j + 1, "%5.1f", frequencies[j]);
		    break;
		default:
		    ;
	    }

	}
	refreshp();

	/* ------------- scan --------------------- */

	for (j = 0; j < 5; j++)
	    testvalue = get_audio_sample();

	for (j = 0; j < 51; j++) {

	    int i;

	    for (i = 0; i < 100; i++) {
		if (get_outfreq() == 0)
		    break;
		usleep(10 * 1000);
	    }

	    set_outfreq(frequencies[j] * 1000);

	    usleep(50 * 1000);
	    testvalue = get_audio_sample();
	    values[j] = testvalue;
	    if (j > 0)
		testvalue = (values[j - 1] + values[j]) / 2;

	    make_bar(5 + j, 20, 20, (int) testvalue, SPOT_BAR);

	    key = key_poll();
	    if (key == 27 || key == '\n' || key == KEY_ENTER)
		break;

	}			// end for

	/* -----------end scan -------------------- */
	mvprintw(23, 60, "----   Key?  ----");

	j = key_get();
	if (j == 27)
	    break;
    }				// end while

    return (0);
}

void scanmenu(void) {
    int j;

    attron(modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (j = 0; j <= 24; j++)
	mvprintw(j, 0,
		 "                                                                                ");

    mvprintw(1, 20, "--- TLF SCANNER UTILITY ---");
    mvprintw(6, 20, "0.: Calibrate S-meter on S9 !");
    mvprintw(8, 20, "1.: Panorama scan");
    mvprintw(9, 20, "2.: Noise bridge scan");
//mvprintw (10, 20, "3.: Channel scan (n.a.)");
//mvprintw (11, 20, "4.: Beacon scan (n.a.)");
    mvprintw(11, 20, "ESC: Exit scan function");

    mvprintw(23, 20, " --- Press a key to continue --- ");
    refreshp();

}

/* -------------------main test routine ------- */

int testaudio() {

    float testvalue;
    char key = '\0';
    int runnit = 1;

    clear();

    scanmenu();

    /* ------------initialize sound card --------*/
    init_audio();

    while (runnit == 1) {

	testvalue = get_audio_sample();

	drawSmeter(5, 1, 21, testvalue);

	key = key_poll();

	switch (key) {

	    case '1':
		panscan();
		scanmenu();
		break;
	    case '2':
		nbscan();
		scanmenu();
		break;

	    // <Escape>
	    case 27:
		runnit = 0;
	}
    }

    close_audio();
    clear();

    return (0);
}

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
    system(commands);
    //G4KNO: Loop until <esc> keypress
    while (1) {
	if (key_get() == 27) {
	    //kill process (SIGINT=Ctrl-C).
	    system("pkill -SIGINT -n rec");
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
		system("echo " " > ~/.VRlock");

		system
		     ("cd ~/tlf/soundlogs; ./soundlog  > /dev/null 2> /dev/null &");

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
		system("rm ~/.VRlock");
		system("pkill -f soundlogs > /dev/null 2> /dev/null ");
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
			    strncpy(printname, soundfilename->d_name, 6);
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
		system(commands);
		runnit = 0;
		break;
	    case 27:
		runnit = 0;
	}
    }

}
