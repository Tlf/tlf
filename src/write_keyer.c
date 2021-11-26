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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <glib.h>
#include <hamlib/rig.h>

#include "clear_display.h"
#include "err_utils.h"
#include "ignore_unused.h"
#include "globalvars.h"
#include "hamlib_keyer.h"
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

/** append char to key buffer*/
void keyer_append_char(const char c) {
    const char buf[] = {c, 0};
    keyer_append(buf);
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
void write_keyer(void) {

    FILE *bfp = NULL;
    char outstring[420] =
	"";	// this was only 120 char length, but wkeyerbuffer is 400
    char *tosend = NULL;

    if (trxmode != CWMODE && trxmode != DIGIMODE)
	return;

    pthread_mutex_lock(&keybuffer_mutex);
    if (data_ready && wkeyerbuffer[0] != 0) {
	/* allocate a copy of the data and clear the buffer */
	tosend = g_strdup(wkeyerbuffer);
	wkeyerbuffer[0] = '\0';
    }
    data_ready = 0;
    pthread_mutex_unlock(&keybuffer_mutex);

    if (tosend == NULL) {
	return;     // nothing to do
    }

    if (digikeyer == FLDIGI && trxmode == DIGIMODE) {
	fldigi_send_text(tosend);
    } else if (cwkeyer == NET_KEYER) {
	netkeyer(K_MESSAGE, tosend);
    } else if (cwkeyer == HAMLIB_KEYER) {
	// Ignore +/- speed up/slow down instructions
	char *q = tosend;
	for (char *p = q; *p; p++) {
	    if (*p != '+' && *p != '-') {
		*q++ = *p;
	    }
	}
	*q = 0;

	int error = hamlib_keyer_send(tosend);
	if (error != RIG_OK) {
	    TLF_LOG_WARN("CW send error: %s", rigerror(error));
	}
    } else if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER) {
	if ((bfp = fopen(controllerport, "a")) == NULL) {
	    TLF_LOG_WARN("1278 not active. Switching to SSB mode.");
	    trxmode = SSBMODE;
	    clear_display();
	} else {
	    fputs(tosend, bfp);
	    fclose(bfp);
	}

    } else if (digikeyer == GMFSK) {
	if (strlen(rttyoutput) < 2) {
	    TLF_LOG_WARN("No modem file specified!");
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
}
