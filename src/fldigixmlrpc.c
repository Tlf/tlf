/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011, 2014     Thomas Beierlein <tb@forth-ev.de>
 *               2014           Ervin Hegedus <airween@gmail.com>
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


#include "startmsg.h"
#include <stdlib.h>	// need for abs()

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LIBXMLRPC
# include <xmlrpc-c/base.h>
# include <xmlrpc-c/client.h>
#endif

#ifdef HAVE_LIBHAMLIB
# include <hamlib/rig.h>
#endif

#define NAME "Tlf"
#define XMLRPCVERSION "1.0"

#define CENTER_FREQ 2210	// low: 2125Hz, high: 2295Hz, shift: 170Hz, 2125+(170/2) = 2210Hz
#define MAXSHIFT 20		// max shift value in Fldigi, when Tlf set it back to RIG carrier

#ifdef HAVE_LIBHAMLIB
extern RIG *my_rig;
#endif

int fldigi_var_carrier = 0;

int fldigi_xmlrpc_get_carrier() {

#ifndef HAVE_LIBXMLRPC
    return 0;
#else

    extern int rigmode;
    extern int trx_control;
    xmlrpc_env env;
    xmlrpc_value * result;
    xmlrpc_int32 sum;
    xmlrpc_env_init(&env);
    freq_t rigfreq;
    int retval;

    static int errflg;
    static int trycnt;
    int signum = -1;
    int modeshift = 0;

    /* if some call timed outs/breaks, we will suspend the call */
    if (errflg > 0 && trycnt < 1000) {
        trycnt++;
	return -1;
    }

    errflg = 0;
    trycnt = 0;
    const char * const serverUrl = "http://localhost:7362/RPC2";
    const char * const methodName = "modem.get_carrier";
    const char * const setmethodName = "modem.set_carrier";

    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, XMLRPCVERSION, NULL, 0);
    if (env.fault_occurred) {
	fldigi_var_carrier = 0;
	errflg = 1;
        return -1;
    }

    result = xmlrpc_client_call(&env, serverUrl, methodName,
                                 "(ii)", (xmlrpc_int32) 5, (xmlrpc_int32) 7);
    if (env.fault_occurred) {
	fldigi_var_carrier = 0;
	errflg = 1;
        return -1;
    }

    xmlrpc_read_int(&env, result, &sum);
    if (env.fault_occurred) {
	fldigi_var_carrier = 0;
	errflg = 1;
	return -1;
    }
    fldigi_var_carrier = (int)sum;

#ifdef HAVE_LIBHAMLIB
    if (trx_control > 0) {

	if (rigmode == RIG_MODE_RTTY || rigmode == RIG_MODE_RTTYR) {
	    if (fldigi_var_carrier != CENTER_FREQ && abs(CENTER_FREQ - fldigi_var_carrier) > MAXSHIFT) {
		result = xmlrpc_client_call(&env, serverUrl, setmethodName,
					"(ii)", (xmlrpc_int32) CENTER_FREQ, (xmlrpc_int32) 7);
		retval = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);
		rigfreq += (freq_t)(CENTER_FREQ-fldigi_var_carrier);
		retval = rig_set_freq(my_rig, RIG_VFO_CURR, rigfreq);
	    }
	}

	if (rigmode != RIG_MODE_NONE) {
	    switch (rigmode) {
		case RIG_MODE_USB:	signum = 1;
					break;
					modeshift = 0;
		case RIG_MODE_LSB:	signum = -1;
					break;
					modeshift = 0;
		case RIG_MODE_RTTY:	signum = 0;
					modeshift = -100; // on my TS850, in FSK mode, the QRG is differ by 100Hz up
							  // possible need to check in other rigs
					break;
		case RIG_MODE_RTTYR:	signum = 0;	// not checked - I don't have RTTY-REV mode on my RIG
					modeshift = -100;
					break;
		default:		signum = 0;	// this is the "normal"
					modeshift = 0;
	    }
	    fldigi_var_carrier = ((signum)*fldigi_var_carrier)+modeshift;
	}

    }
#endif

    xmlrpc_DECREF(result);
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return 0;
#endif

}

int fldigi_get_carrier() {
#ifdef HAVE_LIBXMLRPC
        return fldigi_var_carrier;
#else
        return 0;
#endif
}

void xmlrpc_showinfo() {
#ifdef HAVE_LIBXMLRPC		// Show xmlrpc status
    showmsg("XMLRPC compiled in");
#else
    showmsg("XMLRPC NOT compiled");
#endif
}
