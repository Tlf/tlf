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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	/* ------------------------------------------------------------
	 *              Parse various  call  formats
	 *              Convert country data
	 *--------------------------------------------------------------*/

#include "globalvars.h"
#include "getctydata.h"

int getctydata(char *checkcallptr)
{
    char checkbuffer[17] = "";
    char checkncall[17] = "";
    char checkcall[17] = "";
    char findcall[17] = "";

    int i = 0, w = 0, x = 0, ii = 0, abnormal_call = 0;
    char portable = '\0';
    int pp = 0;
    size_t loc;

    strncpy(checkcall, checkcallptr, 16);
    checkcall[16] = 0;

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

	for (i = 0; i < MAX_DBLINES; i++) {
	    if (strcmp(checkncall, prefixlines[i]) == 0) {
		w = i;
		x = dataindex[i];
		break;
	    }
	}

    } else {
	int bestlen = 0;
	for (i = 0; i < MAX_DBLINES; i++) {
	    int l;
	    if (prefixlines[i][0] != findcall[0])
		continue;
	    l = strlen(prefixlines[i]);
	    if (l <= bestlen)
		continue;
	    if (strncmp(prefixlines[i], findcall, l) == 0) {
		bestlen = l;
		w = i;
	    }
	}
	if (w >= 0)
	    x = dataindex[w];
    }

    if (w < 0 && 0 != strcmp(findcall, checkcall)) {
	// only if not found in prefix full call exception list
	int bestlen = 0;
	for (i = 0; i < MAX_DBLINES; i++) {
	    int l;
	    if (prefixlines[i][0] != checkcall[0])
		continue;
	    l = strlen(prefixlines[i]);
	    if (l <= bestlen)
		continue;
	    if (strncmp(prefixlines[i], checkcall, l) == 0) {
		bestlen = l;
		w = i;
	    }
	}
	if (w >= 0)
	    x = dataindex[w];
    }
// pa3fwm, 20040113: it seems there is no guarantee that w>=0 when we get here; or is that guarantee implicit by completeness of the prefix list?
// pa0r, 20040117:   it will be -1 if there is a non-existing prefix in the cty.dat file.
//                                              so let's use the 'normal' pfx in that case (x defaults to 0).
//      if (strlen(zonearray[w]) > 0) {
    if (w > 0 && strlen(zonearray[w]) > 0) {
	strncpy(cqzone, zonearray[w], 2);
	strncpy(ituzone, ituarray[w], 2);
    } else {
	strncpy(cqzone, datalines[x] + 26, 2);
	strncpy(ituzone, datalines[x] + 31, 2);

	if (x == w_cty) {
	    for (ii = 0; ii < strlen(checkcall); ii++) {
		if (checkcall[ii] == '7' || checkcall[ii] == '6') {
		    strncpy(cqzone, "03", 2);
		    break;
		} else if (checkcall[ii] == '5' || checkcall[ii] == '8'
			   || checkcall[ii] == '9'
			   || checkcall[ii] == '0') {
		    strncpy(cqzone, "04", 2);
		    break;
		} else if (checkcall[ii] == '1' || checkcall[ii] == '2'
			   || checkcall[ii] == '3'
			   || checkcall[ii] == '4') {
		    strncpy(cqzone, "05", 2);
		    break;
		}
	    }

	}

	if (x == ve_cty) {
	    for (ii = 0; ii < strlen(checkcall); ii++) {
		if (checkcall[ii] == '7') {
		    strncpy(cqzone, "03", 2);
		    break;
		} else if (checkcall[ii] == '3' || checkcall[ii] == '4'
			   || checkcall[ii] == '5'
			   || checkcall[ii] == '6') {
		    strncpy(cqzone, "04", 2);
		    break;
		} else
		    strncpy(cqzone, "05", 2);

	    }
	}
    }				// end exception

    if (itumult != 1)
	strcpy(zone_export, cqzone);
    else
	strcpy(zone_export, ituzone);

    strncpy(ituzone, ituarray[w], 2);

    countrynr = x;
    strncpy(continent, datalines[countrynr] + 36, 3);
    continent[2] = '\0';

    return (x);
}

/* --------------------for background ---------------------------*/

// pa3fwm, 20040113: I didn't "clean" this part yet
int getctydata2(char *checkcall)
{
    extern char prefixlines[MAX_DBLINES][17];
    extern char cqzone[];
    extern char zonearray[MAX_DBLINES][3];
    extern int dataindex[MAX_DBLINES];
    extern int countrynr;
    extern char datalines[MAX_DATALINES][81];

    char checkbuffer[17] = "";
    char membuffer[17] = "";
    char checkncall[20];

    int i = 0, w = 0, x = 0;
    int exception;
    char portable = '\0';
    int pp = 0;
    size_t loc;

    strncpy(membuffer, checkcall, 16);
    portable = '\0';

    if (strstr(checkcall, "/QRP") != NULL)	//strip the qrp
	checkcall[strlen(checkcall) - 4] = '\0';

    loc = strcspn(checkcall, "/");

    if (loc > 4) {

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
	if ((strlen(checkbuffer) <= 3) && (checkbuffer[0] <= 57) && (checkbuffer[0] >= 48))	/*  /3,   etc */
	    portable = checkbuffer[0];

    }
    loc = strcspn(checkcall, "/");

    if (loc != strlen(checkcall)) {

	if (loc < 5)
	    checkcall[loc] = '\0';	/*  "PA/DJ0LN/P   */
	else {			/*  DJ0LN/P       */

	    strncpy(checkcall, checkcall, loc + 1);

	}
    }

    /* ------------------------------------------------------------ */

    if (strlen(checkbuffer) == 1) {	/*  /3 */
	for (pp = strlen(checkcall) - 1; pp > 0; pp--) {

	    if ((checkcall[pp] <= '9') && (checkcall[pp] >= '0'))
		if ((checkbuffer[0] <= '9') && (checkbuffer[0] >= '0')) {
		    checkcall[pp] = checkbuffer[0];
		    break;
		}
	}
    } else if (strlen(checkbuffer) > 1)
	strcpy(checkcall, checkbuffer);

    getpx(checkcall);

    /* ------------------------------------------------------------ */

    w = 0;

    for (pp = 1; pp <= strlen(checkcall); pp++) {

	strncpy(checkncall, checkcall, pp);
	checkncall[pp] = '\0';

	for (i = 0; i < MAX_DBLINES; i++) {

	    if ((strncmp
		 (checkncall, prefixlines[i],
		  strlen(prefixlines[i])) == 0)) {

		if (strlen(checkncall) == strlen(prefixlines[i])) {
		    w = i;
		    x = dataindex[i];
		    break;
		}

	    }
	}
    }

    exception = 0;

    if (strlen(zonearray[w]) > 0) {
	exception = 1;
	strncpy(cqzone, zonearray[w], 2);
    } else
	strncpy(cqzone, datalines[x] + 26, 2);

    countrynr = x;

    strcpy(checkcall, membuffer);

    return (x);

}
