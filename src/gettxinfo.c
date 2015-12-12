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

#include <curses.h>

#include "fldigixmlrpc.h"
#include "gettxinfo.h"
#include "tlf.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LIBHAMLIB
# include <hamlib/rig.h>
#endif

int gettxinfo(void)
{

#ifdef HAVE_LIBHAMLIB
    extern RIG *my_rig;
    extern freq_t outfreq;
    extern int cw_bandwidth;
#else
    extern int outfreq;
#endif
    extern float freq;
    extern int bandinx;
    extern float bandfrequency[];

    extern int trx_control;
    extern int trxmode;
    extern int rigmode;
    extern int keyerport;

#ifdef HAVE_LIBHAMLIB
    freq_t rigfreq;
    vfo_t vfo;
#else
    float rigfreq;
#endif
    int retval = 0;
    int retvalmode = 0;
    static int oldbandinx;
    pbwidth_t bwidth;

    void send_bandswitch(int freq);

    if (trx_control != 1)
	return (0);

    if (outfreq == 0) {

	rigfreq = 0.0;

#ifdef HAVE_LIBHAMLIB		// Code for Hamlib interface
	retval = rig_get_vfo(my_rig, &vfo); /* initialize RIG_VFO_CURR */
	if (retval == RIG_OK || retval == -RIG_ENIMPL || retval == -RIG_ENAVAIL) {
	    retval = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);
	    if (trxmode == DIGIMODE && keyerport == GMFSK && retval == RIG_OK) {
		retvalmode = rig_get_mode(my_rig, RIG_VFO_CURR, (rmode_t *)&rigmode, &bwidth);
		if (retvalmode != RIG_OK) {
		    rigmode = RIG_MODE_NONE;
		}
	    }
	}

	if (trxmode == DIGIMODE && keyerport == GMFSK) {
	    rigfreq += fldigi_get_carrier();
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
		bandinx = 0;
		break;
	    }
	case 3500 ... 4000:{
		bandinx = 1;
		break;
	    }
	case 7000 ... 7300:{
		bandinx = 2;
		break;
	    }
	case 10100 ... 10150:{
		bandinx = 3;
		break;
	    }
	case 14000 ... 14350:{
		bandinx = 4;
		break;
	    }
	case 18068 ... 18168:{
		bandinx = 5;
		break;
	    }
	case 21000 ... 21450:{
		bandinx = 6;
		break;
	    }
	case 24890 ... 24990:{
		bandinx = 7;
		break;
	    }
	case 28000 ... 29700:{
		bandinx = 8;
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
				     RIG_PASSBAND_NORMAL);
		else
		    retval =
			rig_set_mode(my_rig, RIG_VFO_CURR,
				     RIG_MODE_USB,
				     RIG_PASSBAND_NORMAL);

		if (retval != RIG_OK) {
		    mvprintw(24, 0,
			     "Problem with rig link: set mode!\n");
		    refreshp();
		    sleep(1);
		}
	    } else if (trxmode == DIGIMODE) {
		retval =
		    rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,
				 RIG_PASSBAND_NORMAL);

		if (retval != RIG_OK) {
		    mvprintw(24, 0,
			     "Problem with rig link: set mode!\n");
		    refreshp();
		    sleep(1);
		}

	    } else {
//                                      retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  RIG_PASSBAND_NORMAL);
		if (cw_bandwidth == 0) {
			retval =
			    rig_set_mode(my_rig, RIG_VFO_CURR,
					 RIG_MODE_CW,
					 RIG_PASSBAND_NORMAL);
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
				 RIG_PASSBAND_NORMAL);
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
			     RIG_PASSBAND_NORMAL);
	else
	    retval =
		rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,
			     RIG_PASSBAND_NORMAL);

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
