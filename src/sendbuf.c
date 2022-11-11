/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 *               2014, 2016               Thomas Beierlein <tb@forth-ev.de>
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
/* ------------------------------------------------------------------------
*    send the text buffer to the keyer  driver
*
---------------------------------------------------------------------------*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "audio.h"
#include "cqww_simulator.h"
#include "callinput.h"
#include "clear_display.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "lancode.h"
#include "netkeyer.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "write_keyer.h"

#include "sendbuf.h"

#define BUFSIZE   81
char buffer[BUFSIZE];

/** shorten CW numbers */
char short_number(char c) {

    if (shortqsonr == SHORTCW) {
	if (c == '9')  return 'N';
	if (c == '0')  return 'T';
    }
    return c;
}

/*
 * Replace occurrences of 'what' in 'buf' by 'rep'.
 * The amount of bytes assigned to 'buf' is 'size'.
 * This includes the terminating \0, i.e. max length of 'buf' is 'size'-1
 * Replacements are done in-place, no other memory area than 'buf' is used.
 * Maximum 'count' replacements are done.
 *
 */
void replace_n(char *buf, int size, const char *what, const char *rep,
	       int count) {
    int len = strlen(buf);
    if (len > size - 1) {
	// input string already too long, don't touch it
	return;
    }
    int len_what = strlen(what);
    if (len_what == 0) {
	return;
    }

    int len_rep = strlen(rep);
    int len_overlap = (len_rep < len_what ? len_rep : len_what);

    buf[size - 1] = 0; // ensure proper termination

    char *p = buf;
    char *q;

    while (count-- > 0 && (q = strstr(p, what)) != NULL) {
	char *dst;
	const char *src;
	int n, overflow = 0;

	strncpy(q, rep, len_overlap);

	if (len_rep < len_what) {
	    //
	    //   ....WHATabcdef
	    //   ....REPTabcdef
	    //      q^  ||
	    //       dst^|
	    //        src^
	    //
	    //   ....REPabcdef
	    //
	    // shift rest down
	    dst = q + len_overlap;
	    src = q + len_what;
	    n = buf + len + 1 - src; // include terminating \0
	    memmove(dst, src, n);

	    // result gets shorter
	    len -= len_what - len_rep;
	} else if (len_rep > len_what) {
	    //
	    //   ....Wabcdef
	    //   ....Rabcdef
	    //      q^| |
	    //     src^ |
	    //       dst^
	    //
	    //   ....R__abcdef
	    //   ....REPabcdef
	    //
	    // shift rest up
	    dst = q + len_rep;
	    src = q + len_overlap;
	    n = buf + len + 1 - src; // include terminating \0
	    if (dst + n - 1 >= buf + size - 1) {
		// would be longer than (size-1), shift only a part
		n = buf + size - 1 - dst;
		if (n <= 0) {
		    // even a part won't fit; no operation
		    n = 0;
		    overflow = 1;
		}
	    }
	    memmove(dst, src, n);

	    // copy tail of rep
	    dst = q + len_overlap;
	    src = rep + len_overlap;
	    n = len_rep - len_what;
	    if (dst + n - 1 >= buf + size - 1) {
		// only a part of rep fits
		n = buf + size - 1 - dst;
		overflow = 1;
	    }
	    memcpy(dst, src, n);

	    if (overflow) {
		break;
	    }

	    // result gets longer
	    len += len_rep - len_what;
	}
	p = q + len_rep;
    }
}

void replace_1(char *buf, int size, const char *what, const char *rep) {
    replace_n(buf, size, what, rep, 1);
}

void replace_all(char *buf, int size, const char *what, const char *rep) {
    replace_n(buf, size, what, rep, 999);
}

void ExpandQsoNumber(char *qsonr) {
    static char qsonroutput[5] = "";
    int leading_zeros = 0;
    bool lead = true;
    for (int i = 0; i <= 4; i++) {
        if (lead && qsonr[i] == '0') {
            ++leading_zeros;
        } else {
            lead = false;
        }
        qsonroutput[i] = short_number(qsonr[i]);
    }
    qsonroutput[4] = '\0';

    if (!noleadingzeros && leading_zeros > 1) {
        leading_zeros = 1;
    }

    replace_all(buffer, BUFSIZE, "#",
            qsonroutput + leading_zeros);   /* serial nr */
}

void ExpandRst(char rst[4]) {
    static char rst_out[4] = "";
    rst_out[0] = rst[0];
    rst_out[1] = short_number(rst[1]);
    rst_out[2] = short_number(rst[2]);
    rst_out[3] = '\0';

    replace_all(buffer, BUFSIZE, "[", rst_out);   /* his RST */
}

void ExpandMacro_CurrentQso(void) {

    replace_all(buffer, BUFSIZE, "%", my.call);   /* mycall */

    if (NULL != strstr(buffer, "@")) {
	char *p = current_qso.call + strlen(hiscall_sent);
	if (strlen(hiscall_sent) != 0) {
	    hiscall_sent[0] = '\0';
	    early_started = 0;
	}
	if (cqmode == CQ && resend_call != RESEND_NOT_SET) {
	    strcpy(sentcall, current_qso.call);
	}
	replace_1(buffer, BUFSIZE, "@", p);   /* his call, 1st occurrence */
	replace_all(buffer, BUFSIZE, "@",
		    current_qso.call);   /* his call, further occurrences */
    }

    ExpandRst(sent_rst);

    if (NULL != strstr(buffer, "#")) {
        ExpandQsoNumber(qsonrstr);

	if (lan_active && contest->exchange_serial) {
	    strncpy(lastqsonr, qsonrstr, 5);
	    send_lan_message(INCQSONUM, qsonrstr);
	}
    }


    replace_all(buffer, BUFSIZE, "!", current_qso.comment);

    if (trxmode == DIGIMODE)
	replace_all(buffer, BUFSIZE, "|", "\r");    /* CR */
    else
	replace_all(buffer, BUFSIZE, "|", "");	    /* drop it */
}

void ExpandMacro_PreviousQso(void) {
    replace_all(buffer, BUFSIZE, "%", my.call);   /* mycall */

    struct qso_t *prev_qso;

    struct qso_t empty_qso;
    char empty_str[1];
    empty_str[0] = '\0';
    empty_qso.call = empty_str;
    empty_qso.qso_nr = 0;

    if (NR_QSOS == 0) {
        prev_qso = &empty_qso;
    } else {
        prev_qso = g_ptr_array_index(qso_array, NR_QSOS - 1);
    }

    if (NULL != strstr(buffer, "@")) {
	replace_all(buffer, BUFSIZE, "@", prev_qso->call);
    }

    ExpandRst(last_rst);

    if (NULL != strstr(buffer, "#")) {
        char *prevnr = g_strdup_printf("%04d", prev_qso->qso_nr);
        ExpandQsoNumber(prevnr);
        g_free(prevnr);
    }
}


void sendbuf(ExpandMacro_t expandMacro) {

    if ((trxmode == CWMODE && cwkeyer != NO_KEYER) ||
	    (trxmode == DIGIMODE && digikeyer != NO_KEYER)) {

	expandMacro();

	if (!simulator) {
	    if (sending_call == 0)
		add_to_keyer_terminal(buffer);
	}

	if (trxmode == DIGIMODE) {

	    if (digikeyer == MFJ1278_KEYER) {
		int i = 0;
		for (i = 0; i < strlen(buffer); i++)
		    if (buffer[i] == '\n')
			buffer[i] = RETURN;
		for (i = 0; i < strlen(buffer); i++)
		    if (buffer[i] == 123)
			buffer[i] = 20;	/* ctrl-t */
		for (i = 0; i < strlen(buffer); i++)
		    if (buffer[i] == 125)
			buffer[i] = CTRL_R;	/* ctrl-r */
	    }
	    keyer_append(buffer);
	}
	if (trxmode == CWMODE) {

	    if (cwkeyer == MFJ1278_KEYER) {
		int i = 0;
		for (i = 0; i < strlen(buffer); i++)
		    if (buffer[i] == '\n')
			buffer[i] = RETURN;
	    }
	    keyer_append(buffer);
	}

	if (simulator) {
	    set_simulator_state(REPEAT);
	}

	buffer[0] = '\0';
    }
}


/** \brief send message
 *
 * Send the message via CW or DIGI mode, but only if not empty
 * \param msg message to send
 */
void sendmessage_with_macro_expand(const char *msg, ExpandMacro_t expandMacro) {
    if (strlen(msg) != 0) {
	g_strlcpy(buffer, msg, sizeof(buffer));
	sendbuf(expandMacro);
    }
}

void sendmessage(const char *msg) {
    sendmessage_with_macro_expand(msg, ExpandMacro_CurrentQso);
}

void send_standard_message_with_macro_expand(int msg, ExpandMacro_t expandMacro) {
    switch (trxmode) {
	case CWMODE:
	    sendmessage_with_macro_expand(message[msg], expandMacro);
	    break;
	case DIGIMODE:
	    sendmessage_with_macro_expand(digi_message[msg], expandMacro);
	    break;
	default:
	    if (msg < 14)
		vk_play_file(ph_message[msg]);
	    break;
    }
}

void send_standard_message(int msg) {
    send_standard_message_with_macro_expand(msg, ExpandMacro_CurrentQso);
}

void send_keyer_message(int msg) {
    switch (trxmode) {
	case DIGIMODE:
	    sendmessage(digi_message[msg]);
	    break;
	default:
	    sendmessage(message[msg]);
	    break;
    }
}
