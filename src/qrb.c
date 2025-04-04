/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *               2014           Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "err_utils.h"
#include "getctydata.h"
#include "globalvars.h"
#include "startmsg.h"
#include "qrb.h"

static int parse_rotconf();


/* Compute the Bearing and Range */

int get_qrb(double *range, double *bearing) {

    extern double DEST_Lat;
    extern double DEST_Long;

    if (*current_qso.call == '\0')
	return -1;

    /* positive numbers are N and E
     *
     * be aware that dxcc counts east longitudes as negative numbers
     */
    return qrb(-1.0 * my.Long, my.Lat, -1.0 * DEST_Long, DEST_Lat,
	       range, bearing);
}

bool get_qrb_for_locator(const char *locator, double *range, double *bearing) {
    double dest_long, dest_lat;

    if (RIG_OK != locator2longlat(&dest_long, &dest_lat, locator)) {
	return false;   // invalid locator
    }

    return RIG_OK == qrb(my.Long, my.Lat, -dest_long, dest_lat,
			 range, bearing);
}


/* Hamlib rotator control */

int init_tlf_rot(void) {
    int retcode;		/* generic return code from functions */

    /*
     * allocate memory, setup & open port
     */
    my_rot = rot_init((rot_model_t) myrot_model);

    if (!my_rot) {
	shownr("Unknown rotator model", myrot_model);
	return -1;
    }

    if (rotportname == NULL || strlen(rotportname) == 0) {
	showmsg("Missing rot port name!");
	return -1;
    }

    rotportname[strlen(rotportname) - 1] = '\0';	// remove '\n'
    strncpy(my_rot->state.rotport.pathname, rotportname,
	    TLFFILPATHLEN - 1);

    my_rot->state.rotport.parm.serial.rate = serial_rate;

    // parse ROTCONF parameters
    if (parse_rotconf() < 0) {
	return -1;
    }

    retcode = rot_open(my_rot);

    if (retcode != RIG_OK) {
	TLF_LOG_WARN("rot_open: %s", rigerror(retcode));
	return -1;
    }

    return 0;
}

void rotate_to_qrb() {
    double bearing;
    double range;
    int retcode;		/* generic return code from functions */
    int pfx_index = getctydata_pfx(current_qso.call);

    prefix_data *pfx = prefix_by_index(pfx_index);

    if (pfx->dxcc_ctynr > 0) {

	if (pfx_index != my.countrynr && 0 == get_qrb(&range, &bearing)) {

	    pthread_mutex_lock(&tlf_rot_mutex);
	    retcode = rot_set_position(my_rot, bearing, 0.0); // azimuth, elevation
	    pthread_mutex_unlock(&tlf_rot_mutex);

	    if (retcode != RIG_OK) {
		TLF_LOG_WARN("Problem with setting rotator position: %s", rigerror(retcode));
	    } else {
		showmsg("Rotator set position ok!");
	    }
	}
    }
}

void close_tlf_rot(ROT *my_rot) {

    pthread_mutex_lock(&tlf_rot_mutex);
    rot_close(my_rot);		/* close port */
    rot_cleanup(my_rot);	/* if you care about memory */
    pthread_mutex_unlock(&tlf_rot_mutex);

    printf("Rotator port %s closed\n", rotportname);
}

static int parse_rotconf() {
    char *cnfparm, *cnfval;
    const int rotconf_len = strlen(rotconf);
    int i;
    int retcode;

    cnfparm = cnfval = rotconf;

    for (i = 0; i < rotconf_len; i++) {
	/* FIXME: left hand value of = cannot be null */
	if (rotconf[i] == '=' && cnfval == cnfparm) {
	    cnfval = rotconf + i + 1;
	    rotconf[i] = '\0';
	    continue;
	}
	if (rotconf[i] == ',' || i + 1 == rotconf_len) {
	    if (cnfval <= cnfparm) {
		showstring("Missing parm value in ROTCONF: ", rotconf);
		return -1;
	    }
	    if (rotconf[i] == ',')
		rotconf[i] = '\0';

	    pthread_mutex_lock(&tlf_rot_mutex);
	    retcode =
		rot_set_conf(my_rot, rot_token_lookup(my_rot, cnfparm),
			     cnfval);
	    pthread_mutex_unlock(&tlf_rot_mutex);

	    if (retcode != RIG_OK) {
		showmsg("rot_set_conf: error  ");
		return -1;
	    }

	    cnfparm = cnfval = rotconf + i + 1;
	    continue;
	}
    }

    return 0;
}
