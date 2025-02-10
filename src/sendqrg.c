/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
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


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "bands.h"
#include "callinput.h"
#include "cw_utils.h"
#include "err_utils.h"
#include "hamlib_keyer.h"
#include "sendqrg.h"
#include "showmsg.h"
#include "gettxinfo.h"
#include "bands.h"
#include "globalvars.h"

static bool init_called = false;
static bool can_send_morse = false;
static bool can_stop_morse = false;

bool rig_has_send_morse() {
    assert(init_called);
    return can_send_morse;
}

bool rig_has_stop_morse() {
    assert(init_called);
    return can_stop_morse;
}

void send_bandswitch(freq_t trxqrg);

static int parse_rigconf();

/* check if call input field contains a valid frequency and switch to it.
 * only integer kHz values are supported.
 * if the frequency is out-of-band or there is no rig control then no action
 * is taken.
 *
 * return:
 *  true    - call input field was a number
 *  false   - call input field was not a number
 */
bool sendqrg(void) {

    if (!plain_number(current_qso.call)) {
	return false;
    }

    if (trx_control) {

	const freq_t trxqrg = atoi(current_qso.call) * 1000.0;

	int bandinx = freq2bandindex(trxqrg);

	if (bandinx != BANDINDEX_OOB) {
	    set_outfreq(trxqrg);
	    send_bandswitch(trxqrg);
	}
    }

    return true;
}

/**************************************************************************/

void show_rigerror(char *message, int errcode) {
    char *str = g_strdup_printf("%s: %s", message, tlf_rigerror(errcode));
    showmsg(str);
    g_free(str);
}


int init_tlf_rig(void) {
    freq_t rigfreq;		/* frequency  */
    vfo_t vfo;
    int retcode;		/* generic return code from functions */

    const struct rig_caps *caps;
    int rig_cwspeed;

    /*
     * allocate memory, setup & open port
     */
    my_rig = rig_init((rig_model_t) myrig_model);

    if (!my_rig) {
	shownr("Unknown rig model", myrig_model);
	return -1;
    }

    if (rigportname == NULL || strlen(rigportname) == 0) {
	showmsg("Missing rig port name!");
	return -1;
    }

    g_strchomp(rigportname);	// remove trailing '\n'
    strncpy(my_rig->state.rigport.pathname, rigportname,
	    TLFFILPATHLEN - 1);

    my_rig->state.rigport.parm.serial.rate = serial_rate;

    caps = my_rig->caps;

    can_send_morse = caps->send_morse != NULL;
#if HAMLIB_VERSION >= 400
    can_stop_morse = caps->stop_morse != NULL;
#else
    can_stop_morse = false; // rig_stop_morse was introduced in Hamlib 4.0
#endif

    /* If CAT PTT is wanted, test for CAT capability of rig backend. */
    if (rigptt & CAT_PTT_WANTED) {
	if (caps->ptt_type == RIG_PTT_RIG) {
	    rigptt |= CAT_PTT_AVAILABLE;
	} else {
	    rigptt = 0;
	    showmsg("Controlling PTT via Hamlib is not supported for that rig!");
	}
    }

    if (parse_rigconf() < 0) {
	return -1;
    }

    retcode = rig_open(my_rig);

    if (retcode != RIG_OK) {
	show_rigerror("rig_open", retcode);
	return -1;
    }

    rigfreq = 0.0;

    retcode = rig_get_vfo(my_rig, &vfo); 	/* initialize RIG_VFO_CURR */
    if (retcode == RIG_OK || retcode == -RIG_ENIMPL || retcode == -RIG_ENAVAIL)
	retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

    if (retcode != RIG_OK) {
	show_rigerror("Problem with rig link", retcode);
	return -1;
    }

    shownr("Freq =", (int) rigfreq);

    if (cwkeyer == HAMLIB_KEYER) {

	retcode = hamlib_keyer_get_speed(&rig_cwspeed); /* read cw speed from rig */

	if (retcode == RIG_OK) {
	    shownr("CW speed = ", rig_cwspeed);
	    speed = rig_cwspeed;
	} else {
	    show_rigerror("Could not read CW speed from rig", retcode);
	    return -1;
	}
    }

    switch (trxmode) {
	case SSBMODE:
	    set_outfreq(SETSSBMODE);
	    break;
	case DIGIMODE:
	    set_outfreq(SETDIGIMODE);
	    break;
	case CWMODE:
	    set_outfreq(SETCWMODE);
	    break;
    }

    init_called = true;

    return 0;
}

void close_tlf_rig(RIG *my_rig) {
    pthread_mutex_lock(&tlf_rig_mutex);
    rig_close(my_rig);		/* close port */
    rig_cleanup(my_rig);	/* if you care about memory */
    pthread_mutex_unlock(&tlf_rig_mutex);
}

static int parse_rigconf() {
    char *cnfparm, *cnfval;
    const int rigconf_len = strlen(rigconf);
    int i;
    int retcode;

    cnfparm = cnfval = rigconf;

    for (i = 0; i < rigconf_len; i++) {
	/* FIXME: left hand value of = cannot be null */
	if (rigconf[i] == '=' && cnfval == cnfparm) {
	    cnfval = rigconf + i + 1;
	    rigconf[i] = '\0';
	    continue;
	}
	if (rigconf[i] == ',' || i + 1 == rigconf_len) {
	    if (cnfval <= cnfparm) {
		showstring("Missing parm value in RIGCONF: ", rigconf);
		return -1;
	    }
	    if (rigconf[i] == ',')
		rigconf[i] = '\0';

	    pthread_mutex_lock(&tlf_rig_mutex);
	    retcode =
		rig_set_conf(my_rig, rig_token_lookup(my_rig, cnfparm),
			     cnfval);
	    pthread_mutex_unlock(&tlf_rig_mutex);

	    if (retcode != RIG_OK) {
		showmsg("rig_set_conf: error  ");
		return -1;
	    }

	    cnfparm = cnfval = rigconf + i + 1;
	    continue;
	}
    }

    return 0;
}

