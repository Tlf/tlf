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
#include "hamlib_keyer.h"

bool rig_has_send_morse() {
   return (my_rig->caps->send_morse != NULL);
}

bool rig_has_stop_morse() {
    return (my_rig->caps->stop_morse != NULL);
}

int hamlib_keyer_set_speed(int cwspeed) {
    value_t spd;
    spd.i = cwspeed;

    return rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, spd);
}

int hamlib_keyer_get_speed( int *cwspeed) {
    value_t value;

    assert (cwspeed != NULL);
    int ret = rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, &value);
    if (ret == RIG_OK)
	*cwspeed = value.i;
    return ret;
}

int hamlib_keyer_send(char *cwmessage) {
    return rig_send_morse(my_rig, RIG_VFO_CURR, cwmessage);
}

int hamlib_keyer_stop() {
    if (rig_has_stop_morse()) {
	return rig_stop_morse(my_rig, RIG_VFO_CURR);
    } else {
	return RIG_OK;
    }
}
