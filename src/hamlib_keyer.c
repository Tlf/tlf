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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <assert.h>
#include <hamlib/rig.h>

#include "globalvars.h"
#include "sendqrg.h"
#include "hamlib_keyer.h"

int hamlib_keyer_set_speed(int *cwspeed) {
    gran_t keyspd_gran = my_rig->caps->level_gran[rig_setting2idx(
			     RIG_LEVEL_KEYSPD)];
    value_t spd;

    /* if rig declared min/max speed, honor it. */
    if (keyspd_gran.min.i > 0 && *cwspeed < keyspd_gran.min.i)
	*cwspeed = keyspd_gran.min.i;
    if (keyspd_gran.max.i > 0 && *cwspeed > keyspd_gran.max.i)
	*cwspeed = keyspd_gran.max.i;
    spd.i = *cwspeed;

    pthread_mutex_lock(&tlf_rig_mutex);
    int ret = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, spd);
    pthread_mutex_unlock(&tlf_rig_mutex);

    return ret;
}

int hamlib_keyer_get_speed(int *cwspeed) {
    value_t value;

    assert(cwspeed != NULL);

    pthread_mutex_lock(&tlf_rig_mutex);
    int ret = rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, &value);
    pthread_mutex_unlock(&tlf_rig_mutex);

    if (ret == RIG_OK)
	*cwspeed = value.i;
    return ret;
}

int hamlib_keyer_send(char *cwmessage) {
    pthread_mutex_lock(&tlf_rig_mutex);
    int ret = rig_send_morse(my_rig, RIG_VFO_CURR, cwmessage);
    pthread_mutex_unlock(&tlf_rig_mutex);

    return ret;
}

int hamlib_keyer_stop() {
#if HAMLIB_VERSION >= 400
    if (rig_has_stop_morse()) {
	pthread_mutex_lock(&tlf_rig_mutex);
	int ret = rig_stop_morse(my_rig, RIG_VFO_CURR);
	pthread_mutex_unlock(&tlf_rig_mutex);
	return ret;
    }
#endif

    return RIG_OK;
}
