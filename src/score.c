/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *               2013-2015      Thomas Beierlein <tb@forth-ev.de>
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
#include "getctydata.h"
#include "locator2longlat.h"
#include "qrb.h"
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

    extern int countrynr;
    extern char hiscall[];
    extern char call[];

    char tmpcall[15];

    if (strlen(hiscall) == 0) {
	strcpy(tmpcall, call);
    } else
	strcpy(tmpcall, hiscall);

    countrynr = getctydata(tmpcall);

    return is_in_countrylist(countrynr);
}

bool exist_in_country_list() {

    extern char pxstr[];
    char prefix[10];

    strcpy(prefix, pxstr);

    if (country_found(prefix)) {
	return true;
    }

    if (!isdigit(prefix[strlen(prefix) - 1])) { /* last char '0'..'9' */
	return false;
    }

    prefix[strlen(prefix) - 1] = '\0';  /* strip trailing digit */
    if (country_found(prefix)) {	/* and try again */
	return true;
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
    extern char hiscall[];
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


int scoreByContinentOrCountry() {

    extern char hiscall[];

    extern int countrylist_points;
    extern bool countrylist_only;

    extern bool continentlist_only;
    extern int continentlist_points;

    extern int my_country_points;
    extern int my_cont_points;
    extern int dx_cont_points;

    extern int countrynr;
    extern int mycountrynr;
    extern char continent[];
    extern char mycontinent[];

    int points = 0;
    int inList = 0;

    inList = exist_in_country_list();
    if (countrylist_only) {
	if (inList == 1 && countrylist_points != -1)
	    points = countrylist_points;
    } else {

	if (inList == 1) {
	    if (countrylist_points != -1)
		points = countrylist_points;

	    if (countrynr == mycountrynr) {
		if (my_country_points != -1)
		    points = my_country_points;
		else if (my_cont_points != -1)
		    points = my_cont_points;
		else
		    points = 0;
	    }

	} else if (countrynr == mycountrynr) {
	    if (my_country_points != -1)
		points = my_country_points;
	    else if (my_cont_points != -1)
		points = my_cont_points;
	} else if (strcmp(continent, mycontinent) == 0) {
	    if (my_cont_points != -1)
		points = my_cont_points;
	} else if (dx_cont_points != -1)
	    points = dx_cont_points;
    }

    /* HA2OS mods */
    if (continentlist_only) {
	// only continent list allowed
	if (is_in_continentlist(continent)) {
	    // are we are on DX continent or not
	    if (strcmp(continent, mycontinent) == 0) {
		points = my_cont_points;
	    } else if (continentlist_points != -1) {
		points = continentlist_points;
	    }
	} else {
	    points = 0;
	}
    }

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
    extern int one_point;
    extern int two_point;
    extern int three_point;

    int points;

    if (one_point == 1) {
	points = 1;
	return points;
    }

    if (two_point == 1) {
	points = 2;
	return points;
    }

    if (three_point == 1) {
	points = 3;
	return points;
    }

    if (ssbpoints != 0 && cwpoints != 0)	//  e.g. arrl 10m contest
	points = scoreByMode();
    else
	points = scoreByContinentOrCountry();

    points = apply_bandweigth(points);
    points = portable_doubles(points);

    return points;
}


int score() {

    extern int dupe;
    extern int band_score[NBANDS];
    extern int bandinx;
    extern int focm;
    extern int wpx;
    extern int pfxmult;
    extern int countrynr;
    extern int mycountrynr;
    extern char continent[];
    extern char mycontinent[];
    extern char comment[];
    extern int cqww;
    extern int arrl_fd;
    extern int arrldx_usa;
    extern int w_cty;
    extern int ve_cty;
    extern int trxmode;
    extern char hiscall[];
    extern char myqra[7];
    extern int stewperry_flg;

    int points;
    int zone;

    if (dupe == ISDUPE) {
	points = 0;
	dupe = NODUPE;
	return points;
    }

    band_score[bandinx]++;	/* qso's per band  */

    if ((arrldx_usa == 1)
	    && ((countrynr == w_cty) || (countrynr == ve_cty)))
	band_score[bandinx]--;

    if (focm == 1) {
	points = foc_score(hiscall);

	return points;
    }

    if (wpx == 1 && pfxmult == 0) {
	if (countrynr == mycountrynr) {
	    points = 1;

	    return points;
	}

	if ((strcmp(continent, mycontinent) == 0)
		&& (bandinx > BANDINDEX_30)) {
	    if (strstr(mycontinent, "NA") != NULL) {
		points = 2;
	    } else {
		points = 1;
	    }

	    return points;
	}

	if ((strcmp(continent, mycontinent) == 0)
		&& (bandinx < BANDINDEX_30)) {
	    if (strstr(mycontinent, "NA") != NULL) {
		points = 4;
	    } else {
		points = 2;
	    }
	    return points;
	}
	if ((strcmp(continent, mycontinent) != 0)
		&& (bandinx > BANDINDEX_30)) {
	    points = 3;

	    return points;
	}
	if ((strcmp(continent, mycontinent) != 0)
		&& (bandinx < BANDINDEX_30)) {
	    points = 6;

	    return points;
	}
    }				// end wpx

    if (cqww == 1) {

	if (countrynr == 0) {
	    zone = atoi(comment);
	    calc_continent(zone);	// sets continent
	}

	if (countrynr == mycountrynr) {
	    points = 0;

	    return points;
	}

	if (strcmp(continent, mycontinent) == 0) {
	    if (strstr(mycontinent, "NA") != NULL) {
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

    /* end cqww */
    if (arrl_fd == 1) {

	if (trxmode == SSBMODE) {
	    points = 1;
	} else {
	    points = 2;

	}
	return points;

    }				// end arrl_fd

    if (arrldx_usa == 1) {

	if ((countrynr == w_cty) || (countrynr == ve_cty)) {
	    points = 0;

	} else {
	    points = 3;
	}

	return points;
    }

    if (stewperry_flg == 1) {

	double s1long, s1lat, s2long, s2lat, distance, azimuth;

	points = 0;

	if (strlen(comment) > 3) {
	    locator2longlat(&s1long, &s1lat, comment);
	    locator2longlat(&s2long, &s2lat, myqra);

	    qrb(s1long, s1lat, s2long, s2lat, &distance, &azimuth);

	    points = (int) ceil(distance / 500.0);
	}

	return points;
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
    extern char continent[];

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

