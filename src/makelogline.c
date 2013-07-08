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
	/* ------------------------------------------------------------
	 *        Makelogline takes care of formatting the log
	 *                       items
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "makelogline.h"
#include "dxcc.h"

#include <assert.h>

/** Construct a new line to add to the logfile.
 *
 * The structure of a logline entry is as follows:
 * - each logline contains exactly 80 characters
 * - it consists of 2 parts
 *   | fixed part one (54 chars) | contest dependent part 2 (26 chars) |
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
 * - contest dependent part (list may not complete):\n
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
 *     hiszone       pfx zn  pp\endverbatim
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

void makelogline(void)
{

    extern int logfrequency;
    extern int trx_control;
    extern char whichcontest[];
    extern float freq;
    extern int points;
    extern int no_rst;

    static char time_buf[80];
    char fillspaces[50] = "                              ";
    static int lastbandinx = 0;
    char grid[7] = "";
    int sr_nr = 0;
    int i;
    char khz[5] = " 000";
    char freq_buff[10];
    int fnr = 0;
    int new_pfx;

    /* first fixed (contest independent) part */

    logline4[0] = '\0';
    qsonr_to_str();
    get_time();

    strftime(time_buf, 60, " %d-%b-%y %H:%M ", time_ptr);

    if (wpx == 1) {		// 10 minute timer
	if (lastbandinx != bandinx) {
	    lastbandinx = bandinx;
	    minute_timer = 600;	// 10 minutes
	}
    }

    strcat(logline4, band[bandinx]);

    if (trxmode == CWMODE)
	strcat(logline4, "CW ");
    else if (trxmode == SSBMODE)
	strcat(logline4, "SSB");
    else
	strcat(logline4, "DIG");

    strcat(logline4, time_buf);

    if (logfrequency == 1 &&
	trx_control == 1 &&
	((strcmp(whichcontest, "qso") == 0) ||
	 (strcmp(whichcontest, "dxped") == 0))) {
	fnr = (int) freq;
	fnr = fnr % 1000;
	sprintf(khz, " %3d", fnr);	// show freq.
	strcat(logline4, khz);

    } else {
	if ((lan_active == 1) && (exchange_serial == 1)) {	// show qso nr
	    strcat(logline4, lastqsonr);
	    lastqsonr[0] = '\0';
	} else
	    strcat(logline4, qsonrstr);
    }

    if (lan_active == 1 && cqwwm2 == 1) {
	logline4[27] = thisnode;	// set node ID...
	logline4[28] = '\0';
	strcat(logline4, " ");
    } else
	strcat(logline4, "  ");

    strcat(logline4, hiscall);	/*  29 */
    lastcall[0] = '\0';		/* remember  for edit  */
    strcat(lastcall, hiscall);
    strncat(logline4, fillspaces, (15 - strlen(hiscall)));	/*  44 */

    if (!no_rst) {
	if ((trxmode == CWMODE) || (trxmode == DIGIMODE)) {
	    his_rst[2] = '9';
	    my_rst[2] = '9';
	} else {
	    his_rst[2] = ' ';
	    my_rst[2] = ' ';
	}

	strcat(logline4, his_rst);	/* till 54 */
	strcat(logline4, "  ");
	strcat(logline4, my_rst);
	strcat(logline4, "  ");
    } else {
	strcat(logline4, "---  ---  ");	/* instead of RST */
    }

    /* second (contest dependent part of logline */

    if (arrlss == 1) {		
	// ----------------------------arrlss----------------
	strcat(logline4, ssexchange);
	section[0] = '\0';

    } else if (serial_section_mult == 1) {
	//-------------------------serial_section---------------
	sprintf(logline4 + 54, "%s", comment);
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
	sprintf(logline4 + 54, "%s", section);
	section[0] = '\0';

    } else if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
	//-------------------------cqww----------------
/*
	if (strlen(zone_fix) > 1) {
		strcat (logline4, zone_fix);
	} else
		strcat (logline4, zone_export);
*/
	strcat(logline4, comment);
    } else {
	strcat(logline4, comment);
    }				//----------------------end cqww ---------------

    lastcomment[0] = '\0';
    strcat(lastcomment, comment);	/* remember  for edit  */

    strncat(logline4, fillspaces, (77 - strlen(logline4)));

    if (contest == 1)
	logline4[68] = '\0';

    /* If WPX 
     * -> add prefix to prefixes_worked and include new pfx in log line */
    new_pfx = (add_pfx(pxstr) == 0);	/* add prefix, remember if new */

    if (wpx ==1) {			/* wpx */
	if (new_pfx) {
	    strcat(logline4, pxstr);
	    strncat(logline4, fillspaces, (5 - strlen(pxstr)));
	} else
	    strncat(logline4, fillspaces, 5);
    }

    if ((cqww == 1) || (wazmult == 1) || (itumult == 1)) {
	/* ------------cqww --------------------- */
	logline4[68] = '\0';

	if (addcty != 0) {
	    strcat(logline4, dxcc_by_index(addcty) -> pfx);

	    strncat(logline4, fillspaces, 72 - strlen(logline4));
	    addcty = 0;
	} else {
	    strcat(logline4, "    ");
	}

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

	    strcat(logline4, "   ");
	    addzone = 0;
	} else {
	    strcat(logline4, "     ");
	    zone_fix[0] = '\0';
	}
	//----------------------------------end cqww-----------------

    } else if (arrldx_usa == 1) {
	logline4[68] = '\0';
	if (addcty != 0) {
	    strcat(logline4, dxcc_by_index(addcty) -> pfx);
	    
	    strncat(logline4, fillspaces, 77 - strlen(logline4));
	    addcty = 0;

	} else
	    strncat(logline4, fillspaces, 9);

    } else if ((dx_arrlsections == 1) && (countrynr != w_cty)
	       && (countrynr != ve_cty)) {
	logline4[68] = '\0';

	if (addcty != 0) {
	    strcat(logline4, dxcc_by_index(addcty) -> pfx);

	    strncat(logline4, fillspaces, 77 - strlen(logline4));
	    addcty = 0;

	} else {
	    strncat(logline4, fillspaces, 9);
	}
    } else if ((wysiwyg_multi == 1)
	       || (serial_section_mult == 1)
	       || (sectn_mult == 1)
	       || (serial_grid4_mult)) {

	logline4[68] = '\0';

	if (shownewmult >= 0) {

	    strcat(logline4, mults[shownewmult]);

	    strncat(logline4, "              |", 77 - strlen(logline4));
	    shownewmult = -1;

	} else
	    strncat(logline4, fillspaces, 9);
    } else if ((dx_arrlsections == 1)
	       && ((countrynr == w_cty) || (countrynr == ve_cty))) {
	logline4[68] = '\0';

	if (shownewmult >= 0) {

	    strcat(logline4, mults[shownewmult]);

	    strncat(logline4, "              |", 77 - strlen(logline4));
	    shownewmult = -1;

	} else
	    strncat(logline4, fillspaces, 9);

    } else if (pacc_pa_flg == 1) {

	logline4[68] = '\0';

	if (addcty != 0) {
	    strcat(logline4, dxcc_by_index(addcty) -> pfx);

	    strncat(logline4, fillspaces, 77 - strlen(logline4));
	    addcty = 0;

	} else if (addcallarea == 1) {

	    strcat(logline4, pxstr);

	    if (strlen(pxstr) == 2) {
		strcat(logline4, " ");
	    }

	    strcat(logline4, "  ");

	    addcallarea = 0;

	    strcat(logline4, "    ");
	} else

	    strncat(logline4, fillspaces, 9);

    } else if ((universal == 1)
	       && ((country_mult == 1) || (dx_arrlsections == 1))) {

	logline4[68] = '\0';

	if (addcty != 0) {
	    strcat(logline4, dxcc_by_index(addcty) -> pfx);

	    strncat(logline4, fillspaces, 77 - strlen(logline4));
	    addcty = 0;

	} else
	    strncat(logline4, fillspaces, 9);

    } else if (wpx == 1) {
	strncat(logline4, fillspaces, 4);
    } else if (arrl_fd == 1) {
	strncat(logline4, fillspaces, 4);
    } else if ((one_point == 1) || (two_point == 1) || (three_point == 1)) {
	strncat(logline4, fillspaces, 4);
    } else {
	strncat(logline4, fillspaces, 4);
    }

    i = 77 - strlen(logline4);
    if (i > 0)				/* fill line until column 77 */
	strncat(logline4, fillspaces, i);


    score();			/* update qso's per band */

    if ((contest == 1) && (dxped == 0)) {
	    sprintf(logline4 + 76, "%2d", points);
    } else {
	sprintf(logline4 + 76, "  ");	/* no points for dxpedition */
    }


    assert(strlen(logline4) <= 80);
    strncat(logline4, fillspaces, 80 - strlen(logline4));

    /* add freq to end of logline */
    if (trx_control == 1) {
	snprintf(freq_buff, 8, "%7.1f", freq);
    } else {
	snprintf(freq_buff, 8, "        ");
    }
    strcat(logline4, freq_buff);
}
