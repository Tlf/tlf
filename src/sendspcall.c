/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2005 Rein Couperus <pa0r@amsat.org>
 *               2012,2014      Thomas Beierlein <tb@forth-ev.de>
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
*        Sendspcall  sends (de) call if callarea empty
*
*--------------------------------------------------------------*/


#include <string.h>

#include <glib.h>

#include "callinput.h"
#include "globalvars.h"
#include "sendbuf.h"
#include "tlf.h"



/** prepares your own call in S&P mode
 *
 * \return string with prepared call, has to be freed with g_free()
 *         after use
 */
char *PrepareSPcall() {

    char *buf = g_malloc(80);
    char *c;
    buf[0] = '\0';

    if (trxmode == CWMODE) {

	if (demode)
	    strcat(buf, "DE ");

	strcat(buf, my.call);

    } else if (trxmode == DIGIMODE) {

	if (digikeyer == MFJ1278_KEYER) {
	    strcat(buf, "{ ");	/* => ctrl-t */
	    strcat(buf, "|");	/* => CR */
	    if (demode) {
		strcat(buf, hiscall);
		strcat(buf, " DE ");
	    }
	    strcat(buf, my.call);
	    if ((c = strchr(buf, '\n')) == NULL)
		strcat(buf, " ");
	    else
		*c = ' ';
	    strcat(buf, "}");	/* => ctrl-r */
	} else {
	    strcat(buf, "|");	/* => CR */
	    if (demode) {
		strcat(buf, hiscall);
		strcat(buf, " DE ");
	    }
	    strcat(buf, my.call);
	    if ((c = strchr(buf, '\n')) == NULL)
		strcat(buf, " ");
	    else
		*c = ' ';
	}
    }
    return buf;
}


void sendspcall(void) {

    if (trxmode == CWMODE || trxmode == DIGIMODE) {

	/* if set use SPCALL message */
	if (*message[SP_CALL_MSG] != '\0') {
	    sendmessage(message[SP_CALL_MSG]);
	} else { /* otherwise prepare one */
	    char *SPcall = PrepareSPcall();
	    sendmessage(SPcall);
	    g_free(SPcall);
	}

    } else

	play_file(ph_message[5]);
}
