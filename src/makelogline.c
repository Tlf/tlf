/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
 *               2011-2014                Thomas Beierlein <tb@forth-ev.de>
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
 *        Makelogline takes care of formatting the log
 *                       items
 *--------------------------------------------------------------*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addcall.h"
#include "addpfx.h"
#include "dxcc.h"
#include "get_time.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "lancode.h"
#include "qsonr_to_str.h"
#include "setcontest.h"
#include "ui_utils.h"


void prepare_fixed_part(void);
void prepare_specific_part(void);


/*
 * fill log line with spaces until column n
 */
#define FILL_TO(n)  strcat(logline4, spaces((n) - strlen(logline4)))


/** Construct a new line to add to the logfile.
 *
 * Prepare a logline for storage in log in 'logline4'
 *
 * The structure of a logline entry is as follows:
 * - each logline contains exactly 87 characters followed by a newline.
 * - it consists of 3 parts
 *   fixed part (54 chars) | contest dependent part (26 chars) | freq (7 chars)
 *
 *   See function definitions below
 */
void makelogline(void) {
    char freq_buff[10];

    /* first fixed (contest independent) part of logline */
    prepare_fixed_part();
    assert(strlen(logline4) == 54);

    /* second (contest dependent) part of logline */
    prepare_specific_part();
    assert(strlen(logline4) == 77);

    /* add points to logline if in contest */
    if (iscontest && !CONTEST_IS(DXPED)) {
	sprintf(logline4 + 76, "%2d", qso_points);
    }

    FILL_TO(80);

    /* add freq to end of logline */
    if (trx_control) {
	snprintf(freq_buff, 8, "%7.1f", freq / 1000.0);
	strcat(logline4, freq_buff);
    }
    FILL_TO(87);

    assert(strlen(logline4) == 87);
}


/** Construct fixed part of logline
 *
 * - fixed part:\n
 *     \verbatim
 *     0        1         2         3         4         5
 *     123456789012345678901234567890123456789012345678901234
 *     bndmod dd-mmm-yy hh:mm qso.  call.......... rst  rst
 *                            nr..                 his  my.\endverbatim
 *   Alternatively in 'qso' or 'dxped' mode if 'LOGFREQUENCY' is set
 *     there is the khz part of the working frequnecy instead of
 *     the qso number:
 *     \verbatim
 *     0        1         2         3         4         5
 *     123456789012345678901234567890123456789012345678901234
 *     bndmod dd-mmm-yy hh:mm  khz  call.......... rst  rst
 *                                                 his  my.\endverbatim
 */
void prepare_fixed_part(void) {
    static char time_buf[80];

    strcpy(logline4, band[bandinx]);

    if (trxmode == CWMODE)
	strcat(logline4, "CW ");
    else if (trxmode == SSBMODE)
	strcat(logline4, "SSB");
    else
	strcat(logline4, "DIG");

    if (do_cabrillo == 0) {
	format_time(time_buf, sizeof(time_buf), " "DATE_TIME_FORMAT" ");
    } else {
	strftime(time_buf, sizeof(time_buf), " "DATE_TIME_FORMAT" ",
		 &time_ptr_cabrillo);
    }
    strcat(logline4, time_buf);

    qsonr_to_str();
    if (logfrequency && trx_control &&
	    ((strcmp(whichcontest, "qso") == 0) ||
	     (strcmp(whichcontest, "dxped") == 0))) {
	char khz[5];
	sprintf(khz, " %3u", ((unsigned int)(freq / 1000.0)) % 1000);	// show freq.
	strcat(logline4, khz);

    } else {
	if (lan_active && (contest->exchange_serial)) {	// show qso nr
	    strcat(logline4, lastqsonr);
	    lastqsonr[0] = '\0';
	} else
	    strcat(logline4, qsonrstr);
    }

    if (lan_active && cqwwm2) {
	logline4[27] = thisnode;	// set node ID...
	logline4[28] = '\0';
	strcat(logline4, " ");
    } else
	strcat(logline4, "  ");
    /* goes till 29 */

    g_strlcat(logline4, hiscall, 44 + 1);

    FILL_TO(44);

    if (no_rst) {
	strcat(logline4, "---  ---  ");	/* instead of RST */
    } else {
	if ((trxmode == CWMODE) || (trxmode == DIGIMODE)) {
	    sent_rst[2] = '9';
	    recvd_rst[2] = '9';
	} else {
	    sent_rst[2] = ' ';
	    recvd_rst[2] = ' ';
	}

	strcat(logline4, sent_rst);	/* till 54 */
	strcat(logline4, "  ");
	strcat(logline4, recvd_rst);
	strcat(logline4, "  ");
    }
}


/** Construct contest dependent part of logline
 *
 * - contest dependent part (list may not be complete):\n
 *   - QSO mode
 *     \verbatim
 *     5    6         7         8
 *     56789012345678901234567890
 *     comment................\endverbatim
 *   - wpx
 *     \verbatim
 *     5    6         7         8
 *     56789012345678901234567890
 *     serialnr      pfx     pp\endverbatim
 *     pfx - new prefix, pp -points
 *   - cqww
 *     \verbatim
 *     5    6         7         8
 *     56789012345678901234567890
 *     hiszone       pfx  zn pp\endverbatim
 *     zn - new zone
 *   - normal contest
 *     \verbatim
 *     5    6         7         8
 *     56789012345678901234567890
 *     exchange      mult    pp\endverbatim
 *     mult - multi (cty, province, ...)
 *   - arllss
 *     \verbatim
 *     5    6         7         8
 *     56789012345678901234567890
 *     nr.. p cc sctn        pp\endverbatim
 *     nr - serial exchange, p - precedent, cc - check, sctn - ARRL/RAC section
 *   - arrlfd
 *     5    6         7         8
 *     56789012345678901234567890
 *     class         sctn    pp\endverbatim
 *     class - TX count + operator class, sctn - ARRL/RAC section
 */
void prepare_specific_part(void) {
    int sr_nr = 0;
    char grid[7] = "";
    int i;
    char *tmp;

    if (CONTEST_IS(ARRL_SS)) {
	// ----------------------------arrlss----------------
	tmp = g_strndup(comment, 22);
	strcat(logline4, tmp);
	g_free(tmp);
	section[0] = '\0';

    } else if (serial_section_mult) {
	//-------------------------serial_section---------------
	tmp = g_strndup(comment, 22);
	strcat(logline4, tmp);
	g_free(tmp);
	section[0] = '\0';

    } else if (serial_grid4_mult) {
	//-------------------------serial_grid 4 characters---------------
	sr_nr = atoi(comment);
	for (i = 0; i < 11; i++) {
	    if (comment[i] > 64 && comment[i] < 91) {
		break;
	    }
	}
	strncpy(grid, comment + i, 6);
	grid[6] = '\0';

	sprintf(logline4 + 54, "%4.0d %s", sr_nr, grid);
	section[4] = '\0';

    } else if (sectn_mult) {
	//-------------------------section only---------------
	tmp = g_strndup(comment, 22);
	strcat(logline4, tmp);
	g_free(tmp);
	section[0] = '\0';

    } else if (CONTEST_IS(CQWW) || wazmult || itumult) {
	//-------------------------cqww----------------
	if (trxmode == DIGIMODE && CONTEST_IS(CQWW) && strlen(comment) < 5) {
	    comment[2] = ' ';
	    comment[3] = 'D';
	    comment[4] = 'X';
	    comment[5] = '\0';
	}
	tmp = g_strndup(comment, 22);
	strcat(logline4, tmp);
	g_free(tmp);
    } else {	//----------------------end cqww ---------------
	tmp = g_strndup(comment, 22);
	strcat(logline4, tmp);
	g_free(tmp);
    }

    FILL_TO(77);

    if (iscontest) 		/* cut back to make room for mults */
	logline4[68] = '\0';

    if (CONTEST_IS(WPX) || pfxmult || pfxmultab) {	/* wpx */
	// include new pfx in log line
	if (new_pfx) {
	    /** \todo FIXME: prefix can be longer than 5 char, e.g. LY1000 */
	    strncat(logline4, wpx_prefix, 5);
	}

	FILL_TO(73);
    }

    if (CONTEST_IS(CQWW) || wazmult || itumult) {
	/* ------------cqww --------------------- */
	logline4[68] = '\0';

	if (new_cty != 0) {
	    if (dxcc_by_index(new_cty)->pfx[0] == '*')
		strncat(logline4, dxcc_by_index(new_cty) -> pfx + 1, 5);
	    else
		strncat(logline4, dxcc_by_index(new_cty) -> pfx, 5);

	    new_cty = 0;
	}

	FILL_TO(73);

	if (new_zone != 0) {
	    if (strlen(comment) < 2) {
		strcat(logline4, "0");
		strncat(logline4, comment, 1);
	    } else
		strncat(logline4, comment, 2);

	    new_zone = 0;
	}

	//----------------------------------end cqww-----------------

    } else if (CONTEST_IS(ARRLDX_USA)) {
	logline4[68] = '\0';
	if (new_cty != 0) {
	    strncat(logline4, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;
	}

    } else if (dx_arrlsections && (countrynr != w_cty)
	       && (countrynr != ve_cty)) {
	logline4[68] = '\0';

	if (new_cty != 0) {
	    strncat(logline4, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;
	}

    } else if (wysiwyg_multi
	       || (unique_call_multi != 0)
	       || serial_section_mult
	       || sectn_mult
	       || sectn_mult_once
	       || serial_grid4_mult) {

	logline4[68] = '\0';

	if (new_mult >= 0) {
	    strncat(logline4, multis[new_mult].name, 9);

	    new_mult = -1;
	}

    } else if (dx_arrlsections
	       && ((countrynr == w_cty) || (countrynr == ve_cty))) {
	logline4[68] = '\0';

	if (new_mult >= 0) {
	    strncat(logline4, multis[new_mult].name, 9);

	    new_mult = -1;
	}

    } else if (CONTEST_IS(PACC_PA) || pfxnummultinr > 0) {

	logline4[68] = '\0';

	if (new_cty != 0) {
	    strncat(logline4, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;

	} else if (addcallarea == 1) {
	    strncat(logline4, wpx_prefix, 3);

	    addcallarea = 0;
	}

    } else if (iscontest
	       && (country_mult || dx_arrlsections)) {

	logline4[68] = '\0';

	if (new_cty != 0) {
	    strncat(logline4, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;
	}

    }

    FILL_TO(77);
}
