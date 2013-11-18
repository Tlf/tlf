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

	/* ------------------------------------------------------------
	 *        AUTO_CQ
	 *
	 *--------------------------------------------------------------*/

#include "autocq.h"
#include "stoptx.h"

int cw_char_length(char *message);
int getlength(int testchar, int message_length);
int play_file(char *audiofile);

/* FIXME: Needs refactorization and cleanup of logic */
int auto_cq(void)
{

    extern int use_rxvt;
    extern char mode[];
    extern char message[15][80];
    extern char ph_message[14][80];
    extern char buffer[];
    extern int cqdelay;
    extern int cqmode;
    extern int trxmode;
    extern char hiscall[];
    extern char speedstr[];
    extern int speed;
    extern int trxmode;

    int inchar = -1, delayval = 0, cw_message_len = 0, realspeed = 0, j =
	0;
    long message_time = 0;
    char cwmessage[80], buff[120];
    int letter = 0;

    strcpy(mode, "AUTO_CQ ");
    clear_display();
    nodelay(stdscr, TRUE);
    while (delayval == 0) {
	if (trxmode == CWMODE || trxmode == DIGIMODE) {
	    strcat(buffer, message[11]);
	    sendbuf();
	} else
	    play_file(ph_message[11]);

	mvprintw(12, 29 + strlen(hiscall), "");

	if (use_rxvt == 0)
	    attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
	else
	    attron(COLOR_PAIR(NORMCOLOR));

	delayval = cqdelay;

	if (trxmode == CWMODE) {
	    strncpy(buff, (speedstr + (speed * 2)), 2);
	    buff[2] = '\0';
	    realspeed = atoi(buff);
	    strncpy(cwmessage, message[11], 79);
	    cw_message_len = cw_char_length(cwmessage);
	    message_time = (long) (1200.0 / realspeed) * cw_message_len;
	    for (j = 0; j < 10; j++) {
		usleep(message_time * 100);
		inchar = getch();
		if (inchar > 0)
		    letter = inchar;
		if (inchar > 0) {
		    stoptx();
		    break;
		}
	    }
	}
	for (delayval = cqdelay; delayval > 0; delayval--) {
	    if (inchar < 0) {
		mvprintw(12, 29, "Auto cq  %d  ", delayval - 1);
		refreshp();
	    } else {
		break;
	    }

	    usleep(500000);

	    if (inchar < 0)
		inchar = getch();
	    letter = inchar;
	    if (inchar > 0)
		break;
	}
	mvprintw(12, 29, "            ");
	mvprintw(12, 29, "");
	refreshp();
    }
    if (cqmode == CQ)
	strcpy(mode, "Log     ");
    else
	strcpy(mode, "S&P     ");

    clear_display();

    if (use_rxvt == 0)
	attron(COLOR_PAIR(NORMCOLOR) | A_BOLD);
    else
	attron(COLOR_PAIR(NORMCOLOR));

    if (letter > 96 && letter < 123)
	letter -= 32;

    mvprintw(12, 29, "             ");
    printcall();
    nodelay(stdscr, FALSE);
    if (inchar == 27)
	return (27);
    else
	return (letter);
}

/* FIXME: move next two functions to new file -> refactoring */
int cw_char_length(char *message)	// calculate point lenght of cw messages...
{
    extern char call[];

    int i;
    int message_length = 0;
    char cwmessage[80];
    int testchar, j;

    strncpy(cwmessage, message, 79);
    cwmessage[79] = '\0';

    for (i = 0; i < strlen(cwmessage); i++) {

	testchar = cwmessage[i];
	if (testchar == '%') {
	    for (j = 0; j < strlen(call); j++) {
		testchar = call[j];
		message_length = getlength(testchar, message_length);
	    }

	} else
	    message_length = getlength(testchar, message_length);

    }
    return (message_length);
}

int getlength(int testchar, int message_length)
{

    switch (testchar) {
    case 'A':
	message_length += 9;
	break;
    case 'B':
	message_length += 13;
	break;
    case 'C':
	message_length += 15;
	break;
    case 'D':
	message_length += 11;
	break;
    case 'E':
	message_length += 5;
	break;
    case 'F':
	message_length += 13;
	break;
    case 'G':
	message_length += 13;
	break;
    case 'H':
	message_length += 11;
	break;
    case 'I':
	message_length += 7;
	break;
    case 'J':
	message_length += 17;
	break;
    case 'K':
	message_length += 13;
	break;
    case 'L':
	message_length += 13;
	break;
    case 'M':
	message_length += 11;
	break;
    case 'N':
	message_length += 9;
	break;
    case 'O':
	message_length += 15;
	break;
    case 'P':
	message_length += 15;
	break;
    case 'Q':
	message_length += 17;
	break;
    case 'R':
	message_length += 11;
	break;
    case 'S':
	message_length += 9;
	break;
    case 'T':
	message_length += 7;
	break;
    case 'U':
	message_length += 11;
	break;
    case 'V':
	message_length += 13;
	break;
    case 'W':
	message_length += 13;
	break;
    case 'X':
	message_length += 15;
	break;
    case 'Y':
	message_length += 17;
	break;
    case 'Z':
	message_length += 15;
	break;
    case '0':
	message_length += 23;
	break;
    case '1':
	message_length += 21;
	break;
    case '2':
	message_length += 19;
	break;
    case '3':
	message_length += 17;
	break;
    case '4':
	message_length += 15;
	break;
    case '5':
	message_length += 13;
	break;
    case '6':
	message_length += 15;
	break;
    case '7':
	message_length += 17;
	break;
    case '8':
	message_length += 19;
	break;
    case '9':
	message_length += 21;
	break;
    case '/':
	message_length += 17;
	break;
    case '?':
	message_length += 19;
	break;
    case ' ':
	message_length += 3;
	break;
    default:
	;
    }
    return (message_length);
}
