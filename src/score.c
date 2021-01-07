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

#include "focm.h"
#include "globalvars.h"
#include "getctydata.h"
#include "locator2longlat.h"
#include "qrb.h"
#include "setcontest.h"
#include "tlf.h"

extern char countrylist[][6];
extern char continent_multiplier_list[7][3];

int calc_continent(int zone);

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
    char prefix[10];

    strcpy(prefix, pxstr);

    if (country_found(prefix)) {
	return true;
    }

    if (strlen(prefix) == 0) {
	return false;
    }

    if (!isdigit(prefix[strlen(prefix) - 1])) { /* last char '0'..'9' */
	return false;
    }

    prefix[strlen(prefix) - 1] = '\0';  /* strip trailing digit */
    if (country_found(prefix)) {	/* and try again */
	return true;
    }

    if (strlen(prefix) == 0) {
	return false;
    }

    if (!isdigit(prefix[strlen(prefix) - 1])) {
	return false;
    }

    prefix[strlen(prefix) - 1] = '\0';	/* last try */
    return country_found(prefix);
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


/* apply bandweigth scoring *
 * at the moment only LOWBAND_DOUBLES (<30m) can be active */
int apply_bandweigth(int points) {
    extern int lowband_point_mult;
    extern int bandinx;
    extern int bandweight_points[];

    if (lowband_point_mult != 0 && (bandinx < BANDINDEX_30))
	points *= 2;

    points *= bandweight_points[bandinx];

    return points;
}


/* portable stations may count double
 * see PORTABLE_X2 */
int portable_doubles(int points) {
    extern int portable_x2;
    char *loc;

    if (portable_x2 == 1) {	// portable x2
	loc = strstr(hiscall, "/P");
	if (loc == hiscall + strlen(hiscall) - 2) {
	    points *= 2;
	}
    }
    return points;
}


/* apply points by mode */
int scoreByMode() {
    extern int cwpoints;
    extern int ssbpoints;
    extern int trxmode;

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

    extern bool countrylist_only;
    extern int countrylist_points;

    extern bool continentlist_only;
    extern int continentlist_points;

    extern int my_country_points;
    extern int my_cont_points;
    extern int dx_cont_points;

    int points = 0;
    bool inCountryList = false;

    inCountryList = exist_in_country_list();

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

    extern int cwpoints;
    extern int ssbpoints;

    int points;

    if (ssbpoints != 0 && cwpoints != 0)	//  e.g. arrl 10m contest
	points = scoreByMode();
    else
	points = scoreByContinentOrCountry();

    points = apply_bandweigth(points);
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
    return 0;
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
    extern int dupe;

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
int calc_continent(int zone) {

    switch (zone) {
	case 1 ... 8:
	    strncpy(continent, "NA", 3);
	    break;
	case 9 ... 13:
	    strncpy(continent, "SA", 3);
	    break;
	case 14 ... 16:
	    strncpy(continent, "EU", 3);
	    break;
	case 17 ... 26:
	    strncpy(continent, "AS", 3);
	    break;
	case 27 ... 32:
	    strncpy(continent, "AS", 3);
	    break;
	case 33 ... 39:
	    strncpy(continent, "AF", 3);
	    break;
	case 40:
	    strncpy(continent, "EU", 3);
	    break;
	default:
	    strncpy(continent, "??", 3);
    }
    return 0;
}

