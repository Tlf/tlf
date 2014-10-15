/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
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
#include "write_keyer.h"
#include <assert.h>
#include "clear_display.h"
#include "cwkeyer.h"
#include <curses.h>
#include "netkeyer.h"
#include "cw_utils.h"

int write_keyer(void)
{

    extern char wkeyerbuffer[];
    extern int trxmode;
    extern int keyerport;
    extern int data_ready;
    extern char controllerport[];
    extern char rttyoutput[];

    FILE *bfp = NULL;
    int rc;
    char outstring[420] = "";	// this was only 120 char length, but wkeyerbuffer is 400

    if (trxmode != CWMODE && trxmode != DIGIMODE)
	return (1);

    if (data_ready == 1) {

	if (keyerport == NET_KEYER) {
	    netkeyer(K_MESSAGE, wkeyerbuffer);
	    wkeyerbuffer[0] = '\0';
	    data_ready = 0;

	} else if (keyerport == MFJ1278_KEYER) {
	    if ((bfp = fopen(controllerport, "a")) == NULL) {
		mvprintw(24, 0, "1278 not active. Switching to SSB mode.");
		sleep(1);
		trxmode = SSBMODE;
		clear_display();
	    } else {
		fputs(wkeyerbuffer, bfp);
		wkeyerbuffer[0] = '\0';
		data_ready = 0;

		fclose(bfp);
	    }

	} else if (keyerport == GMFSK) {
	    if (strlen(rttyoutput) < 2) {
		mvprintw(24, 0, "No modem file specified!");
	    }
	    // when GMFSK used (possible Fldigi interface), the trailing \n doesn't need
	    if (wkeyerbuffer[strlen(wkeyerbuffer)-1] == '\n') {
		wkeyerbuffer[strlen(wkeyerbuffer)-1] = '\0';
	    }
	    sprintf(outstring, "echo -n \"\n%s\" >> %s",
		    wkeyerbuffer, rttyoutput);
	    rc = system(outstring);

	    wkeyerbuffer[0] = '\0';
	    data_ready = 0;


	}
    }

    return (0);

}
