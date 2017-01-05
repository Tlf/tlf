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
 	*      Stop TX
 	*
 	*--------------------------------------------------------------*/


#include "clear_display.h"
#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"

#include "fldigixmlrpc.h"

int stoptx(void)
{
  	extern int trxmode;
  	extern int keyerport;
	extern int digikeyer;


	if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
	    fldigi_to_rx();
 	}
	else if (trxmode == CWMODE) {
	    if (keyerport == NET_KEYER) {

		if (netkeyer (K_ABORT, NULL) < 0) {

			mvprintw(24,0, "keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();

		}
	    }
	}
	else {
	    return(1);
	}
	return(0);
}

