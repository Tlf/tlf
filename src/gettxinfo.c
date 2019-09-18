/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
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
 *      get trx info
 *
 *--------------------------------------------------------------*/


#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include "bands.h"
#include "err_utils.h"
#include "fldigixmlrpc.h"
#include "gettxinfo.h"
#include "tlf.h"
#include "tlf_curses.h"
#include "callinput.h"
#include "bands.h"

#include <hamlib/rig.h>

#ifdef RIG_PASSBAND_NOCHANGE
#define TLF_DEFAULT_PASSBAND RIG_PASSBAND_NOCHANGE
#else
#define TLF_DEFAULT_PASSBAND RIG_PASSBAND_NORMAL
#endif

extern RIG *my_rig;
extern int cw_bandwidth;
extern int trxmode;
extern int rigmode;
extern int digikeyer;
extern rmode_t digi_mode;

extern freq_t freq;
extern int bandinx;
extern freq_t bandfrequency[];

extern int trx_control;
extern unsigned char rigptt;

/* output frequency to rig or other rig-related request
 *
 * possible values:
 *  0 - poll rig
 *  SETCWMODE
 *  SETSSBMODE
 *  RESETRIT
 *  SETDIGIMODE
 *  else - set rig frequency
 *
 */
static freq_t outfreq = 0;

static pthread_mutex_t outfreq_mutex = PTHREAD_MUTEX_INITIALIZER;

static double get_current_seconds();
static void handle_trx_bandswitch(const freq_t freq);

void set_outfreq(freq_t hertz) {
    if (!trx_control) {
	hertz = 0;      // no rig control, ignore request
    }
    pthread_mutex_lock(&outfreq_mutex);
    outfreq = hertz;
    pthread_mutex_unlock(&outfreq_mutex);
}

freq_t get_outfreq() {
    return outfreq;
}

static freq_t get_and_reset_outfreq() {
    pthread_mutex_lock(&outfreq_mutex);
    freq_t f = outfreq;
    outfreq = 0.0;
    pthread_mutex_unlock(&outfreq_mutex);
    return f;
}

static rmode_t get_ssb_mode() {
    // LSB below 14 MHz, USB above it
    return (freq < bandcorner[BANDINDEX_20][0] ? RIG_MODE_LSB : RIG_MODE_USB);
}

static pbwidth_t get_cw_bandwidth() {
    // use specified bandwidth (CWBANDWIDTH) if available
    return (cw_bandwidth > 0 ? cw_bandwidth : TLF_DEFAULT_PASSBAND);
}


void gettxinfo(void) {

    freq_t rigfreq;
    vfo_t vfo;
    pbwidth_t bwidth;
    int retval;
    int retvalmode;
    static double last_freq_time = 0.0;

    static int oldbandinx;
    static int fldigi_carrier;
    static int fldigi_shift_freq;

    if (!trx_control)
	return;

    /* CAT PTT wanted, available, inactive, and PTT On requested
     * bits 0, 1, and 3 set.
     */
    if (rigptt == 0x0b) {
	retval = rig_set_ptt(my_rig, RIG_VFO_CURR, RIG_PTT_ON);

	/* Set PTT active bit. */
	rigptt |= (1 << 2);		/* 0x0f */

	/* Clear PTT On requested bit. */
	rigptt &= ~(1 << 3);		/* 0x07 */
    }

    /* CAT PTT wanted, available, active and PTT Off requested
     * bits 0, 1, 2, and 4 set.
     */
    if (rigptt == 0x17) {
	retval = rig_set_ptt(my_rig, RIG_VFO_CURR, RIG_PTT_OFF);

	/* Clear PTT Off requested bit. */
	rigptt &= ~(1 << 4);		/* 0x07 */

	/* Clear PTT active bit. */
	rigptt &= ~(1 << 2);		/* 0x03 */
    }

    freq_t reqf = get_and_reset_outfreq();  // get actual request

    if (reqf == 0) {

	rigfreq = 0.0;

	double now = get_current_seconds();
	if (now < last_freq_time + 0.2) {
	    return;   // last read-out was within 200 ms, skip this query
	}
	last_freq_time = now;

	retval = rig_get_vfo(my_rig, &vfo); /* initialize RIG_VFO_CURR */
	if (retval == RIG_OK || retval == -RIG_ENIMPL || retval == -RIG_ENAVAIL) {
	    retval = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);
	    if (trxmode == DIGIMODE && (digikeyer == GMFSK || digikeyer == FLDIGI)
		    && retval == RIG_OK) {
		retvalmode = rig_get_mode(my_rig, RIG_VFO_CURR, (rmode_t *)&rigmode, &bwidth);
		if (retvalmode != RIG_OK) {
		    rigmode = RIG_MODE_NONE;
		}
	    }
	}

	if (trxmode == DIGIMODE && (digikeyer == GMFSK || digikeyer == FLDIGI)) {
	    fldigi_carrier = fldigi_get_carrier();
	    rigfreq += (freq_t)fldigi_carrier;
	    if (rigmode == RIG_MODE_RTTY || rigmode == RIG_MODE_RTTYR) {
		fldigi_shift_freq = fldigi_get_shift_freq();
		if (fldigi_shift_freq != 0) {
		    retval = rig_set_freq(my_rig, RIG_VFO_CURR,
					  ((freq_t)rigfreq + (freq_t)fldigi_shift_freq));
		}
	    }
	}

	if (retval != RIG_OK || rigfreq < 0.1) {
	    freq = 0.0;
	    return;
	}


	if (rigfreq >= bandcorner[0][0]) {
	    freq = rigfreq; // Hz
	}

	bandinx = freq2band((unsigned int)freq);

	bandfrequency[bandinx] = freq;

	if (bandinx != oldbandinx) {	// band change on trx
	    oldbandinx = bandinx;
	    handle_trx_bandswitch((int) freq);
	}

    } else if (reqf == SETCWMODE) {

	retval = rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW, get_cw_bandwidth());

	if (retval != RIG_OK) {
	    TLF_LOG_WARN("Problem with rig link!");
	}

    } else if (reqf == SETSSBMODE) {

	retval = rig_set_mode(my_rig, RIG_VFO_CURR, get_ssb_mode(),
			      TLF_DEFAULT_PASSBAND);

	if (retval != RIG_OK) {
	    TLF_LOG_WARN("Problem with rig link!");
	}

    } else if (reqf == SETDIGIMODE) {
	rmode_t new_mode = digi_mode;
	if (new_mode == RIG_MODE_NONE) {
	    if (digikeyer == FLDIGI)
		new_mode = RIG_MODE_USB;
	    else
		new_mode = RIG_MODE_LSB;
	}
	retval = rig_set_mode(my_rig, RIG_VFO_CURR, new_mode,
			      TLF_DEFAULT_PASSBAND);

	if (retval != RIG_OK) {
	    TLF_LOG_WARN("Problem with rig link!");
	}

    } else if (reqf == RESETRIT) {
	retval = rig_set_rit(my_rig, RIG_VFO_CURR, 0);

	if (retval != RIG_OK) {
	    TLF_LOG_WARN("Problem with rig link!");
	}

    } else {
	// set rig frequency to `reqf'
	retval = rig_set_freq(my_rig, RIG_VFO_CURR, (freq_t) reqf);

	if (retval != RIG_OK) {
	    TLF_LOG_WARN("Problem with rig link: set frequency!");
	}

    }

}


static double get_current_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}


static void handle_trx_bandswitch(const freq_t freq) {

    send_bandswitch(freq);

    rmode_t mode = RIG_MODE_NONE;           // default: no change
    pbwidth_t width = TLF_DEFAULT_PASSBAND; // passband width, in Hz

    if (trxmode == SSBMODE) {
	mode = get_ssb_mode();
    } else if (trxmode == DIGIMODE) {
	if ((rigmode & (RIG_MODE_LSB | RIG_MODE_USB | RIG_MODE_RTTY | RIG_MODE_RTTYR))
		!= rigmode) {
	    mode = RIG_MODE_LSB;
	}
    } else {
	mode = RIG_MODE_CW;
	width = get_cw_bandwidth();
    }

    if (mode == RIG_MODE_NONE) {
	return;     // no change was requested
    }

    int retval = rig_set_mode(my_rig, RIG_VFO_CURR, mode, width);

    if (retval != RIG_OK) {
	TLF_LOG_WARN("Problem with rig link!");
    }

}

