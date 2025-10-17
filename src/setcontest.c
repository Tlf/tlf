/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
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
 *      Set contest parameters
 *
 *--------------------------------------------------------------*/


#include <stdbool.h>
#include <string.h>

#include "addpfx.h"
#include "bandmap.h"
#include "bands.h"
#include "focm.h"
#include "getctydata.h"
#include "getpx.h"
#include "globalvars.h"
#include "setcontest.h"
#include "score.h"
#include "tlf.h"

#include "getexchange.h"
#include "addmult.h"
#include "log_utils.h"

// create a qso_t from a spot for multiplier checking
static struct qso_t *qso_from_spot(spot *data) {
    struct qso_t *qso = g_malloc0(sizeof(struct qso_t));
    qso->call = g_strdup(data->call);
    qso->comment = g_strdup("");    // TODO recall exchange if possible
    qso->freq = data->freq;
    qso->bandindex = data->bandindex;
    qso->band = bandindex2nr(data->bandindex);
    return qso;
}

/* No Multiplier mark in bandmap for multis determined from comment field;
 * Code works also for modes with no multiplier at all */
bool no_multi(spot *data) {
    return false;
}


static bool pfx_on_band_ismulti(spot *data) {
    int bandindex = data->bandindex;
    char *call = data->call;

    char *prefix = get_wpx_pfx(call);
    bool multi = pfx_is_new_on(prefix, bandindex);
    g_free(prefix);
    return multi;
}


static bool wpx_ismulti(spot *data) {
    char *prefix = get_wpx_pfx(data->call);
    bool multi = pfx_is_new(prefix);
    g_free(prefix);
    return multi;
}


static bool cqww_ismulti(spot *data) {
    int bandindex = data->bandindex;

    if ((zones[data->cqzone] & inxes[bandindex]) == 0
	    || (countries[data->ctynr] & inxes[bandindex]) == 0) {
	return true;
    }

    return false;
}

static bool arrldx_usa_ismulti(spot *data)  {
    int ctynr = data->ctynr;
    int bandindex = data->bandindex;

    if ((countries[ctynr] & inxes[bandindex]) != 0)
	return false;

    if (ctynr == w_cty || ctynr == ve_cty)
	return false;

    return true;
}


bool general_ismulti(spot *data) {
    int bandindex = data->bandindex;

    if (dx_arrlsections) {
	/* no evaluation of sections, check only country */
	return arrldx_usa_ismulti(data);
    }

    if (country_mult) {
	return ((countries[data->ctynr] & inxes[bandindex]) == 0);
    }

    if (pfxmult) {
	return wpx_ismulti(data);
    }

    if (pfxmultab) {
	return pfx_on_band_ismulti(data);
    }

    if (itumult || wazmult) {
	return ((zones[data->cqzone] & inxes[bandindex]) == 0);
    }

    struct qso_t *qso = qso_from_spot(data);
    checkexchange(qso, false);
    bool new_mult = (check_mult(qso) >= 0);
    free_qso(qso);

    return new_mult;
}



/* configurations for supported contest */
contest_config_t config_unknown = {
    .id = UNKNOWN,
    .name = "Unknown",
    .exchange_width = 14,
};

contest_config_t config_qso = {
    .id = QSO,
    .name = QSO_MODE,
    .is_multi = no_multi,
    .exchange_width = 77 - 55 + 1,  // full width
};

contest_config_t config_dxped = {
    .id = DXPED,
    .name = "DXPED",
    .recall_mult = true,
    .is_multi = no_multi,
    .exchange_width = 77 - 55 + 1,  // full width
};

contest_config_t config_wpx = {
    .id = WPX,
    .name = "WPX",
    .points = {
	.type = FUNCTION,
	.fn = score_wpx,
    },
    .is_multi = wpx_ismulti,
    .exchange_width = 5,    // serial nr
};

contest_config_t config_cqww = {
    .id = CQWW,
    .name = "CQWW",
    .recall_mult = true,
    .points = {
	.type = FUNCTION,
	.fn = score_cqww,
    },
    .is_multi = cqww_ismulti,
    .exchange_width = 12,    // zone nr + fixes
};

contest_config_t config_sprint = {
    .id = SPRINT,
    .name = "SPRINT",
    .points = {
	.type = FIXED,
	.point = 1,
    },
    .is_multi = no_multi,
    .exchange_width = 77 - 55 + 1,  // full width
};

contest_config_t config_arrldx_usa = {
    .id = ARRLDX_USA,
    .name = "ARRLDX_USA",
    .recall_mult = true,
    .points = {
	.type = FUNCTION,
	.fn = score_arrldx_usa,
    },
    .is_multi = arrldx_usa_ismulti,
    .exchange_width = 5,    // 2 or 3 chars
};

contest_config_t config_arrldx_dx = {
    .id = ARRLDX_DX,
    .name = "ARRLDX_DX",
    .recall_mult = true,
    .points = {
	.type = FIXED,
	.point = 3,
    },
    .is_multi = no_multi,
    .exchange_width = 5,    // 2 or 3 chars
};

contest_config_t config_arrl_ss = {
    .id = ARRL_SS,
    .name = "ARRL_SS",
    .points = {
	.type = FIXED,
	.point = 2,
    },
    .is_multi = no_multi,
    .exchange_width = 77 - 55 + 1,  // full width
};

contest_config_t config_arrl_fd = {
    .id = ARRL_FD,
    .name = "ARRL_FD",
    .recall_mult = true,
    .points = {
	.type = FUNCTION,
	.fn = score_arrlfd,
    },
    .is_multi = no_multi,
    .exchange_width = 13,
};

contest_config_t config_pacc_pa = {
    .id = PACC_PA,
    .name = "PACC_PA",
    .points = {
	.type = FIXED,
	.point = 1,
    },
    // .ismulti =
    .exchange_width = 5,    // province/serial
};

contest_config_t config_stewperry = {
    .id = STEWPERRY,
    .name = "STEWPERRY",
    .points = {
	.type = FUNCTION,
	.fn = score_stewperry
    },
    .is_multi = no_multi,
    .exchange_width = 5,    // 4 char grid square
};


/* table with pointers to all supported contests */
contest_config_t *contest_configs[] = {
    &config_qso,
    &config_dxped,
    &config_wpx,
    &config_cqww,
    &config_sprint,
    &config_arrldx_usa,
    &config_arrldx_dx,
    &config_arrl_ss,
    &config_arrl_fd,
    &config_pacc_pa,
    &config_stewperry,
    &config_focm,
};

#define NR_CONTESTS (sizeof(contest_configs)/sizeof(contest_config_t*))

/** lookup contest config by name in config table
 *
 * ignore configs where .name is not set
 */
contest_config_t *lookup_contest(char *name) {
    for (int i = 0; i < NR_CONTESTS; i++) {
	if (contest_configs[i]->name != NULL) {
	    if (strcasecmp(contest_configs[i]->name, name) == 0) {
		return contest_configs[i];
	    }
	}
    }
    return &config_unknown;
}


/** initalize contests
 *
 */
void init_contests() {
    for (int i = 0; i < NR_CONTESTS; i++) {
	contest_configs[i]->recall_serials = true;
    }
}


/** show a list of supported/hard-coded contests
 *
 * works out of ncurses context for 'tlf -l'
 */
void list_contests() {
    puts(
	"\nTLF has built-in support for the following contest identifiers:"
    );
    for (int i = 0; i < NR_CONTESTS; i++) {
	printf("\t%s\n", contest_configs[i]->name);
    }
    puts("");
}


/** setup standard configuration for contest 'name' */
void setcontest(char *name) {

    extern int zl_cty;
    extern int ja_cty;
    extern int py_cty;
    extern int ce_cty;
    extern int lu_cty;
    extern int vk_cty;
    extern int zs_cty;
    extern int ua9_cty;

    char wcall[] = "W1AW";
    char vecall[] = "VE1AA";

    char zlcall[] = "ZL1AA";
    char jacall[] = "JA1AA";
    char pycall[] = "PY1AA";
    char cecall[] = "CE1AA";
    char lucall[] = "LU1AA";
    char vkcall[] = "VK1AA";
    char zscall[] = "ZS6AA";
    char ua9call[] = "UA9AA";

    iscontest = true;
    showscore_flag = true;
    searchflg = true;
    sectn_mult = false;

    w_cty = getctynr(wcall);
    ve_cty = getctynr(vecall);

    if (whichcontest != name) {    /* avoid overlapping copy */
	strcpy(whichcontest, name);
    }

    contest = lookup_contest(name);


    if (CONTEST_IS(ARRLDX_DX)) {
	sectn_mult = true;
    }

    if (CONTEST_IS(ARRL_SS)) {
	qso_once = true;
	multlist = 1;
//      sectn_mult = true;
	leading_zeros_serial = false;
    }

    if (CONTEST_IS(PACC_PA)) {

	zl_cty = getctynr(zlcall);
	ja_cty = getctynr(jacall);
	py_cty = getctynr(pycall);
	ce_cty = getctynr(cecall);
	lu_cty = getctynr(lucall);
	vk_cty = getctynr(vkcall);
	zs_cty = getctynr(zscall);
	ua9_cty = getctynr(ua9call);
    }

    if (dx_arrlsections) {
	/* same here */
    }

    if (CONTEST_IS(QSO)) {
	iscontest = false;
	showscore_flag = false;
    }
}
