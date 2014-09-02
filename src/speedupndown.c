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

#include "speedupndown.h"
#include "tlf.h"
#include "sendbuf.h"
#include "clear_display.h"
#include "netkeyer.h"
#include "cw_utils.h"

/* ------------------------------------------------------------
 *        Page-up increases CW speed with 2 wpm
 *
 *--------------------------------------------------------------*/
int speedup(void)
{

    extern int trxmode;
    extern int keyerport;

    int retval = 0;
    char buff[3];

    if (trxmode != CWMODE)
	return (0);

    if (keyerport == NET_KEYER) {

	if (speed < 20) {

	    speed++;

	    snprintf(buff, 3, "%2d", GetCWSpeed());

	    retval = netkeyer(K_SPEED, buff);

	    if (retval < 0) {
		mvprintw(24, 0, "keyer not active");
//                      trxmode = SSBMODE;
		sleep(1);
		clear_display();
	    }

	}
    }

    if (keyerport == MFJ1278_KEYER) {

	if (speed < 20) {

	    speed++;

	    snprintf(buff, 3, "%2d", GetCWSpeed());

	    strcpy(buffer, "\\\015");
	    sendbuf();
	    usleep(500000);
	    strcpy(buffer, "MSP ");
	    strcat(buffer, buff);
	    strcat(buffer, " \015");
	    sendbuf();
	    usleep(500000);
	    strcpy(buffer, "CONV\015\n");
	    sendbuf();

	    if (retval < 0) {
		mvprintw(24, 0, "keyer not active");
//                      trxmode = SSBMODE;
		sleep(1);
		clear_display();
	    }

	}
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
    extern int keyerport;

    int retval;
    char buff[3];

    if (trxmode != CWMODE)	/* bail out, this is an SSB contest */
	return (0);

    if (keyerport == NET_KEYER) {

	if (speed >= 1) {

	    speed--;

	    snprintf(buff, 3, "%2d", GetCWSpeed());

	    retval = netkeyer(K_SPEED, buff);
	    if (retval < 0) {
		mvprintw(24, 0, "keyer not active; switching to SSB");
		trxmode = SSBMODE;
		clear_display();
	    }

	}
    }
    if (keyerport == MFJ1278_KEYER) {

	if (speed >= 1) {

	    speed--;

	    snprintf(buff, 3, "%2d", GetCWSpeed());

	    strcpy(buffer, "\\\015");
	    sendbuf();
	    usleep(500000);
	    strcpy(buffer, "MSP ");
	    strcat(buffer, buff);
	    strcat(buffer, " \015");
	    sendbuf();
	    usleep(500000);
	    strcpy(buffer, "CONV\015\n");
	    sendbuf();
	}
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
