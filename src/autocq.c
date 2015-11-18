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
#include "tlf.h"
#include "printcall.h"
#include "clear_display.h"
#include "sendbuf.h"
#include "stoptx.h"
#include "cw_utils.h"
#include "ui_utils.h"
#include "time_update.h"

int play_file(char *audiofile);

/* FIXME: Needs refactorization and cleanup of logic */
int auto_cq(void)
{
    extern char mode[];
    extern char message[][80];
    extern char ph_message[14][80];
    extern int cqdelay;
    extern int cqmode;
    extern int trxmode;
    extern char hiscall[];

    int inchar = -1, delayval = 0, cw_message_len = 0, realspeed = 0, j =
	0;
    long message_time = 0;
    char cwmessage[80];
    int letter = 0;

    strcpy(mode, "AUTO_CQ ");
    clear_display();
    while (delayval == 0) {
	if (trxmode == CWMODE || trxmode == DIGIMODE) {
	    sendmessage(message[11]);
	} else
	    play_file(ph_message[11]);

	mvprintw(12, 29 + strlen(hiscall), "");

	attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

	delayval = cqdelay;

	if (trxmode == CWMODE) {
	    realspeed = GetCWSpeed();
	    strncpy(cwmessage, message[11], 79);
	    cw_message_len = cw_message_length(cwmessage);
	    message_time = (long) (1200.0 / realspeed) * cw_message_len;
	    for (j = 0; j < 10; j++) {
		usleep(message_time * 100);
		time_update();
		inchar = key_poll();
		if (inchar > 0) {
		    letter = inchar;
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
	    time_update();

	    if (inchar < 0)
		inchar = key_poll();
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

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)));

    if (letter > 96 && letter < 123)
	letter -= 32;

    mvprintw(12, 29, "             ");
    printcall();
    if (inchar == 27)
	return (27);
    else
	return (letter);
}


