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

#include <curses.h>

#include "bandmap.h"
#include "fldigixmlrpc.h"
#include "getctydata.h"
#include "searchlog.h"		// Includes glib.h
#include "showinfo.h"
#include "tlf.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LIBHAMLIB
# include <hamlib/rig.h>
#endif

void send_bandswitch(int outfreq);

void grabspot(void)
{
    extern char hiscall[];
    extern char mode[];
    extern int cqmode;
    extern int trx_control;

    extern float mem;
    extern float freq;

#ifdef HAVE_LIBHAMLIB
    extern freq_t outfreq;
#else
    extern int outfreq;
#endif

    spot *data;

    if (trx_control == 0)
	return;

    if (hiscall[0] != '\0') {

	data = bandmap_lookup( hiscall );

	if (data != NULL) {

	    outfreq = data -> freq;
	    outfreq -= fldigi_get_carrier();
	    send_bandswitch( outfreq );

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
	}

    }
}

void grab_next(void)
{
    extern char hiscall[];
    extern char mode[];
    extern int cqmode;
    extern int trx_control;

    extern float mem;
    extern float freq;

#ifdef HAVE_LIBHAMLIB
    extern freq_t outfreq;
#else
    extern int outfreq;
#endif

    static int dir = 1;		/* start scanning up */

    spot *data;

    if (trx_control == 0)
	return;

    data = bandmap_next( dir, (unsigned int)(freq*1000) );

    if (data == NULL) {		/* nothing in that direction */
				/* try other one */
	dir = 1 - dir;
	data = bandmap_next( dir, (unsigned int)(freq*1000));
    }

    if (data != NULL) {

	outfreq = data -> freq;
	outfreq -= fldigi_get_carrier();
	send_bandswitch( outfreq );

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
    }
}
