/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *               2013-2023	Thomas Beierlein <tb@forth-ev.de>
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
#include "qrb.h"
#include "plugin.h"
#include "setcontest.h"
#include "tlf.h"

char *calc_continent(int zone);

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


/* check if current_qso.call is in COUNTRYLIST from logcfg.dat */
// FIXME: *** this function does not use its argument ***
bool country_found(char prefix[]) {
    char tmpcall[15];

    if (strlen(current_qso.call) == 0) {
	strcpy(tmpcall, my.call);
    } else
	strcpy(tmpcall, current_qso.call);

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


/* check if continent is in CONTINENT_LIST */
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


/* apply bandweight scoring */
int apply_bandweight(int points, int bandindex) {

    if (lowband_point_mult && (bandindex < BANDINDEX_30))
	points *= 2;

    points *= bandweight_points[bandindex];

    return points;
}


/* portable stations may count double
 * see PORTABLE_X2 */
int portable_doubles(int points, char *call) {
    if (portable_x2 && g_str_has_suffix(call, "/P")) {
	points *= 2;
    }
    return points;
}


/* apply points by mode */
int scoreByMode(struct qso_t *qso) {

    switch (qso->mode) {
	case CWMODE:
	    return cwpoints;
	case SSBMODE:
	    return ssbpoints;
	default:
	    return 0;
    }
}


/* return x points if set */
#define RETURN_IF_SET(x) do { \
			if (x >= 0) \
			    return x; \
			} while(0);

int scoreByContinentOrCountry(struct qso_t *qso) {

    prefix_data *ctyinfo = getctyinfo(qso->call);
    bool inCountryList = is_in_countrylist(ctyinfo->dxcc_ctynr);

    if (ctyinfo->dxcc_ctynr == my.countrynr) {
	RETURN_IF_SET(my_country_points);
    }

    if (inCountryList) {
	RETURN_IF_SET(countrylist_points);
    }

    if (strcmp(ctyinfo->continent, my.continent) == 0) {
	RETURN_IF_SET(my_cont_points);
    }

    if (is_in_continentlist(ctyinfo->continent)) {
	RETURN_IF_SET(continentlist_points);
    }

    if (strcmp(ctyinfo->continent, my.continent) != 0) {
	RETURN_IF_SET(dx_cont_points);
    }
    return 0;
}


/** default scoring code
 *
 * the default scoring rules will be applied if no contest specific rules
 * are active
 * \return points for QSO
 */
int scoreDefault(struct qso_t *qso) {

    int points;

    if (ssbpoints != 0 && cwpoints != 0)	//  e.g. arrl 10m contest
	points = scoreByMode(qso);
    else
	points = scoreByContinentOrCountry(qso);

    points = apply_bandweight(points, qso->bandindex);
    points = portable_doubles(points, qso->call);

    return points;
}

int score_wpx(struct qso_t *qso) {
    int points;

    prefix_data *ctyinfo = getctyinfo(qso->call);
    int countrynr = ctyinfo->dxcc_ctynr;

    if (countrynr == my.countrynr) {
	points = 1;

	return points;
    }

    if ((strcmp(ctyinfo->continent, my.continent) == 0)
	    && (qso->bandindex > BANDINDEX_30)) {
	if (strstr(my.continent, "NA") != NULL) {
	    points = 2;
	} else {
	    points = 1;
	}

	return points;
    }

    if ((strcmp(ctyinfo->continent, my.continent) == 0)
	    && (qso->bandindex < BANDINDEX_30)) {
	if (strstr(my.continent, "NA") != NULL) {
	    points = 4;
	} else {
	    points = 2;
	}
	return points;
    }
    if ((strcmp(ctyinfo->continent, my.continent) != 0)
	    && (qso->bandindex > BANDINDEX_30)) {
	points = 3;

	return points;
    }
    if ((strcmp(ctyinfo->continent, my.continent) != 0)
	    && (qso->bandindex < BANDINDEX_30)) {
	points = 6;

	return points;
    }
    return 0;
}


int score_cqww(struct qso_t *qso) {
    int points;
    int zone;

    prefix_data *ctyinfo = getctyinfo(qso->call);
    int countrynr = ctyinfo->dxcc_ctynr;
    char *continent = ctyinfo->continent;

    if (countrynr == 0) {
	zone = atoi(qso->comment);
	continent = calc_continent(zone);	// sets continent
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

    } else {
	points = 3;
    }

    return points;
}


int score_arrlfd(struct qso_t *qso) {
    int points;

    if (qso->mode == SSBMODE) {
	points = 1;
    } else {
	points = 2;
    }
    return points;
}


int score_arrldx_usa(struct qso_t *qso) {
    int points;

    prefix_data *ctyinfo = getctyinfo(qso->call);
    int countrynr = ctyinfo->dxcc_ctynr;

    if ((countrynr == w_cty) || (countrynr == ve_cty)) {
	points = 0;
    } else {
	points = 3;
    }

    return points;
}


int score_stewperry(struct qso_t *qso) {
    int points;
    double s1long, s1lat, s2long, s2lat, distance, azimuth;

    points = 0;

    if (strlen(qso->comment) > 3) {
	locator2longlat(&s1long, &s1lat, qso->comment);
	locator2longlat(&s2long, &s2lat, my.qra);

	qrb(s1long, s1lat, s2long, s2lat, &distance, &azimuth);

	points = (int) ceil(distance / 500.0);
    }

    return points;
}


int score(struct qso_t *qso) {

    int points;

    if (dupe == ISDUPE) {
	points = 0;
	dupe = NODUPE;
	return points;
    }

    if (plugin_has_score()) {
	return plugin_score(qso);
    }

    if (contest->points.type == FUNCTION) {
	return contest->points.fn(qso);
    }

    if (contest->points.type == FIXED) {
	return contest->points.point;
    }

    /* start of the universal scoring code */
    return scoreDefault(qso);
}

/* score QSO and add to total points */
void score_qso(struct qso_t *qso) {
    qso_points = score(qso);		/* update qso's per band and score */
    total = total + qso_points;
}

/* -----------------------------------------------------------------*/
int score2(char *line) {
    return atoi(line + 75);
}


/* ----------------------------------------------------------------- */
/* calculates continent from zone and sets 'continent' variable      */
char *calc_continent(int zone) {
    char *continent;

    switch (zone) {
	case 1 ... 8:
	    continent = "NA";
	    break;
	case 9 ... 13:
	    continent = "SA";
	    break;
	case 14 ... 16:
	    continent = "EU";
	    break;
	case 17 ... 32:
	    continent = "AS";
	    break;
	case 33 ... 39:
	    continent = "AF";
	    break;
	case 40:
	    continent = "EU";
	    break;
	default:
	    continent = "??";
    }
    return continent;
}
