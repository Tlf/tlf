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

#include "dxcc.h"
#include "get_time.h"
#include "printcall.h"
#include "tlf.h"
#include "tlf_curses.h"

double ssn_r = 50;
time_t lastwwv_time = 0;
char lastwwv[100] = "";
char lastwwv_raw[100] = "";

void wwv_set_r(double r) {
    ssn_r = (r > 0 ? r : 0);
}

void wwv_set_sfi(double sfi) {
    wwv_set_r((sfi - 70.0) * (200.0 / 180.0));
}

void wwv_add(const char *s) {
    if (strncmp(s, "WWV", 3) != 0 && strncmp(s, "WCY", 3) != 0) {
	return;     // not a WWV message
    }

    // save message skipping control chars
    char *dest = lastwwv_raw;
    while (*s) {
	if (*s >= ' ') {*dest = *s; ++dest;}
	++s;
    }
    *dest = 0;

    lastwwv_time = get_time();

    char gmt[10] = "";
    char *p = strstr(lastwwv_raw, "<");
    if (p != NULL) {
	sprintf(gmt, "%02d GMT", atoi(p + 1));
    }

    // Sunspot Number
    char rstr[10] = "";
    p = strstr(lastwwv_raw, "R=");
    if (p != NULL) {
	wwv_set_r(atof(p + 2));
	sprintf(rstr, "R=%.0f", ssn_r);
    }

    // Solar Flux Index
    char sfistr[20] = "";
    p = strstr(lastwwv_raw, "SFI=");
    if (p != NULL) {
	double sfi = atof(p + 4);
	sprintf(sfistr, "SFI=%.0f", sfi);
	wwv_set_sfi(sfi);
    }

    // Solar Activity (SA)
    char *sa = "";
    p = strstr(lastwwv_raw, "eru");
    if (p != NULL) {
	sa = "eruptive";
    }

    // Geomagnetic Field (GMF)
    char *gmf = "";
    p = strstr(lastwwv_raw, "act");
    if (p != NULL) {
	gmf = "act";
    }

    // Aurora
    char *au = "";
    p = strstr(lastwwv_raw, "Au=au");
    if (p != NULL) {
	au = "AURORA!";
    }

    sprintf(lastwwv, "Condx: %-18s  %-8s %-10s %-9s %-5s %s",
	    gmt, rstr, sfistr,
	    sa, gmf, au);

}

//
// show footer if WWV was received not later than 3 mins ago
//
void wwv_show_footer() {
    if (lastwwv_time > get_time() - 3 * 60) {
	mvprintw(LINES - 1, 0, "%s", lastwwv);
    }
}
