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
 	*      Stop TX
 	*
 	*--------------------------------------------------------------*/

#include "stoptx.h"
#include "tlf.h"
#include "cwkeyer.h"
#include "clear_display.h"
#include "netkeyer.h"

int stoptx(void)
{
	extern char hiscall[20];
  	extern char speedstr[];
  	extern int speed;
  	extern int trxmode;
  	extern int keyerport;
  	extern int cfd;

 	char speedbuf[3] = "  ";
 	int retval;
 	
//mvprintf (22, 0, "trxmode=%d\n", trxmode);
 	
 	if (trxmode != CWMODE){
 		return(1);
 	}


	if (keyerport == NET_KEYER) {

		if (netkeyer (K_ABORT, NULL) < 0) {
			
			mvprintw(24,0, "keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
			
		}
	}
	return(0);
}

