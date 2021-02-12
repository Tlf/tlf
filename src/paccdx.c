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


#include <string.h>

#include "getctydata.h"
#include "getpx.h"
#include "globalvars.h"
#include "tlf.h"
#include "bands.h"


int pacc_pa(void) {

    extern int zl_cty;
    extern int ja_cty;
    extern int py_cty;
    extern int ce_cty;
    extern int lu_cty;
    extern int vk_cty;
    extern int zs_cty;
    extern int ua9_cty;

    extern int pacc_qsos[10][10];

    int pxnr;
    /* FIXME: Initialisation is wrong as 0 is bandindex for 160m */
    int i = 0, j = 0, k;

    k = 1;

    switch (bandinx) {

	case BANDINDEX_160:
	case BANDINDEX_80:
	case BANDINDEX_40:
	case BANDINDEX_20:
	case BANDINDEX_15:
	case BANDINDEX_10:
	    i = inxes[bandinx];
	    j = bandinx;
	    break;
    }

    getpx(hiscall);

    pxnr = pxstr[strlen(pxstr) - 1] - 48;

    getctydata(hiscall);

    if (countrynr == w_cty) {

	if ((pacc_qsos[0][pxnr] & i) == 0) {
	    pacc_qsos[0][pxnr] = pacc_qsos[0][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == ve_cty) {
	if ((pacc_qsos[1][pxnr] & i) == 0) {
	    pacc_qsos[1][pxnr] = pacc_qsos[1][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == zl_cty) {
	if ((pacc_qsos[2][pxnr] & i) == 0) {
	    pacc_qsos[2][pxnr] = pacc_qsos[2][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == ja_cty) {
	if ((pacc_qsos[3][pxnr] & i) == 0) {
	    pacc_qsos[3][pxnr] = pacc_qsos[3][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == py_cty) {
	if ((pacc_qsos[4][pxnr] & i) == 0) {
	    pacc_qsos[4][pxnr] = pacc_qsos[4][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == ce_cty) {
	if ((pacc_qsos[5][pxnr] & i) == 0) {
	    pacc_qsos[5][pxnr] = pacc_qsos[5][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == lu_cty) {
	if ((pacc_qsos[6][pxnr] & i) == 0) {
	    pacc_qsos[6][pxnr] = pacc_qsos[6][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == vk_cty) {
	if ((pacc_qsos[7][pxnr] & i) == 0) {
	    pacc_qsos[7][pxnr] = pacc_qsos[7][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == zs_cty) {
	if ((pacc_qsos[8][pxnr] & i) == 0) {
	    pacc_qsos[8][pxnr] = pacc_qsos[8][pxnr] | i;
	    countryscore[j]++;
	    addcallarea = 1;
	}
	k = 0;

    } else if (countrynr == ua9_cty) {
	if ((pxnr == 9) || (pxnr == 0)) {
	    if ((pacc_qsos[9][pxnr] & i) == 0) {
		pacc_qsos[9][pxnr] = pacc_qsos[9][pxnr] | i;
//                      countryscore[j]++;
		addcallarea = 1;
	    }
	    k = 0;

	}
    } else {
	addcallarea = 0;
	k = 1;
    }

    return (k);
}
