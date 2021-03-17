/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *               2013-2015, 2020
 *               		Thomas Beierlein <tb@forth-ev.de>
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
 *     score
 *
 *--------------------------------------------------------------*/


#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <hamlib/rotator.h>

#include "focm.h"
#include "globalvars.h"
#include "getctydata.h"
#include "locator2longlat.h"
#include "qrb.h"
#include "setcontest.h"
#include "tlf.h"

void calc_continent(int zone);

/* check if countrynr is in countrylist */
bool is_in_countrylist(int countrynr) {
    int i = 0;

    while (strlen(countrylist[i]) != 0) {
	if (getctynr(countrylist[i]) == countrynr) {
	    return true;
	}
	i++;
    }
    return false;
}


/* check if hiscall is in COUNTRY_LIST from logcfg.dat */
// FIXME: *** this function does not use its argument ***
bool country_found(char prefix[]) {
    char tmpcall[15];

    if (strlen(hiscall) == 0) {
	strcpy(tmpcall, my.call);
    } else
	strcpy(tmpcall, hiscall);

    countrynr = getctydata(tmpcall);

    return is_in_countrylist(countrynr);
}

bool exist_in_country_list() {
    char prefix[11];

    strcpy(prefix, wpx_prefix);
    int len = strlen(prefix);

    // make 3 iterations
    for (int i = 1; i <= 3; ++i) {
	if (country_found(prefix)) {
	    return true;
	}

	/* try to strip trailing digit */
	if (len == 0 || !isdigit(prefix[len - 1])) {
	    return false;   // empty or not a digit
	}

	prefix[len - 1] = '\0';
	--len;
    }

    return false;
}


/* HA2OS - check if continent is in CONTINENT_LIST from logcfg.dat */
bool is_in_continentlist(char *continent) {
    int i = 0;

    while (strlen(continent_multiplier_list[i]) != 0) {
	if (strcmp(continent_multiplier_list[i], continent) == 0) {
	    return true;
	}
	i++;
    }
    return false;
}


/* apply bandweight scoring *
 * at the moment only LOWBAND_DOUBLES (<30m) can be active */
int apply_bandweight(int points) {

    if (lowband_point_mult != 0 && (bandinx < BANDINDEX_30))
	points *= 2;

    points *= bandweight_points[bandinx];

    return points;
}


/* portable stations may count double
 * see PORTABLE_X2 */
int portable_doubles(int points) {
    if (portable_x2 && g_str_has_suffix(hiscall, "/P")) {
	points *= 2;
    }
    return points;
}


/* apply points by mode */
int scoreByMode() {

    switch (trxmode) {
	case CWMODE:
	    return cwpoints;
	case SSBMODE:
	    return ssbpoints;
	default:
	    return 0;
    }
}

/* Overwrite points with x if set */
#define USE_IF_SET(x) do { \
			if (x >= 0) \
			    points = x; \
			} while(0);

int scoreByContinentOrCountry() {

    int points = 0;
    bool inCountryList = exist_in_country_list();

    if (countrylist_only) {
	points = 0;
	if (inCountryList)
	    USE_IF_SET(countrylist_points);

	return points;
    }

    /* HA2OS mods */
    if (continentlist_only) {
	points = 0;
	// only continent list allowed
	if (is_in_continentlist(continent)) {
	    USE_IF_SET(continentlist_points);
	    // overwrite if own continent and my_cont_points set
	    if (strcmp(continent, my.continent) == 0) {
		USE_IF_SET(my_cont_points);
	    }
	}
	return points;
    }

    // default
    if (countrynr == my.countrynr) {
	points = 0;
	USE_IF_SET(my_cont_points);
	USE_IF_SET(my_country_points);
    } else if (inCountryList) {
	USE_IF_SET(countrylist_points);
    } else if (strcmp(continent, my.continent) == 0) {
	USE_IF_SET(my_cont_points);
    } else
	USE_IF_SET(dx_cont_points);

    return points;
}


/** default scoring code
 *
 * the default scoring rules will be applied if no contest specific rules
 * are active
 * \return points for QSO
 */
int scoreDefault() {

    int points;

    if (ssbpoints != 0 && cwpoints != 0)	//  e.g. arrl 10m contest
	points = scoreByMode();
    else
	points = scoreByContinentOrCountry();

    points = apply_bandweight(points);
    points = portable_doubles(points);

    return points;
}

int score_wpx() {
    int points;
    if (countrynr == my.countrynr) {
	points = 1;

	return points;
    }

    if ((strcmp(continent, my.continent) == 0)
	    && (bandinx > BANDINDEX_30)) {
	if (strstr(my.continent, "NA") != NULL) {
	    points = 2;
	} else {
	    points = 1;
	}

	return points;
    }

    if ((strcmp(continent, my.continent) == 0)
	    && (bandinx < BANDINDEX_30)) {
	if (strstr(my.continent, "NA") != NULL) {
	    points = 4;
	} else {
	    points = 2;
	}
	return points;
    }
    if ((strcmp(continent, my.continent) != 0)
	    && (bandinx > BANDINDEX_30)) {
	points = 3;

	return points;
    }
    if ((strcmp(continent, my.continent) != 0)
	    && (bandinx < BANDINDEX_30)) {
	points = 6;

	return points;
    }
    return 0;
}


int score_cqww() {
    int points;
    int zone;

    if (countrynr == 0) {
	zone = atoi(comment);
	calc_continent(zone);	// sets continent
    }

    if (countrynr == my.countrynr) {
	points = 0;

	return points;
    }

    if (strcmp(continent, my.continent) == 0) {
	if (strstr(my.continent, "NA") != NULL) {
	    points = 2;
	} else {
	    points = 1;
	}

	return points;
    } else {
	points = 3;

	return points;
    }
}


int score_arrlfd() {
    int points;

    if (trxmode == SSBMODE) {
	points = 1;
    } else {
	points = 2;
    }
    return points;
}


int score_arrldx_usa() {
    int points;

    if ((countrynr == w_cty) || (countrynr == ve_cty)) {
	points = 0;
    } else {
	points = 3;
    }

    return points;
}


int score_stewperry() {
    int points;
    double s1long, s1lat, s2long, s2lat, distance, azimuth;

    points = 0;

    if (strlen(comment) > 3) {
	locator2longlat(&s1long, &s1lat, comment);
	locator2longlat(&s2long, &s2lat, my.qra);

	qrb(s1long, s1lat, s2long, s2lat, &distance, &azimuth);

	points = (int) ceil(distance / 500.0);
    }

    return points;
}


int score() {

    int points;

    if (dupe == ISDUPE) {
	points = 0;
	dupe = NODUPE;
	return points;
    }

    band_score[bandinx]++;	/* qso's per band  */

    if (CONTEST_IS(ARRLDX_USA)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	band_score[bandinx]--;


    if (contest->points.type == FUNCTION) {
	return contest->points.fn();
    }

    if (contest->points.type == FIXED) {
	return contest->points.point;
    }

    /* start of the universal scoring code */
    return scoreDefault();
}


/* -----------------------------------------------------------------*/
int score2(char *line) {
    return atoi(line + 75);
}


/* ----------------------------------------------------------------- */
/* calculates continent from zone and sets 'continent' variable      */
void calc_continent(int zone) {

    switch (zone) {
	case 1 ... 8:
	    strcpy(continent, "NA");
	    break;
	case 9 ... 13:
	    strcpy(continent, "SA");
	    break;
	case 14 ... 16:
	    strcpy(continent, "EU");
	    break;
	case 17 ... 26:
	    strcpy(continent, "AS");
	    break;
	case 27 ... 32:
	    strcpy(continent, "AS");
	    break;
	case 33 ... 39:
	    strcpy(continent, "AF");
	    break;
	case 40:
	    strcpy(continent, "EU");
	    break;
	default:
	    strcpy(continent, "??");
    }
}
