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

#include "bands.h"
#include "cw_utils.h"
#include "err_utils.h"
#include "hamlib_keyer.h"
#include "sendqrg.h"
#include "startmsg.h"
#include "gettxinfo.h"
#include "bands.h"
#include "globalvars.h"


void send_bandswitch(freq_t trxqrg);

static int parse_rigconf();
static void debug_tlf_rig();

/* check if call input field contains a frequency value and switch to it.
 *
 */
int sendqrg(void) {

    if (!trx_control) {
	return 0;               /* nothing to do here */
    }

    const freq_t trxqrg = atof(hiscall) * 1000.0;

    int bandinx = freq2band(trxqrg);

    if (bandinx == BANDINDEX_OOB) {
	return 0;   // not a frequency or out of band
    }

    set_outfreq(trxqrg);
    send_bandswitch(trxqrg);

    return trxqrg;
}

/**************************************************************************/

int init_tlf_rig(void) {
    freq_t rigfreq;		/* frequency  */
    vfo_t vfo;
    int retcode;		/* generic return code from functions */

    const char *ptt_file = NULL, *dcd_file = NULL;
    dcd_type_t dcd_type = RIG_DCD_NONE;

    const struct rig_caps *caps;
    int rig_cwspeed;

    rig_set_debug(RIG_DEBUG_NONE);

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

    rigportname[strlen(rigportname) - 1] = '\0';	// remove '\n'
    strncpy(my_rig->state.rigport.pathname, rigportname,
	    TLFFILPATHLEN - 1);

    caps = my_rig->caps;

    /* If CAT PTT is wanted, test for CAT capability of rig backend. */
    if (rigptt & CAT_PTT_WANTED) {
	if (caps->ptt_type == RIG_PTT_RIG) {
	    rigptt |= CAT_PTT_AVAILABLE;
	} else {
	    rigptt = 0;
	    showmsg("Controlling PTT via Hamlib is not supported for that rig!");
	}
    }

    if (dcd_type != RIG_DCD_NONE)
	my_rig->state.dcdport.type.dcd = dcd_type;
    if (ptt_file)
	strncpy(my_rig->state.pttport.pathname, ptt_file, TLFFILPATHLEN);
    if (dcd_file)
	strncpy(my_rig->state.dcdport.pathname, dcd_file, TLFFILPATHLEN);

    my_rig->state.rigport.parm.serial.rate = serial_rate;

    // parse RIGCONF parameters
    if (parse_rigconf() < 0) {
	return -1;
    }

    retcode = rig_open(my_rig);

    if (retcode != RIG_OK) {
	TLF_LOG_WARN("rig_open: %s", rigerror(retcode));
	return -1;
    }

    rigfreq = 0.0;

    retcode = rig_get_vfo(my_rig, &vfo); 	/* initialize RIG_VFO_CURR */
    if (retcode == RIG_OK || retcode == -RIG_ENIMPL || retcode == -RIG_ENAVAIL)
	retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

    if (retcode != RIG_OK) {
	TLF_LOG_WARN("Problem with rig link: %s", rigerror(retcode));
	if (!debugflag)
	    return -1;
    }

    shownr("Freq =", (int) rigfreq);

    if (cwkeyer == HAMLIB_KEYER) {
	retcode = hamlib_keyer_get_speed(&rig_cwspeed); /* read cw speed from rig */

	if (retcode == RIG_OK) {
	    shownr("CW speed = ", rig_cwspeed);
	    SetCWSpeed(rig_cwspeed);
	} else {
	    TLF_LOG_WARN("Could not read CW speed from rig: %s", rigerror(retcode));
	    if (!debugflag)
		return -1;
	}
    }

    if (debugflag) {	// debug rig control
	debug_tlf_rig();
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

    return 0;
}

void close_tlf_rig(RIG *my_rig) {

    rig_close(my_rig);		/* close port */
    rig_cleanup(my_rig);	/* if you care about memory */

    printf("Rig port %s closed\n", rigportname);
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
	    retcode =
		rig_set_conf(my_rig, rig_token_lookup(my_rig, cnfparm),
			     cnfval);
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


static void debug_tlf_rig() {
    freq_t rigfreq;
    int retcode;

    sleep(10);

    retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

    if (retcode != RIG_OK) {
	TLF_LOG_WARN("Problem with rig get freq: %s", rigerror(retcode));
    } else {
	shownr("freq =", (int) rigfreq);
    }
    sleep(10);

    const freq_t testfreq = 14000000;	// test set frequency

    retcode = rig_set_freq(my_rig, RIG_VFO_CURR, testfreq);

    if (retcode != RIG_OK) {
	TLF_LOG_WARN("Problem with rig set freq: %s", rigerror(retcode));
    } else {
	showmsg("Rig set freq ok!");
    }

    retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);	// read qrg

    if (retcode != RIG_OK) {
	TLF_LOG_WARN("Problem with rig get freq: %s", rigerror(retcode));
    } else {
	shownr("freq =", (int) rigfreq);
	if (rigfreq != testfreq) {
	    showmsg("Failed to set rig freq!");
	}
    }
    sleep(10);

}
