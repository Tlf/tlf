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

/* ------------------------------------------------------------
 *        Get messages  from  -paras file
 *        and  gets  the last  5 qso records for  display
 *        also gets the nr of the last qso from  the logfile
 *--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "checklogfile.h"
#include "dxcc.h"
#include "getctydata.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "qsonr_to_str.h"
#include "ignore_unused.h"
#include "tlf_curses.h"


/* get countrynumber, QTH, CQ zone and continent for myself */
void getstationinfo() {
    dxcc_data *mydx;

    my.countrynr = getctydata(my.call);	/* whoami? */
    mydx = dxcc_by_index(my.countrynr);

    sprintf(my.cqzone, "%02d", mydx -> cq);
    strcpy(my.continent, mydx->continent);
    my.Lat = mydx->lat; 	/* whereami? */
    my.Long = mydx->lon;
}


void getmessages(void) {

    getstationinfo();

    printw("\n     Call = ");
    printw(my.call);

    printw("     My Zone = ");
    printw(my.cqzone);

    printw("     My Continent = ");
    printw(my.continent);

    printw("\n\n");
    refreshp();
}
