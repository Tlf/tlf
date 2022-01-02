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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */



#include <stdio.h>
#include <unistd.h>

#include <glib.h>

#include "clear_display.h"
#include "cw_utils.h"
#include "err_utils.h"
#include "globalvars.h"
#include "hamlib_keyer.h"
#include "netkeyer.h"
#include "sendbuf.h"
#include "tlf.h"
#include "tlf_curses.h"


void setspeed(void) {

    int retval = 0;
    char buff[3];
    int cwspeed = GetCWSpeed();

    snprintf(buff, 3, "%2u", cwspeed);

    if (cwkeyer == NET_KEYER) {

	retval = netkeyer(K_SPEED, buff);

	if (retval < 0) {
	    TLF_LOG_WARN("keyer not active");
//                      trxmode = SSBMODE;
	    clear_display();
	}
    }

    if (cwkeyer == HAMLIB_KEYER) {

	retval = hamlib_keyer_set_speed(cwspeed);

	if (retval < 0) {
	    TLF_LOG_WARN("Could not set CW speed: %s", rigerror(retval));
	    clear_display();
	}
    }

    if (cwkeyer == MFJ1278_KEYER) {

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
int speedup(void) {

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
int speeddown(void) {

    if (trxmode != CWMODE)	/* bail out, this is an SSB contest */
	return (0);

    if (speed >= 1) {

	speed--;
	setspeed();

    }

    return (speed);
}


/*  write weight to netkeyer */
int setweight(int weight) {

    int retval;
    char buff[4];

    if (cwkeyer == NET_KEYER && weight > -51 && weight < 51) {

	sprintf(buff, "%d", weight);

	retval = netkeyer(K_WEIGHT, buff);

	if (retval < 0) {
	    TLF_LOG_INFO("keyer not active ?");
	    clear_display();
	}
    }

    return (0);

}
