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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <string.h>

#include <hamlib/rotator.h>

#include "dxcc.h"
#include "getctydata.h"
#include "globalvars.h"
#include "showmsg.h"


/* get countrynumber, QTH, CQ zone and continent for myself */
void getstationinfo() {
    dxcc_data *mydx;

    my.countrynr = getctydata(my.call);        /* whoami? */

    mydx = dxcc_by_index(my.countrynr);

    my.cqzone = mydx -> cq;
    strcpy(my.continent, mydx->continent);

    /* whereami? use QRA if possible */
    if (RIG_OK == locator2longlat(&my.Long, &my.Lat, my.qra)) {
	my.Long = -my.Long;     // W <-> E fix
    } else {
	my.Long = mydx->lon;
	my.Lat = mydx->lat;
    }
}


void show_station_info(void) {

    getstationinfo();

    showmsg("");
    char *info;
    info = g_strdup_printf("     Call = %s\n", my.call);
    showmsg(info);
    g_free(info);
    info = g_strdup_printf("     My Zone = %02d     My Continent = %s\n",
			   my.cqzone, my.continent);
    showmsg(info);
    g_free(info);
    showmsg("");

    refreshp();
}
