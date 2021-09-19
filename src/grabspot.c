/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0r@eudxf.org>
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


#include <string.h>

#include "grabspot.h"

#include "bandmap.h"
#include "fldigixmlrpc.h"
#include "getctydata.h"
#include "gettxinfo.h"
#include "globalvars.h"
#include "searchlog.h"		// Includes glib.h
#include "showinfo.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "trx_memory.h"

void send_bandswitch(freq_t outfreq);

static freq_t execute_grab(spot *data);

freq_t grabspot(void) {

    if (!trx_control) {
	return 0;   // no trx control
    }

    if (hiscall[0] == '\0') {
	return 0;   // call input is empty
    }

    spot *data = bandmap_lookup(hiscall);

    if (data == NULL) {
	return 0;   // no spot found
    }

    return execute_grab(data);
}

bool grab_up = true;        /* start scanning up */


freq_t grab_next(void) {

    if (!trx_control) {
	return 0;   // no trx control
    }

    spot *data = bandmap_next(grab_up, freq);

    if (data == NULL) {		/* nothing in that direction */
	/* try other one */
	grab_up = !grab_up;
	data = bandmap_next(grab_up, freq);
    }

    if (data == NULL) {
	return 0;
    }

    return execute_grab(data);
}

/* Perform the steps needed to grab a call and then free data
 * \return frequency of the spot in Hz
 */
static freq_t execute_grab(spot *data) {

    freq_t f = data->freq;
    set_outfreq(f);
    send_bandswitch(f);

    strcpy(hiscall, data->call);

    show_call_info(hiscall);
    searchlog();

    /* if in CQ mode switch to S&P and remember QRG */
    if (cqmode == CQ) {
	memory_store();
	cqmode = S_P;
    }

    refreshp();

    free_spot(data);

    return f;
}
