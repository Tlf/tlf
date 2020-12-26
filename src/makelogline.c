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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* ------------------------------------------------------------
 *        Makelogline takes care of formatting the log
 *                       items
 *--------------------------------------------------------------*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addpfx.h"
#include "dxcc.h"
#include "get_time.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "qsonr_to_str.h"
#include "score.h"
#include "setcontest.h"


void prepare_fixed_part(void);
void prepare_specific_part(void);
void fillto(int n);



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
    extern int trx_control;
    extern freq_t freq;

    static int lastbandinx = 0;
    char freq_buff[10];
    int points;

    /* restart band timer if qso on new band */
    if (CONTEST_IS(WPX)) {		// 10 minute timer
	if (lastbandinx != bandinx) {
	    lastbandinx = bandinx;
	    minute_timer = 600;	// 10 minutes
	}
    }

    /* remember call for resend after qso (see callinput.c)  */
    strcpy(lastcall, hiscall);

    /* first fixed (contest independent) part of logline */
    prepare_fixed_part();
    assert(strlen(logline4) == 54);

    /* second (contest dependent) part of logline */
    prepare_specific_part();
    assert(strlen(logline4) == 77);

    /* score QSO and add to logline
     * if not DXpedition or QSO mode */
    points = score();			/* update qso's per band and score */
    total = total + points;

    if (iscontest && !CONTEST_IS(DXPED)) {
	sprintf(logline4 + 76, "%2d", points);
    }

    fillto(80);

    /* add freq to end of logline */
    if (trx_control == 1) {
	snprintf(freq_buff, 8, "%7.1f", freq / 1000.0);
	strcat(logline4, freq_buff);
    }
    fillto(87);

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
    extern int no_rst;
    extern char whichcontest[];
    extern int logfrequency;
    extern int trx_control;
    extern freq_t freq;

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
    if (logfrequency == 1 &&
	    trx_control == 1 &&
	    ((strcmp(whichcontest, "qso") == 0) ||
	     (strcmp(whichcontest, "dxped") == 0))) {
	char khz[5];
	sprintf(khz, " %3d", ((unsigned int)(freq / 1000.0)) % 1000);	// show freq.
	strcat(logline4, khz);

    } else {
	if (lan_active && (exchange_serial == 1)) {	// show qso nr
	    strcat(logline4, lastqsonr);
	    lastqsonr[0] = '\0';
	} else
	    strcat(logline4, qsonrstr);
    }

    if (lan_active && cqwwm2 == 1) {
	logline4[27] = thisnode;	// set node ID...
	logline4[28] = '\0';
	strcat(logline4, " ");
    } else
	strcat(logline4, "  ");
    /* goes till 29 */

    g_strlcat(logline4, hiscall, 44 + 1);

    fillto(44);

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
    extern int pfxnummultinr;
    extern int excl_add_veto;
    int new_pfx;
    int sr_nr = 0;
    char grid[7] = "";
    int i;

    if (CONTEST_IS(ARRL_SS)) {
	// ----------------------------arrlss----------------
	strncat(logline4, ssexchange, 22);
	section[0] = '\0';

    } else if (serial_section_mult == 1) {
	//-------------------------serial_section---------------
	strncat(logline4, comment, 22);
	section[0] = '\0';

    } else if (serial_grid4_mult == 1) {
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

    } else if (sectn_mult == 1) {
	//-------------------------section only---------------
	strncat(logline4, comment, 22);
	section[0] = '\0';

    } else if (CONTEST_IS(CQWW) || (wazmult == 1) || (itumult == 1)) {
	//-------------------------cqww----------------
	/*
		if (strlen(zone_fix) > 1) {
			strcat (logline4, zone_fix);
		} else
			strcat (logline4, zone_export);
	*/
	if (trxmode == DIGIMODE && CONTEST_IS(CQWW) && strlen(comment) < 5) {
	    comment[2] = ' ';
	    comment[3] = 'D';
	    comment[4] = 'X';
	    comment[5] = '\0';
	}
	strncat(logline4, comment, 22);
    } else {	//----------------------end cqww ---------------
	strncat(logline4, comment, 22);
    }

    fillto(77);

    if (iscontest) 		/* cut back to make room for mults */
	logline4[68] = '\0';

    /* If WPX
     * -> add prefix to prefixes_worked and include new pfx in log line */
    new_pfx = 0;
    if (!(pfxmultab == 1 && excl_add_veto == 1)) {
	new_pfx = (add_pfx(pxstr, bandinx) == 0);	/* add prefix, remember if new */
    }

    if (CONTEST_IS(WPX) ||pfxmult == 1 || pfxmultab == 1) {	/* wpx */
	if (new_pfx) {
	    /** \todo FIXME: prefix can be longer than 5 char, e.g. LY1000 */
	    strncat(logline4, pxstr, 5);
	}

	fillto(73);
    }

    if (CONTEST_IS(CQWW) || (wazmult == 1) || (itumult == 1)) {
	/* ------------cqww --------------------- */
	logline4[68] = '\0';

	if (addcty != 0) {
	    if (dxcc_by_index(addcty)->pfx[0] == '*')
		strncat(logline4, dxcc_by_index(addcty) -> pfx + 1, 5);
	    else
		strncat(logline4, dxcc_by_index(addcty) -> pfx, 5);

	    addcty = 0;
	}

	fillto(73);

	if (addzone != 0) {
	    /*
	    		if (strlen(zone_fix) > 1) {
	    			strncat (logline4, zone_fix, 2);
	    		} else
	    			strncat (logline4, zone_export, 2);
	    */
	    if (strlen(comment) < 2) {
		strcat(logline4, "0");
		strncat(logline4, comment, 1);
	    } else
		strncat(logline4, comment, 2);

	    addzone = 0;
	} else {
	    zone_fix[0] = '\0';
	}

	fillto(77);

	//----------------------------------end cqww-----------------

    } else if (CONTEST_IS(ARRLDX_USA)) {
	logline4[68] = '\0';
	if (addcty != 0) {
	    strncat(logline4, dxcc_by_index(addcty) -> pfx, 9);

	    addcty = 0;
	}

	fillto(77);

    } else if ((dx_arrlsections == 1) && (countrynr != w_cty)
	       && (countrynr != ve_cty)) {
	logline4[68] = '\0';

	if (addcty != 0) {
	    strncat(logline4, dxcc_by_index(addcty) -> pfx, 9);

	    addcty = 0;
	}

	fillto(77);

    } else if ((wysiwyg_multi == 1)
	       || (unique_call_multi != 0)
	       || (serial_section_mult == 1)
	       || (sectn_mult == 1)
	       || (sectn_mult_once == 1)
	       || (serial_grid4_mult)) {

	logline4[68] = '\0';

	if (shownewmult >= 0) {
	    strncat(logline4, multis[shownewmult].name, 9);

	    shownewmult = -1;
	}

	fillto(77);

    } else if ((dx_arrlsections == 1)
	       && ((countrynr == w_cty) || (countrynr == ve_cty))) {
	logline4[68] = '\0';

	if (shownewmult >= 0) {
	    strncat(logline4, multis[shownewmult].name, 9);

	    shownewmult = -1;
	}

	fillto(77);

    } else if (CONTEST_IS(PACC_PA) || pfxnummultinr > 0) {

	logline4[68] = '\0';

	if (addcty != 0) {
	    strncat(logline4, dxcc_by_index(addcty) -> pfx, 9);

	    addcty = 0;

	} else if (addcallarea == 1) {
	    strncat(logline4, pxstr, 3);

	    addcallarea = 0;
	}

	fillto(77);

    } else if (iscontest
	       && ((country_mult == 1) || (dx_arrlsections == 1))) {

	logline4[68] = '\0';

	if (addcty != 0) {
	    strncat(logline4, dxcc_by_index(addcty) -> pfx, 9);

	    addcty = 0;
	}

	fillto(77);

    }

    fillto(77);
}


/** fill logline4 with spaces
 *
 * fill logline4 with spaces until column n
 */
void fillto(int n) {
    char fillspaces[] = "                                                    ";
    int len = strlen(logline4);

    if (len < n)
	strncat(logline4, fillspaces, n - len);
}

