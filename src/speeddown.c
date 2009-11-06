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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
  	/* ------------------------------------------------------------
 	*        Page down,  decrementing the cw speed with  2 wpm
 	*
 	*--------------------------------------------------------------*/

#include "speeddown.h"
#include "tlf.h"
#include "cwkeyer.h"
#include "clear_display.h"
#include "netkeyer.h"

int speeddown(void)
{

extern int speed;
extern char speedstr[];
extern int trxmode;
extern int keyerport;
extern int cfd;
extern char buffer[];

int  retval;
char buff[3];

if (trxmode != CWMODE)          /* bail out, this is an SSB contest */
		return (0);

if (keyerport == COM1_KEYER) {

	if (speed >= 1)
	{
		speed--;

		strncpy(buff, speedstr+(speed * 2), 2);
		buff[2] = '\0';
		retval = ioctl(cfd, CWSPEED, atoi(buff));
		if	(retval)
		{
			mvprintw(24,0, "keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
		}
	}
}

if (keyerport == LPT_KEYER) {

 	if (speed >= 1) {

 		speed--;

 		strncpy(buff, speedstr+(speed * 2), 2);
		buff[2] = '\0';

		retval = ioctl(cfd, CWKEYER_IOCSSPEED, atoi(buff));
		if	(retval)
		{
			mvprintw(24,0, "keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
		}

 	}
}

if (keyerport == NET_KEYER){

	if (speed >= 1) {

 		speed--;

 		strncpy(buff, speedstr+(speed * 2), 2);
		buff[2] = '\0';

		retval = netkeyer (K_SPEED, buff);
		if	(retval < 0)
		{
			mvprintw(24,0, "keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
		}

 	}
}
if (keyerport == MFJ1278_KEYER) {

 	if (speed >=1) {

 		speed--;

 		strncpy(buff, speedstr+(speed * 2), 2);
		buff[2] = '\0';

		strcpy (buffer, "\\\015");
		sendbuf();
		usleep(500000);
		strcpy (buffer, "MSP ");
		strcat (buffer, buff);
		strcat (buffer, " \015");
		sendbuf();
		usleep(500000);
		strcpy (buffer, "CONV\015\n");
		sendbuf();
 	}
}
if (keyerport == ORION_KEYER) {

 	if (speed >= 1) {

 		speed--;

 		strncpy(buff, speedstr+(speed * 2), 2);
		buff[2] = '\0';

		orion_set_cw_speed(atoi(buff));

 	}
}
return (speed);	
}

