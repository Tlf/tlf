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


void prepare_fixed_part(char *logline, struct qso_t *qso);
void prepare_specific_part(char *logline, struct qso_t *qso);


static int standard_rst(struct qso_t *qso) {
    return qso->mode == SSBMODE ? 59 : 599;
}

/*
 * fill log line with spaces until column n
 */
#define FILL_TO(n)  strcat(logline, spaces((n) - strlen(logline)))


/** Construct a new line to add to the logfile.
 *
 * Prepare a logline for storage in log
 * Returned value must be g_free'd after use
 *
 * The structure of a logline entry is as follows:
 * - each logline contains exactly 87 characters followed by a newline.
 * - it consists of 3 parts
 *   fixed part (54 chars) | contest dependent part (26 chars) | freq (7 chars)
 *
 *   See function definitions below
 */
char *makelogline(struct qso_t *qso) {
    char freq_buff[10];

    char *logline = g_malloc0(sizeof(logline4));

    /* first fixed (contest independent) part of logline */
    prepare_fixed_part(logline, qso);
    assert(strlen(logline) == 54);

    /* second (contest dependent) part of logline */
    prepare_specific_part(logline, qso);
    assert(strlen(logline) == 77);

    /* add points to logline if in contest */
    if (iscontest && !CONTEST_IS(DXPED)) {
	sprintf(logline + 76, "%2d", qso_points);   //FIXME qso->score
    }

    FILL_TO(80);

    /* add freq to end of logline */
    if (qso->freq > 0) {
	snprintf(freq_buff, 8, "%7.1f", qso->freq / 1000.0);
	strcat(logline, freq_buff);
    }
    FILL_TO(87);

    assert(strlen(logline) == 87);

    return logline;
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
void prepare_fixed_part(char *logline, struct qso_t *qso) {
    char buf[80];

    strcpy(logline, band[qso->bandindex]);

    if (qso->mode == CWMODE)
	strcat(logline, "CW ");
    else if (qso->mode == SSBMODE)
	strcat(logline, "SSB");
    else
	strcat(logline, "DIG");

    struct tm time_tm;
    gmtime_r(&qso->timestamp, &time_tm);
    strftime(buf, sizeof(buf), " "DATE_TIME_FORMAT" ", &time_tm);
    strcat(logline, buf);

    qsonr_to_str(buf, qso->qso_nr);
    if (logfrequency && qso->freq > 0 &&
	    ((strcmp(whichcontest, "qso") == 0) ||
	     (strcmp(whichcontest, "dxped") == 0))) {
	char khz[5];
	sprintf(khz, " %3u", ((unsigned int)(freq / 1000.0)) % 1000);	// show freq.
	strcat(logline, khz);

    } else {
	if (lan_active && contest->exchange_serial) {	// show qso nr
	    strcat(logline, lastqsonr);
	    lastqsonr[0] = '\0';
	} else
	    strcat(logline, buf);
    }

    if (lan_active && cqwwm2) {
	logline[27] = thisnode;	// set node ID...
	logline[28] = '\0';
	strcat(logline, spaces(1));
    } else
	strcat(logline, spaces(2));
    /* goes till 29 */

    g_strlcat(logline, qso->call, 44 + 1);

    FILL_TO(44);

    if (no_rst) {
	strcat(logline, "---  ---  ");	/* instead of RST */
    } else {
	sprintf(buf, "%-3d  %-3d  ",
		qso->rst_s ? qso->rst_s : standard_rst(qso),
		qso->rst_r ? qso->rst_r : standard_rst(qso));
	strcat(logline, buf);
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
void prepare_specific_part(char *logline, struct qso_t *qso) {
    char *tmp;

    if (CONTEST_IS(CQWW) || wazmult || itumult) {
	//-------------------------cqww----------------
	char buf[80];
	strcpy(buf, qso->comment);
	if (qso->mode == DIGIMODE && CONTEST_IS(CQWW) && strlen(buf) < 5) {
	    buf[2] = ' ';
	    buf[3] = 'D';
	    buf[4] = 'X';
	    buf[5] = '\0';
	}
	tmp = g_strndup(buf, 22);
	strcat(logline, tmp);
	g_free(tmp);
    } else {	//----------------------end cqww ---------------
	tmp = g_strndup(qso->comment, 22);
	strcat(logline, tmp);
	g_free(tmp);
    }

    FILL_TO(77);

    if (!iscontest) {
        return;         // not a contest, we are done
    }

    logline[68] = '\0'; /* cut back to make room for mults */

    if (CONTEST_IS(WPX) || pfxmult || pfxmultab) {	/* wpx */
	// include new pfx in log line
	if (new_pfx) {  //FIXME global
	    /** \todo FIXME: prefix can be longer than 5 char, e.g. LY1000 */
	    strncat(logline, wpx_prefix, 5);    //FIXME global
	}

	FILL_TO(73);

    } else if (CONTEST_IS(CQWW) || wazmult || itumult) {
	/* ------------cqww --------------------- */

	if (new_cty != 0) {     //FIXME global
	    if (dxcc_by_index(new_cty)->pfx[0] == '*')
		strncat(logline, dxcc_by_index(new_cty) -> pfx + 1, 5);
	    else
		strncat(logline, dxcc_by_index(new_cty) -> pfx, 5);

	    new_cty = 0;
	}

	FILL_TO(73);

	if (new_zone != 0) {    //FIXME global
	    tmp = g_strdup_printf("%02d", new_zone);
	    strcat(logline, tmp);
	    g_free(tmp);

	    new_zone = 0;
	}

	//----------------------------------end cqww-----------------

    } else if (CONTEST_IS(ARRLDX_USA)) {

	if (new_cty != 0) {
	    strncat(logline, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;
	}

    } else if (dx_arrlsections && (countrynr != w_cty)
	       && (countrynr != ve_cty)) {

	if (new_cty != 0) {
	    strncat(logline, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;
	}

    } else if (wysiwyg_multi
	       || unique_call_multi != MULT_NONE
	       || serial_section_mult
	       || sectn_mult
	       || sectn_mult_once
	       || serial_grid4_mult) {

	if (new_mult >= 0) {    //FIXME global
	    strncat(logline, multis[new_mult].name, 9);

	    new_mult = -1;
	}

    } else if (dx_arrlsections
	       && ((countrynr == w_cty) || (countrynr == ve_cty))) {

	if (new_mult >= 0) {
	    strncat(logline, multis[new_mult].name, 9);

	    new_mult = -1;
	}

    } else if (CONTEST_IS(PACC_PA) || pfxnummultinr > 0) {

	if (new_cty != 0) {
	    strncat(logline, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;

	} else if (addcallarea == 1) {
	    strncat(logline, wpx_prefix, 3);

	    addcallarea = 0;
	}

    } else if (country_mult || dx_arrlsections) {

	if (new_cty != 0) {
	    strncat(logline, dxcc_by_index(new_cty) -> pfx, 9);

	    new_cty = 0;
	}

    }

    FILL_TO(77);
}
