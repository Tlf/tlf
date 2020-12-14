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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* ------------------------------------------------------------
 *      Set contest parameters
 *
 *--------------------------------------------------------------*/


#include <stdbool.h>
#include <string.h>

#include "focm.h"
#include "getctydata.h"
#include "globalvars.h"
#include "setcontest.h"
#include "tlf.h"

/* configurations for supported contest */
contest_config_t config_unknown = {
    .id = UNKNOWN,
    .name = "Unknown"
};

contest_config_t config_qso = {
    .id = QSO,
    .name = QSO_MODE
};

contest_config_t config_dxped = {
    .id = DXPED,
    .name = "DXPED"
};

contest_config_t config_wpx = {
    .id = WPX,
    .name = "WPX"
};

contest_config_t config_cqww = {
    .id = CQWW,
    .name = "CQWW"
};

contest_config_t config_sprint = {
    .id = SPRINT,
    .name = "SPRINT"
};

contest_config_t config_arrldx_usa = {
    .id = ARRLDX_USA,
    .name = "ARRLDX_USA"
};

contest_config_t config_arrldx_dx = {
    .id = ARRLDX_DX,
    .name = "ARRLDX_DX"
};

contest_config_t config_arrl_ss = {
    .id = ARRL_SS,
    .name = "ARRL_SS"
};

contest_config_t config_arrl_fd = {
    .id = ARRL_FD,
    .name = "ARRL_FD"
};

contest_config_t config_pacc_pa = {
    .id = PACC_PA,
    .name = "PACC_PA"
};

contest_config_t config_stewperry = {
    .id = STEWPERRY,
    .name = "STEWPERRY"
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


/** show a list of supported/hard-coded contests
 *
 * works out of ncurses context for 'tlf -l' i
 */
void list_contests() {
    puts(
	"\nTLF has built-in support for the following contest identifiers:"
	);
    for(int i = 0; i < NR_CONTESTS; i++) {
	printf("\t%s\n", contest_configs[i]->name);
    }
    puts("");
}


/** setup standard configuration for contest 'name' */
void setcontest(char *name) {

    extern int dx_arrlsections;
    extern int multlist;
    extern int exchange_serial;
    extern int w_cty;
    extern int ve_cty;
    extern int zl_cty;
    extern int ja_cty;
    extern int py_cty;
    extern int ce_cty;
    extern int lu_cty;
    extern int vk_cty;
    extern int zs_cty;
    extern int ua9_cty;
    extern int showscore_flag;
    extern int searchflg;
    extern char whichcontest[];
    extern int one_point;
    extern int two_point;
    extern int three_point;
    extern bool qso_once;
    extern int sectn_mult;
    extern int recall_mult;
    extern int noleadingzeros;

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
    showscore_flag = 1;
    searchflg = 1;
    one_point = 0;
    two_point = 0;
    three_point = 0;
    recall_mult = 0;
    sectn_mult = 0;
    noleadingzeros = 0;

    w_cty = getctynr(wcall);
    ve_cty = getctynr(vecall);

    strcpy(whichcontest, name);

    contest = lookup_contest(name);


    if (CONTEST_IS(CQWW)) {
	recall_mult = 1;
    }

    if (CONTEST_IS(DXPED)) {
	recall_mult = 1;
    }

    if (CONTEST_IS(SPRINT)) {
	one_point = 1;
    }

    if (CONTEST_IS(ARRLDX_USA)) {
	recall_mult = 1;
    }

    if (CONTEST_IS(ARRLDX_DX)) {
	three_point = 1;
	recall_mult = 1;
	sectn_mult = 1;
    }

    if (CONTEST_IS(ARRL_SS)) {
	two_point = 1;
	qso_once = true;
	exchange_serial = 1;
	multlist = 1;
//      sectn_mult = 1;
	noleadingzeros = 1;
    }

    if (CONTEST_IS(ARRL_FD)) {
	recall_mult = 1;
    }

    if (CONTEST_IS(PACC_PA)) {
	one_point = 1;

	zl_cty = getctynr(zlcall);
	ja_cty = getctynr(jacall);
	py_cty = getctynr(pycall);
	ce_cty = getctynr(cecall);
	lu_cty = getctynr(lucall);
	vk_cty = getctynr(vkcall);
	zs_cty = getctynr(zscall);
	ua9_cty = getctynr(ua9call);
    }

    if (dx_arrlsections == 1) {
	/* same here */
    }

    if (CONTEST_IS(QSO)) {
	iscontest = false;
	showscore_flag = 0;
    }
}
