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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* ------------------------------------------------------------
*      Stop TX
*
*--------------------------------------------------------------*/


#include <hamlib/rig.h>
#include "clear_display.h"
#include "err_utils.h"
#include "globalvars.h"
#include "hamlib_keyer.h"
#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"

#include "fldigixmlrpc.h"

int stoptx(void) {

    if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
	fldigi_to_rx();
    } else if (trxmode == CWMODE) {
	if (cwkeyer == NET_KEYER) {

	    if (netkeyer(K_ABORT, NULL) < 0) {

		TLF_LOG_WARN("keyer not active; switching to SSB");
		trxmode = SSBMODE;
		clear_display();

	    }
	} else if (cwkeyer == HAMLIB_KEYER) {
	    int error = hamlib_keyer_stop();
	    if (error != RIG_OK) {
		TLF_LOG_WARN("CW stop error: %s", rigerror(error));
	    }
	}
    } else {
	return (1);
    }
    return (0);
}

