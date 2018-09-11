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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <glib.h>

#include "clear_display.h"
#include "ignore_unused.h"
#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"

#include "fldigixmlrpc.h"

char wkeyerbuffer[400] = "";
int data_ready = 0;

pthread_mutex_t keybuffer_mutex = PTHREAD_MUTEX_INITIALIZER;

/** append string to key buffer*/
void keyer_append(const char *string) {
    pthread_mutex_lock(&keybuffer_mutex);
    g_strlcat(wkeyerbuffer, string, sizeof(wkeyerbuffer));
    data_ready = 1;
    pthread_mutex_unlock(&keybuffer_mutex);
}

/** flush key buffer */
void keyer_flush() {
    pthread_mutex_lock(&keybuffer_mutex);
    wkeyerbuffer[0] = '\0';
    data_ready = 0;
    pthread_mutex_unlock(&keybuffer_mutex);
}


/** write key buffer to keying device
 *
 * should be called periodically from the background task */
int write_keyer(void) {

    extern int trxmode;
    extern int cwkeyer;
    extern int digikeyer;
    extern char controllerport[];
    extern char rttyoutput[];

    FILE *bfp = NULL;
    char outstring[420] =
	"";	// this was only 120 char length, but wkeyerbuffer is 400
    char *tosend = NULL;

    if (trxmode != CWMODE && trxmode != DIGIMODE)
	return (1);

    pthread_mutex_lock(&keybuffer_mutex);
    if (data_ready == 1) {
	/* allocate a copy of the data and free the buffer */
	tosend = g_strdup(wkeyerbuffer);
	wkeyerbuffer[0] = '\0';
	data_ready = 0;
    }
    pthread_mutex_unlock(&keybuffer_mutex);

    if (tosend != NULL) {

	if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
	    fldigi_send_text(tosend);
	} else if (cwkeyer == NET_KEYER) {
	    netkeyer(K_MESSAGE, tosend);

	} else if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
	    if ((bfp = fopen(controllerport, "a")) == NULL) {
		mvprintw(24, 0, "1278 not active. Switching to SSB mode.");
		sleep(1);
		trxmode = SSBMODE;
		clear_display();
	    } else {
		fputs(tosend, bfp);
		fclose(bfp);
	    }

	} else if (digikeyer == GMFSK) {
	    if (strlen(rttyoutput) < 2) {
		mvprintw(24, 0, "No modem file specified!");
	    }
	    // when GMFSK used (possible Fldigi interface), the trailing \n doesn't need
	    if (tosend[strlen(tosend) - 1] == '\n') {
		tosend[strlen(tosend) - 1] = '\0';
	    }
	    sprintf(outstring, "echo -n \"\n%s\" >> %s",
		    tosend, rttyoutput);
	    IGNORE(system(outstring));;
	}

	g_free(tosend);
	tosend = NULL;
    }
    return (0);
}
