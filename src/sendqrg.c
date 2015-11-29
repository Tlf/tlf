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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sendqrg.h"		// Sets HAVE_LIBHAMLIB if enabled
#include "startmsg.h"


void send_bandswitch(int trxqrg);

int sendqrg(void)
{

    extern char hiscall[];
    extern int trx_control;
#ifdef HAVE_LIBHAMLIB
//extern RIG *my_rig;
    extern freq_t outfreq;
#else
//extern int my_rig;
    extern int outfreq;
#endif

    float trxqrg;

    if (trx_control != 1)
	return (0);		/* nothing to do here */

    trxqrg = atof(hiscall);

    switch ((int) trxqrg) {

    case 0 ... 1799:{
	    trxqrg = 0;
	    break;
	}
    case 1800 ...  2000:
    case 3500 ...  4000:
    case 7000 ...  7300:
    case 10100 ... 10150:
    case 14000 ... 14350:
    case 18068 ... 18168:
    case 21000 ... 21450:
    case 24890 ... 24990:
    case 28000 ... 29700:
	{
#ifdef HAVE_LIBHAMLIB
	    outfreq = (freq_t) (trxqrg * 1000);
#else
	    outfreq = (int) (trxqrg * 1000);
#endif

	    send_bandswitch((int) trxqrg);

	    break;
	}
    default:
	trxqrg = 0;

    }

    return ((int)trxqrg);
}

/**************************************************************************/
#ifdef HAVE_LIBHAMLIB		//code for Hamlib interface

int init_tlf_rig(void)
{
    extern RIG *my_rig;
    extern rig_model_t myrig_model;
    extern freq_t outfreq;
    extern char rigconf[];
    extern int serial_rate;
    extern char rigportname[];
    extern int debugflag;

    freq_t rigfreq;		/* frequency  */
    vfo_t vfo;
    int retcode;		/* generic return code from functions */

    const char *ptt_file = NULL, *dcd_file = NULL;
    ptt_type_t ptt_type = RIG_PTT_NONE;
    dcd_type_t dcd_type = RIG_DCD_NONE;

    char *cnfparm, *cnfval;
    int rigconf_len, i;

    /*
     * allocate memory, setup & open port
     */
    rig_set_debug(RIG_DEBUG_NONE);

    my_rig = rig_init(myrig_model);

    if (!my_rig) {
	shownr("Unknown rig num %d", (int) myrig_model);
	return (-1);
    } else {
	if (strlen(rigportname) > 1) {
	    rigportname[strlen(rigportname) - 1] = '\0';	// remove '\n'
	    strncpy(my_rig->state.rigport.pathname, rigportname,
		    FILPATHLEN);
	} else
	{
	    showmsg("Missing rig port name!");
	    return (-1);
	}


    }

    if (ptt_type != RIG_PTT_NONE)
	my_rig->state.pttport.type.ptt = ptt_type;
    if (dcd_type != RIG_DCD_NONE)
	my_rig->state.dcdport.type.dcd = dcd_type;
    if (ptt_file)
	strncpy(my_rig->state.pttport.pathname, ptt_file, FILPATHLEN);
    if (dcd_file)
	strncpy(my_rig->state.dcdport.pathname, dcd_file, FILPATHLEN);

    my_rig->state.rigport.parm.serial.rate = serial_rate;

    cnfparm = cnfval = rigconf;
    rigconf_len = strlen(rigconf);
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
		return (-1);
	    }
	    if (rigconf[i] == ',')
		rigconf[i] = '\0';
	    retcode =
		rig_set_conf(my_rig, rig_token_lookup(my_rig, cnfparm),
			     cnfval);
	    if (retcode != RIG_OK) {
		showmsg("rig_set_conf: error  ");
		return (-1);
	    }

	    cnfparm = cnfval = rigconf + i + 1;
	    continue;
	}
    }

    retcode = rig_open(my_rig);

    if (retcode != RIG_OK) {
	showmsg("rig_open: error ");
	return (-1);
    }

    rigfreq = 0.0;

    retcode = rig_get_vfo(my_rig, &vfo); 	/* initialize RIG_VFO_CURR */
    if (retcode == RIG_OK || retcode == -RIG_ENIMPL || retcode == -RIG_ENAVAIL)
	retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

    if (retcode != RIG_OK) {
	showmsg("Problem with rig link!");
	if (!debugflag)
	    return (-1);
    }

    shownr("freq =", (int) rigfreq);

    if (debugflag == 1) {	// debug routines
	sleep(10);

	retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

	if (retcode != RIG_OK) {
	    showmsg("Problem with rig get freq!");
	    sleep(1);
	} else {
	    shownr("freq =", (int) rigfreq);
	}
	sleep(10);

	outfreq = 14000000;	//test set frequency

	retcode = rig_set_freq(my_rig, RIG_VFO_CURR, outfreq);

	if (retcode != RIG_OK) {
	    showmsg("Problem with rig set freq!");
	    sleep(1);
	} else {
	    showmsg("Rig set freq ok!");
	}

	outfreq = 0;

	retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);	// read qrg

	if (retcode != RIG_OK) {
	    showmsg("Problem with rig get freq!");
	    sleep(1);
	} else {
	    shownr("freq =", (int) rigfreq);
	}
	sleep(10);

    }				// end debug

    sleep(1);
    return (0);
}

int close_tlf_rig(RIG * my_rig)
{
    extern char rigportname[];

    rig_close(my_rig);		/* close port */
    rig_cleanup(my_rig);	/* if you care about memory */

    printf("port %s closed ok \n", rigportname);

    return (0);
}

#endif				// end code for Hamlib interface

