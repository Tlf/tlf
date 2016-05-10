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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "displayit.h"
#include "lancode.h"
#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"

#define BUFSIZE   81
char buffer[BUFSIZE];

/** shorten CW numbers */
char short_number( char c) {
    extern int shortqsonr;

    if (shortqsonr == SHORTCW) {
	if (c == '9')  return 'N';
	if (c == '0')  return 'T';
    }
    return c;
}

/*
 * Replace in-place occurences of 'what' in 'buf' by 'rep'.
 * Maximum 'count' replacements are done.
 *
 */
void replace_n(char *buf, const char *what, const char *rep, int count) {
    char tmp[BUFSIZE];
    char *p = buf;
    char *q;
    int len_what = strlen(what);
    int len_rep = strlen(rep);

    if (len_what == 0) {
        return;
    }

    while (count-- > 0 && ((q = strstr(p, what)) != NULL)) {
        strcpy(tmp, q + len_what);
        int available = BUFSIZE - (q - buf);
        strncpy(q, rep, available);
        available -= len_rep;
        strncpy(q + len_rep, tmp, available);
        p = q + len_rep;
    }
}

void replace_1(char *buf, const char *what, const char *rep) {
    replace_n(buf, what, rep, 1);
}

void replace_all(char *buf, const char *what, const char *rep) {
    replace_n(buf, what, rep, 999);
}

void ExpandMacro(void) {

    extern char call[20];
    extern char hiscall[20];
    extern char hiscall_sent[];
    extern char his_rst[];
    extern char qsonrstr[5];
    extern char comment[];
    extern char lastqsonr[];
    extern int early_started;
    extern int noleadingzeros;
    extern int lan_active;
    extern int exchange_serial;

    int i;
    static char comstr[BUFSIZE] = "";
    static char qsonroutput[5] = "";
    static char rst_out[4] = "";


    strcpy(comstr, call); 
    comstr[strlen(call) - 1] = '\0'; // skip trailing \n
    replace_all(buffer, "%", comstr);   /* mycall */


    if (NULL != strstr(buffer, "@")) {
        char *p = hiscall + strlen(hiscall_sent);
	if (strlen(hiscall_sent) != 0) {
	    hiscall_sent[0] = '\0';
	    early_started = 0;
//                              sending_call = 0;
	}
        replace_1(buffer, "@", p);   /* his call, 1st occurence */
        replace_all(buffer, "@", hiscall);   /* his call, further occurrences */
    }


    strncpy(rst_out, his_rst, 4);
    rst_out[1] = short_number(rst_out[1]);
    rst_out[2] = short_number(rst_out[2]);
    rst_out[3] = '\0';

    replace_all(buffer, "[", rst_out);   /* his RST */


    if (NULL != strstr(buffer, "#")) {
        int leading_zeros = 0;
        int lead = 1;
        for (i = 0; i <= 4; i++) {
            if (lead && qsonrstr[i] == '0') {
                ++leading_zeros;
            } else {
                lead = 0;
            }
            qsonroutput[i] = short_number(qsonrstr[i]);
        }
        qsonroutput[4] = '\0';

	if (noleadingzeros != 1 && leading_zeros > 1) {
            leading_zeros = 1;
	}

        replace_all(buffer, "#", qsonroutput + leading_zeros);   /* serial nr */

	if ((lan_active == 1) && (exchange_serial == 1)) {
	    strncpy(lastqsonr, qsonrstr, 5);
	    send_lan_message(INCQSONUM, qsonrstr);
	}
    }


    replace_all(buffer, "!", comment);
}


void sendbuf(void)
{
    extern int trxmode;
    extern int searchflg;
    extern char termbuf[];
    extern char backgrnd_str[];
    extern char wkeyerbuffer[];
    extern int keyerport;
    extern int data_ready;
    extern int simulator;
    extern int simulator_mode;
    extern int lan_active;
    extern int exchange_serial;
    extern int noleadingzeros;
    extern int early_started;
    extern int sending_call;

    static char printlinebuffer[82] = "";

    printlinebuffer[0] = '\0';

    if ((trxmode == CWMODE || trxmode == DIGIMODE)
	&& (keyerport != NO_KEYER)) {

	ExpandMacro();

	if ((strlen(buffer) + strlen(termbuf)) < 80) {
	    if (simulator == 0)
		strcat(termbuf, buffer);
//              if (sending_call == 1) {
//                      strcat (termbuf, " ");
//                      sending_call = 0;
//              }
	}

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

	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);

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
    }
}


/** \brief send message
 *
 * Send the message via CW or DIGI mode, but only if not empty
 * \param msg message to send
 */
void sendmessage(const char *msg)
{
    if (strlen(msg) != 0) {
	g_strlcat(buffer, msg, sizeof(buffer));
	sendbuf();
    }
}
