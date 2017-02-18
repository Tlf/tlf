/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
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
	 *              Parse various  call  formats
	 *              Convert country data
	 *--------------------------------------------------------------*/


#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "dxcc.h"
#include "getpx.h"
#include "globalvars.h"		// Includes glib.h and tlf.h


int getpfxindex(char *checkcallptr)
{
    char checkbuffer[17] = "";
    char checkncall[17] = "";
    char checkcall[17] = "";
    char findcall[17] = "";

    prefix_data *pfx;
    int pfxmax = prefix_count();

    int i = 0, w = 0, abnormal_call = 0;
    char portable = '\0';
    int pp = 0;
    size_t loc;

    g_strlcpy(checkcall, checkcallptr, 17);

    portable = '\0';

    if (strstr(checkcall, "/QRP") != NULL)	/* drop QRP suffix */
	checkcall[strlen(checkcall) - 4] = '\0';

    if (strstr(checkcall, "/AM") != NULL)	// airborne mobile, no country (0), no zone (0)
	checkcall[0] = '\0';

    if (strstr(checkcall, "/MM") != NULL)	// maritime mobile, no country, no zone
	checkcall[0] = '\0';

    strncpy(findcall, checkcall, 16);

    loc = strcspn(checkcall, "/");

    if (loc != strlen(checkcall)) {
	char call1[17];
	char call2[17];


	strncpy(call1, checkcall, loc);		/* 1st part before '/' */
	call1[loc] = '\0';
	strcpy(call2, checkcall + loc + 1);	/* 2nd part after '/' */

	if (strlen(call2) < strlen(call1)
	    && strlen(call2) > 1) {
	    sprintf(checkcall, "%s/%s", call2, call1);
	    abnormal_call = 1;
	    loc = strcspn(checkcall, "/");
	}

	if (loc > 3) {

	    strncpy(checkbuffer, (checkcall + loc + 1),
		    (strlen(checkcall) + 1) - loc);

	    if (strlen(checkbuffer) == 1)
		checkcall[loc] = '\0';
	    if (checkbuffer[0] == 'M' && strlen(checkbuffer) <= 3)
		checkcall[loc] = '\0';
	    if (checkbuffer[0] == 'Q' && strlen(checkbuffer) == 3)	/* /QRP */
		checkcall[loc] = '\0';
	    if (checkbuffer[0] == 'A' && strlen(checkbuffer) <= 3)	/*  /A,  /AM etc */
		checkcall[loc] = '\0';
	    if ((strlen(checkbuffer) <= 3) && (checkbuffer[0] <= '9') && (checkbuffer[0] >= '0'))	/*  /3,   etc */
		portable = checkbuffer[0];
	    loc = strcspn(checkcall, "/");
	}

	if (loc != strlen(checkcall)) {

	    if (loc < 5)
		checkcall[loc] = '\0';	/*  "PA/DJ0LN/P   */
	    else {		/*  DJ0LN/P       */
		strncpy(checkcall, checkcall, loc + 1);
	    }
	}

	/* ------------------------------------------------------------ */

	if ((strlen(checkbuffer) == 1) && isdigit(checkbuffer[0])) {	/*  /3 */
	    for (pp = strlen(checkcall) - 1; pp > 0; pp--) {
		if (isdigit(checkcall[pp])) {
		    checkcall[pp] = checkbuffer[0];
		    break;
		}
	    }
	} else if (strlen(checkbuffer) > 1)
	    strcpy(checkcall, checkbuffer);

    }

    /* -------------check full call exceptions first...--------------------- */

    w = -1;
    if (abnormal_call == 1) {
	// pa3fwm 20040111: is pp guaranteed to be properly initialized
	// if/when we get here??
	// pa0r 20040117: It is not. Code changed...
	//      strncpy(checkncall , findcall, pp);
	strncpy(checkncall, findcall, sizeof(checkncall) - 1);

	for (i = 0; i < pfxmax; i++) {
	    pfx = prefix_by_index(i);
	    if (strcmp(checkncall, pfx->pfx) == 0) {
		w = i;
		break;
	    }
	}

    } else {
	int bestlen = 0;
	for (i = 0; i < pfxmax; i++) {
	    int l;
	    pfx = prefix_by_index(i);
	    if (*pfx->pfx != findcall[0])
		continue;

	    l = strlen(pfx->pfx);
	    if (l <= bestlen)
		continue;

	    if (strncmp(pfx->pfx, findcall, l) == 0) {
		bestlen = l;
		w = i;
	    }
	}
    }

    if (w < 0 && 0 != strcmp(findcall, checkcall)) {
	// only if not found in prefix full call exception list
	int bestlen = 0;
	for (i = 0; i < pfxmax; i++) {
	    int l;
	    pfx = prefix_by_index(i);
	    if (*pfx->pfx != checkcall[0])
		continue;
	    l = strlen(pfx->pfx);
	    if (l <= bestlen)
		continue;
	    if (strncmp(pfx->pfx, checkcall, l) == 0) {
		bestlen = l;
		w = i;
	    }
	}
    }

    return w;
}

int getctynr(char *checkcall)
{
    int w;

    w = getpfxindex(checkcall);

    if (w >= 0)
	return prefix_by_index(w)->dxcc_index;
    else
	return 0;	/* no country found */
}


int getctydata(char *checkcallptr)
{
    char checkbuffer[17] = "";
    char checkncall[17] = "";
    char checkcall[17] = "";
    char findcall[17] = "";

    prefix_data *pfx;
    int pfxmax = prefix_count();

    int i = 0, w = 0, x = 0, abnormal_call = 0;
    char portable = '\0';
    int pp = 0;
    size_t loc;

    g_strlcpy(checkcall, checkcallptr, 17);

    portable = '\0';

    if (strstr(checkcall, "/QRP") != NULL)	/* drop QRP suffix */
	checkcall[strlen(checkcall) - 4] = '\0';

    if (strstr(checkcall, "/AM") != NULL)	// airborne mobile, no country (0), no zone (0)
	checkcall[0] = '\0';

    if (strstr(checkcall, "/MM") != NULL)	// maritime mobile, no country, no zone
	checkcall[0] = '\0';

    strncpy(findcall, checkcall, 16);

    loc = strcspn(checkcall, "/");

    if (loc != strlen(checkcall)) {
	char call1[17];
	char call2[17];

	strncpy(call1, checkcall, loc);		/* 1st part before '/' */
	call1[loc] = '\0';
	strcpy(call2, checkcall + loc + 1);	/* 2nd part after '/' */

	if (strlen(call2) < strlen(call1)
	    && strlen(call2) > 1) {
	    sprintf(checkcall, "%s/%s", call2, call1);
	    abnormal_call = 1;
	    loc = strcspn(checkcall, "/");
	}

	if (loc > 3) {

	    strncpy(checkbuffer, (checkcall + loc + 1),
		    (strlen(checkcall) + 1) - loc);

	    if (strlen(checkbuffer) == 1)
		checkcall[loc] = '\0';
	    if (checkbuffer[0] == 'M' && strlen(checkbuffer) <= 3)
		checkcall[loc] = '\0';
	    if (checkbuffer[0] == 'Q' && strlen(checkbuffer) == 3)	/* /QRP */
		checkcall[loc] = '\0';
	    if (checkbuffer[0] == 'A' && strlen(checkbuffer) <= 3)	/*  /A,  /AM etc */
		checkcall[loc] = '\0';
	    if ((strlen(checkbuffer) <= 3) && (checkbuffer[0] <= '9') && (checkbuffer[0] >= '0'))	/*  /3,   etc */
		portable = checkbuffer[0];
	    loc = strcspn(checkcall, "/");
	}

	if (loc != strlen(checkcall)) {

	    if (loc < 5)
		checkcall[loc] = '\0';	/*  "PA/DJ0LN/P   */
	    else {		/*  DJ0LN/P       */
		strncpy(checkcall, checkcall, loc + 1);
	    }
	}

	/* ------------------------------------------------------------ */

	if ((strlen(checkbuffer) == 1) && isdigit(checkbuffer[0])) {	/*  /3 */
	    for (pp = strlen(checkcall) - 1; pp > 0; pp--) {
		if (isdigit(checkcall[pp])) {
		    checkcall[pp] = checkbuffer[0];
		    break;
		}
	    }
	} else if (strlen(checkbuffer) > 1)
	    strcpy(checkcall, checkbuffer);

    }

    if (wpx == 1 || pfxmult == 1)
    	/* needed for wpx and other pfx contests */
	getpx(checkcall);

    /* -------------check full call exceptions first...--------------------- */

    w = -1;
    if (abnormal_call == 1) {
	// pa3fwm 20040111: is pp guaranteed to be properly initialized
	// if/when we get here??
	// pa0r 20040117: It is not. Code changed...
	//      strncpy(checkncall , findcall, pp);
	strncpy(checkncall, findcall, sizeof(checkncall) - 1);

	for (i = 0; i < pfxmax; i++) {
	    pfx = prefix_by_index(i);
	    if (strcmp(checkncall, pfx->pfx) == 0) {
		w = i;
		x = pfx->dxcc_index;
		break;
	    }
	}

    } else {
	int bestlen = 0;
	for (i = 0; i < pfxmax; i++) {
	    int l;
	    pfx = prefix_by_index(i);
	    if (*pfx->pfx != findcall[0])
		continue;

	    l = strlen(pfx->pfx);
	    if (l <= bestlen)
		continue;

	    if (strncmp(pfx->pfx, findcall, l) == 0) {
		bestlen = l;
		w = i;
	    }
	}
	if (w >= 0)
	    x = prefix_by_index(w)->dxcc_index;
    }

    if (w < 0 && 0 != strcmp(findcall, checkcall)) {
	// only if not found in prefix full call exception list
	int bestlen = 0;
	for (i = 0; i < pfxmax; i++) {
	    int l;
	    pfx = prefix_by_index(i);
	    if (*pfx->pfx != checkcall[0])
		continue;
	    l = strlen(pfx->pfx);
	    if (l <= bestlen)
		continue;
	    if (strncmp(pfx->pfx, checkcall, l) == 0) {
		bestlen = l;
		w = i;
	    }
	}
	if (w >= 0)
	    x = prefix_by_index(w)->dxcc_index;
    }

    if (w >= 0 ) {
	sprintf(cqzone, "%02d", prefix_by_index(w) -> cq);
	sprintf(ituzone, "%02d", prefix_by_index(w) -> itu);
    }

    if (itumult != 1)
	strcpy(zone_export, cqzone);
    else
	strcpy(zone_export, ituzone);

// w must be >0  tb 17feb2011
//    strncpy(ituzone, ituarray[w], 2);

    countrynr = x;
    g_strlcpy(continent, dxcc_by_index(countrynr) -> continent , 3);

    return (x);
}

