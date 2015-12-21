/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2014           Thomas Beierlein <tb@forth-ev.de>
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



#include <stdio.h>
#include <unistd.h>

#include <glib.h>

#include "clear_display.h"
#include "cw_utils.h"
#include "netkeyer.h"
#include "sendbuf.h"
#include "tlf.h"
#include "tlf_curses.h"


void setspeed(void) {

    extern int keyerport;

    int retval = 0;
    char buff[3];

    snprintf(buff, 3, "%2d", GetCWSpeed());

    if (keyerport == NET_KEYER) {

	retval = netkeyer(K_SPEED, buff);

	if (retval < 0) {
	    mvprintw(24, 0, "keyer not active");
//                      trxmode = SSBMODE;
	    sleep(1);
	    clear_display();
	}
    }

    if (keyerport == MFJ1278_KEYER) {

	char *msg;

	sendmessage("\\\015");
	usleep(500000);

	msg = g_strdup_printf("MSP %s \015", buff);
	sendmessage(msg);
	g_free(msg);

	usleep(500000);
	sendmessage("CONV\015\n");
    }
}

/* ------------------------------------------------------------
 *        Page-up increases CW speed with 2 wpm
 *
 *--------------------------------------------------------------*/
int speedup(void)
{
    extern int trxmode;

    if (trxmode != CWMODE)
	return (0);

    if (speed < 20) {

	speed++;
	setspeed();

    }

    return (speed);
}


/* ------------------------------------------------------------
 *        Page down,  decrementing the cw speed with  2 wpm
 *
 *--------------------------------------------------------------*/
int speeddown(void)
{
    extern int trxmode;

    if (trxmode != CWMODE)	/* bail out, this is an SSB contest */
	return (0);

    if (speed >= 1) {

	speed--;
	setspeed();

    }

    return (speed);
}


int setweight(int weight)
{				//  write weight to netkeyer

    extern int keyerport;

    int retval;
    char buff[4];

    if (keyerport == NET_KEYER && weight > -51 && weight < 51) {

	sprintf(buff, "%d", weight);

	retval = netkeyer(K_WEIGHT, buff);

	if (retval < 0) {
	    mvprintw(24, 0, "keyer not active ?");
	    sleep(1);
	    clear_display();
	}
    }

    return (0);

}
