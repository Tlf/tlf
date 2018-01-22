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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <string.h>

#include "bandmap.h"
#include "fldigixmlrpc.h"
#include "getctydata.h"
#include "searchlog.h"		// Includes glib.h
#include "showinfo.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "gettxinfo.h"

void send_bandswitch(int outfreq);

static double execute_grab(spot *data);

double grabspot(void)
{
    extern char hiscall[];
    extern int trx_control;

    spot *data;

    if (!trx_control) {
	return 0;   // no trx control
    }

    if (hiscall[0] == '\0') {
        return 0;   // call input is empty
    }

    data = bandmap_lookup( hiscall );

    if (data == NULL) {
        return 0;   // no spot found
    }

    return execute_grab(data);
}

double grab_next(void)
{
    extern int trx_control;
    extern float freq;

    static int dir = 1;		/* start scanning up */

    spot *data;

    if (!trx_control) {
	return 0;   // no trx control
    }

    data = bandmap_next( dir, (unsigned int)(freq*1000) );

    if (data == NULL) {		/* nothing in that direction */
				/* try other one */
	dir = 1 - dir;
	data = bandmap_next( dir, (unsigned int)(freq*1000));
    }

    if (data == NULL) {
        return 0;
    }

    return execute_grab(data);
}

/* Perform the steps needed to grab a call and then free data
 * \return frequency of the spot in Hz
 */
static double execute_grab(spot *data)
{
    extern char hiscall[];
    extern char mode[];
    extern int cqmode;
    extern float mem;
    extern float freq;

    double f = data->freq - fldigi_get_carrier();
    set_outfreq(f);
    send_bandswitch( (int) f );

    strcpy( hiscall, data->call );

    showinfo( getctydata( hiscall ) );
    searchlog( hiscall );

    /* if in CQ mode switch to S&P and remember QRG */
    if (cqmode == CQ) {
        cqmode = S_P;
        strcpy(mode, "S&P     ");
        mem = freq;
        mvprintw(14, 68, "MEM: %7.1f", mem);
    }

    refreshp();

    g_free( data->call );
    g_free( data );

    return f;
}
