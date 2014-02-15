/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *               2013-2014      Thomas Beierlein <tb@forth-ev.de>
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

#include "qrb.h"
#include "locator2longlat.h"
#include "score.h"
#include "tlf.h"
#include "getctydata.h"
#include "focm.h"
#include <math.h>


int calc_continent(int zone);

/* LZ3NY - check if call is in COUNTRY_LIST from logcfg.dat */
int country_found(char prefix[])
{

    extern int countrynr;
    extern char hiscall[];
    extern char call[];
    extern char mit_multiplier_list[][6];

    char tmpcall[15];
    int mit_fg = 0;

    if (strlen(hiscall) == 0) {
	strcpy(tmpcall, call);
    } else
	strcpy(tmpcall, hiscall);

    if (countrynr == 0)
	countrynr = getctydata(tmpcall);

    while (strlen(mit_multiplier_list[mit_fg]) != 0) {
	if (getctydata(mit_multiplier_list[mit_fg]) == getctydata(tmpcall)) {
	    return 1;
	}
	mit_fg++;
    }
    return 0;
}

int exist_in_country_list()
{
//    extern char mit_multiplier_list[][6];
    extern char pxstr[];
    char prefix[10];

    memset(prefix, '\0', 10);
    strcpy(prefix, pxstr);

    if (country_found(prefix) == 1) {
	return (1);
    } else {
	if ((prefix[strlen(prefix) - 1] < 58)	/* last char '0'..'9' */
	    && (prefix[strlen(prefix) - 1] > 47)) {
	    prefix[strlen(prefix) - 1] = '\0';  /* strip number */
	    if (country_found(prefix) == 1) {
		return 1;
	    } else {
		if ((prefix[strlen(prefix) - 1] < 58) /* see above */
		    && (prefix[strlen(prefix) - 1] > 47)) {
		    prefix[strlen(prefix) - 1] = '\0';
		    if (country_found(prefix) == 1)
			return (1);
		    else
			return (0);
		} else
		    return 0;
	    }
	} else
	    return 0;
    }
}

/* end LZ3NY code */

int score()
{

    extern int dupe;
    extern int one_point;
    extern int two_point;
    extern int three_point;
    extern int band_score[9];
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
    extern int cwpoints;
    extern int ssbpoints;
    extern int lowband_point_mult;
    extern int portable_x2;
    extern char hiscall[];
    extern char myqra[7];
    extern char call[];
    extern int stewperry_flg;


/* LZ3NY mods */

    extern int countrylist_points;
    extern int my_cont_points;
    extern int my_country_points;
    extern int dx_cont_points;
    extern int countrylist_only;

    int points;
    int is_mult = 0;
/* end LZ3NY mods */

    int zone;
    char *loc;

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

	if ((countrynr == mycountrynr)) {
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

	    points = ceil(distance/500.0);
	}

	return points;
    }

    /* end arrldx_usa */
    /* LZ3NY mods */
    is_mult = exist_in_country_list();
    if (countrylist_only == 1) {
	if (is_mult == 1 && countrylist_points != -1)
	    points = countrylist_points;
	else
	    points = 0;
    } else {

	if (is_mult == 1) {
	    if (countrylist_points != -1)
		points = countrylist_points;
	    else
		points = 0;

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
	    else
		points = 0;
	} else if (strcmp(continent, mycontinent) == 0) {
	    if (my_cont_points != -1)
		points = my_cont_points;
	    else
		points = 0;
	} else if (dx_cont_points != -1)
	    points = dx_cont_points;

	else
	    points = 0;
    }

    if (ssbpoints != 0 && cwpoints != 0)	//  e.g. arrl 10m contest
    {
	if (trxmode == CWMODE) {
	    points = cwpoints;
	} else if (trxmode == SSBMODE) {
	    points = ssbpoints;
	} else
	    points = 0;
    }

    if (lowband_point_mult != 0 && (bandinx < BANDINDEX_30))	// lowband 2x points
	points *= 2;

    if (portable_x2 == 1) {	// portable x2
	loc = strstr(hiscall, "/P");
	if (loc == hiscall + strlen(hiscall) - 2) {
	    points *= 2;
	}
    }

    return points;
}


/* -----------------------------------------------------------------*/
int score2(char *line)
{

    return atoi(line + 75);
}


/* ----------------------------------------------------------------- */
int calc_continent(int zone)
{			// calculates continent from zone and sets continent

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
    return (0);
}

