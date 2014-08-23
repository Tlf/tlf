/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 *               2014                     Thomas Beierlein <tb@forth-ev.de>
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
/* ------------------------------------------------------------------------
*    send the text buffer to the keyer  driver
*
---------------------------------------------------------------------------*/
#include "sendbuf.h"
#include "netkeyer.h"
#include "lancode.h"
#include "displayit.h"
#include "tlf.h"

#include <glib.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <curses.h>

char buffer[81];

void sendbuf(void)
{
    extern int trxmode;
    extern char call[20];
    extern char hiscall[20];
    extern char his_rst[];
    extern char qsonrstr[5];
    extern int shortqsonr;
    extern int searchflg;
    extern char termbuf[];
    extern char backgrnd_str[];
    extern int bufloc;
    extern char wkeyerbuffer[];
    extern int keyerport;
    extern int data_ready;
    extern int simulator;
    extern int simulator_mode;
    extern int lan_active;
    extern int exchange_serial;
    extern char lastqsonr[];
    extern int noleadingzeros;
    extern int arrlss;
    extern int keyerport;
    extern char hiscall_sent[];
    extern int early_started;
    extern int sending_call;
    extern char comment[];

    static char comstr[80] = "";
    char comstr3[5];
    static char printlinebuffer[82] = "";
    static char qsonroutput[5] = "";
    static char rst_out[4] = "";

    size_t loc;
    int i, nr;

    comstr[0] = '\0';
    printlinebuffer[0] = '\0';

    if (arrlss == 1)
	shortqsonr = 0;

    if ((trxmode == CWMODE || trxmode == DIGIMODE)
	&& (keyerport != NO_KEYER)) {

	loc = strcspn(buffer, "%");	/* mycall */

	while (strlen(buffer) > loc) {

	    if (loc != 0)
		strncat(comstr, buffer, loc);
	    strncat(comstr, call, (strlen(call) - 1));
	    strcat(comstr, buffer + loc + 1);
	    strcpy(buffer, comstr);
	    strcpy(comstr, "");

	    loc = strcspn(buffer, "%");
	}

	loc = strcspn(buffer, "@");	/* his call */

	while (strlen(buffer) > loc) {

	    if (loc != 0)
		strncat(comstr, buffer, loc);
	    if (strlen(hiscall_sent) == 0) {
		strcat(comstr, hiscall);
	    } else {
		strcat(comstr, hiscall + strlen(hiscall_sent));
		hiscall_sent[0] = '\0';
		early_started = 0;
//                              sending_call = 0;
	    }
	    strcat(comstr, buffer + loc + 1);
	    strcpy(buffer, comstr);
	    strcpy(comstr, "");

	    loc = strcspn(buffer, "@");
	}

	loc = strcspn(buffer, "[");	/* his RST */

	while (strlen(buffer) > loc) {

	    if (loc != 0)
		strncat(comstr, buffer, loc);

	    strncpy(rst_out, his_rst, 4);

	    if (shortqsonr == 1) {
		if (rst_out[1] == '9')
		    rst_out[1] = 'N';
		if (rst_out[2] == '9')
		    rst_out[2] = 'N';
	    }
	    strcat(comstr, rst_out);
	    strcat(comstr, buffer + loc + 1);
	    strcpy(buffer, comstr);
	    strcpy(comstr, "");

	    loc = strcspn(buffer, "[");
	}

	strcpy(qsonroutput, qsonrstr);

	if (shortqsonr != 0) {
	    for (i = 0; i <= 4; i++) {
		if (qsonroutput[i] == '0')
		    qsonroutput[i] = 'T';
		if (qsonroutput[i] == '9')
		    qsonroutput[i] = 'N';
	    }
	}

	loc = strcspn(buffer, "#");	/* serial nr */

	while (strlen(buffer) > loc) {

	    if (loc != 0)
		strncat(comstr, buffer, loc);

	    if (noleadingzeros == 1) {

		nr = atoi(qsonroutput);
		sprintf(comstr3, "%d", nr);
		strcat(comstr, comstr3);

	    } else {
		if (qsonroutput[0] == '0' || qsonroutput[0] == 'T')
		    strcat(comstr, qsonroutput + 1);
		else
		    strcat(comstr, qsonroutput);
	    }

	    strcat(comstr, buffer + loc + 1);

	    strcpy(buffer, comstr);

	    if ((lan_active == 1) && (exchange_serial == 1)) {
		strncpy(lastqsonr, qsonrstr, 5);
		send_lan_message(INCQSONUM, qsonrstr);
	    }

	    strcpy(comstr, "");

	    loc = strcspn(buffer, "#");
	}

	loc = strcspn(buffer, "!");	/* his serial nr/comment  */

	while (strlen(buffer) > loc) {

	    if (loc != 0)
		strncat(comstr, buffer, loc);
	    strncat(comstr, comment, strlen(comment));

	    strcat(comstr, buffer + loc + 1);
	    strcpy(buffer, comstr);
	    strcpy(comstr, "");

	    loc = strcspn(buffer, "!");
	}

	if ((strlen(buffer) + strlen(termbuf)) < 80) {
	    if (simulator == 0)
		strcat(termbuf, buffer);
//              if (sending_call == 1) {
//                      strcat (termbuf, " ");
//                      sending_call = 0;
//              }
	}

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

	if (simulator == 0)
	    strncat(printlinebuffer, termbuf, strlen(termbuf));
	else
	    strncat(printlinebuffer, termbuf, strlen(termbuf));

	if (searchflg == 0 && simulator == 0)
	    strncat(printlinebuffer, backgrnd_str,
		    80 - strlen(printlinebuffer));
	else {
	    int len = 40 - (int)strlen(printlinebuffer);
	    if (len > 0) {
		strncat(printlinebuffer, backgrnd_str, len);
	    }
	    if (strlen(printlinebuffer) > 45) {
		printlinebuffer[42] = '.';
		printlinebuffer[43] = '.';
		printlinebuffer[44] = '.';
		printlinebuffer[45] = '\0';
	    }

	}

	if ((simulator_mode == 0)) {
	    mvprintw(5, 0, printlinebuffer);
	    refreshp();
	}
	refreshp();

	if (trxmode == DIGIMODE) {

	    if (data_ready != 1) {
		if (keyerport == MFJ1278_KEYER) {
		    int i = 0;
		    for (i = 0; i < strlen(buffer); i++)
			if (buffer[i] == '\n')
			    buffer[i] = 13;
		    for (i = 0; i < strlen(buffer); i++)
			if (buffer[i] == 123)
			    buffer[i] = 20;	/* ctrl-t */
		    for (i = 0; i < strlen(buffer); i++)
			if (buffer[i] == 125)
			    buffer[i] = 18;	/* ctrl-r */
		}
		strcat(wkeyerbuffer, buffer);
		buffer[0] = '\0';
		data_ready = 1;
	    } else
		buffer[0] = '\0';
	}

	if (trxmode == CWMODE) {

	    if (data_ready != 1) {
		if (keyerport == MFJ1278_KEYER) {
		    int i = 0;
		    for (i = 0; i < strlen(buffer); i++)
			if (buffer[i] == '\n')
			    buffer[i] = 13;
		}
		strcat(wkeyerbuffer, buffer);
		if (keyerport == NET_KEYER) {
		    netkeyer(K_MESSAGE, wkeyerbuffer);
		    wkeyerbuffer[0] = '\0';
		    data_ready = 0;
		} else
		    data_ready = 1;

	    } else
		buffer[0] = '\0';
	}

	if (simulator == 0) {
	    if (sending_call == 0)
		displayit();
	    refreshp();
	}

	buffer[0] = '\0';
	bufloc = 0;
    }
}


/** \brief send message
 *
 * Send the message via CW or DIGI mode, but only if not empty
 * \param msg message to send
 */
void sendmessage(char *msg)
{
    if (strlen(msg) != 0) {
	g_strlcat(buffer, msg, sizeof(buffer));
	sendbuf();
    }
}
