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

#include "fldigixmlrpc.h"
#include "gettxinfo.h"
#include "tlf.h"
#include "tlf_curses.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LIBHAMLIB
# include <hamlib/rig.h>
#ifdef RIG_PASSBAND_NOCHANGE
#define TLF_DEFAULT_PASSBAND RIG_PASSBAND_NOCHANGE
#else
#define TLF_DEFAULT_PASSBAND RIG_PASSBAND_NORMAL
#endif
#endif

int gettxinfo(void)
{

#ifdef HAVE_LIBHAMLIB
    extern RIG *my_rig;
    extern freq_t outfreq;
    extern int cw_bandwidth;
    extern int trxmode;
    extern int rigmode;
    extern int digikeyer;
#else
    extern int outfreq;
#endif
    extern float freq;
    extern int bandinx;
    extern float bandfrequency[];

    extern int trx_control;
    extern unsigned char rigptt;

#ifdef HAVE_LIBHAMLIB
    freq_t rigfreq;
    vfo_t vfo;
    pbwidth_t bwidth;
    int retval;
    int retvalmode;
#else
    float rigfreq;
#endif

    static int oldbandinx;
    static int fldigi_carrier;
    static int fldigi_shift_freq;

    void send_bandswitch(int freq);

    if (trx_control != 1)
	return (0);

#ifdef HAVE_LIBHAMLIB
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
#endif

    if (outfreq == 0) {

	rigfreq = 0.0;

#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface
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
		    retval = rig_set_freq(my_rig, RIG_VFO_CURR, ((freq_t)rigfreq + (freq_t)fldigi_shift_freq));
		}
	    }
	}

	if (retval != RIG_OK || rigfreq < 0.1) {
	    freq = 0.0;
	    return (0);
	}
#endif


	if (rigfreq >= 1800000.0) {
	    freq = rigfreq / 1000.0;		/* kHz */
	}


	switch ((int)freq) {
	case 1800 ... 2000:{
		bandinx = BANDINDEX_160;
		break;
	    }
	case 3500 ... 4000:{
		bandinx = BANDINDEX_80;
		break;
	    }
	case 7000 ... 7300:{
		bandinx = BANDINDEX_40;
		break;
	    }
	case 10100 ... 10150:{
		bandinx = BANDINDEX_30;
		break;
	    }
	case 14000 ... 14350:{
		bandinx = BANDINDEX_20;
		break;
	    }
	case 18068 ... 18168:{
		bandinx = BANDINDEX_17;
		break;
	    }
	case 21000 ... 21450:{
		bandinx = BANDINDEX_15;
		break;
	    }
	case 24890 ... 24990:{
		bandinx = BANDINDEX_12;
		break;
	    }
	case 28000 ... 29700:{
		bandinx = BANDINDEX_10;
		break;
	    }
	default:
		bandinx = NBANDS;	/* out of band */
	}

	if (bandinx != NBANDS)
	    bandfrequency[bandinx] = freq;

	if (bandinx != oldbandinx)	// band change on trx
	{
	    oldbandinx = bandinx;
	    send_bandswitch((int) freq);


#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface

	    if (trxmode == SSBMODE) {
		if (freq < 14000.0)
		    retval =
			rig_set_mode(my_rig, RIG_VFO_CURR,
				     RIG_MODE_LSB,
				     TLF_DEFAULT_PASSBAND);
		else
		    retval =
			rig_set_mode(my_rig, RIG_VFO_CURR,
				     RIG_MODE_USB,
				     TLF_DEFAULT_PASSBAND);

		if (retval != RIG_OK) {
		    mvprintw(24, 0,
			     "Problem with rig link: set mode!\n");
		    refreshp();
		    sleep(1);
		}
	    } else if (trxmode == DIGIMODE) {
                if ((rigmode & (RIG_MODE_LSB | RIG_MODE_USB | RIG_MODE_RTTY | RIG_MODE_RTTYR)) != rigmode) {
		    retval =
		    	rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,
				 TLF_DEFAULT_PASSBAND);

		    if (retval != RIG_OK) {
			mvprintw(24, 0,
			     "Problem with rig link: set mode!\n");
			refreshp();
		    	sleep(1);
                    }
		}

	    } else {
//                                      retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  TLF_DEFAULT_PASSBAND);
		if (cw_bandwidth == 0) {
			retval =
			    rig_set_mode(my_rig, RIG_VFO_CURR,
					 RIG_MODE_CW,
					 TLF_DEFAULT_PASSBAND);
		} else {
			retval =
			    rig_set_mode(my_rig, RIG_VFO_CURR,
					 RIG_MODE_CW, cw_bandwidth);
		}

		if (retval != RIG_OK) {
		    mvprintw(24, 0,
			     "Problem with rig link: set mode!\n");
		    refreshp();
		    sleep(1);
		}

	    }
#endif

	}

    } else if (outfreq == SETCWMODE) {

#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface
	if (cw_bandwidth == 0) {
	    retval =
		rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,
				 TLF_DEFAULT_PASSBAND);
	} else {
	    retval =
		rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,
				 cw_bandwidth);
	}

	if (retval != 0) {
	    mvprintw(24, 0, "Problem with rig link!\n");
	    refreshp();
	    sleep(1);
	}
#endif

	outfreq = 0;

    } else if (outfreq == SETSSBMODE) {
#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface
	if (freq > 13999.9)
	    retval =
		rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_USB,
			     TLF_DEFAULT_PASSBAND);
	else
	    retval =
		rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,
			     TLF_DEFAULT_PASSBAND);

	if (retval != RIG_OK) {
	    mvprintw(24, 0, "Problem with rig link!\n");
	    refreshp();
	    sleep(1);
	}
#endif

	outfreq = 0;

    } else if (outfreq == RESETRIT) {
#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface
	    retval = rig_set_rit(my_rig, RIG_VFO_CURR, 0);

	    if (retval != RIG_OK) {
		mvprintw(24, 0, "Problem with rig link!\n");
		refreshp();
		sleep(1);
	    }
#endif

	outfreq = 0;

    } else {

#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface
	retval = rig_set_freq(my_rig, RIG_VFO_CURR, outfreq);

	if (retval != RIG_OK) {
	    mvprintw(24, 0, "Problem with rig link: set frequency!\n");
	    refreshp();
	    sleep(1);
	}

	if (retval != 0) {
	    mvprintw(24, 0, "Problem with rig link: set frequency!\n");
	    refreshp();
	    sleep(1);
	}
#endif

	outfreq = 0;

    }

    return (0);
}
