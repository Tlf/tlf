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

#include "setcontest.h"
#include "focm.h"

int setcontest(void)
{

    extern int focm;
    extern int wpx;
    extern int pfxmult;
    extern int cqww;
    extern int dxped;
    extern int sprint;
    extern int arrldx_usa;
    extern int dx_arrlsections;
    extern int arrl_fd;
    extern int arrlss;
    extern int multlist;
    extern int pacc_pa_flg;
    extern int stewperry_flg;
    extern int universal;
    extern int other_flg;
    extern int exchange_serial;
    extern int wysiwyg_multi;
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
    extern int contest;
    extern int showscore_flag;
    extern int searchflg;
    extern char whichcontest[];
    extern int one_point;
    extern int two_point;
    extern int three_point;
    extern int qso_once;
    extern int sectn_mult;
    extern int recall_mult;
    extern int noleadingzeros;
    extern int shortqsonr;

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

    if (pfxmult == 0 && wpx == 1)
	wpx = 0;
    cqww = 0;
    dxped = 0;
    sprint = 0;
    arrldx_usa = 0;
    pacc_pa_flg = 0;
    focm = 0;
    universal = 0;
    contest = 0;
    showscore_flag = 0;
    searchflg = 0;
    one_point = 0;
    two_point = 0;
    three_point = 0;
    recall_mult = 0;
    sectn_mult = 0;
    noleadingzeros = 0;

    if (strcmp(whichcontest, "wpx") == 0) {
	wpx = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (strcmp(whichcontest, "cqww") == 0) {
	cqww = 1;
	recall_mult = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
	w_cty = getctydata(wcall);
	ve_cty = getctydata(vecall);
    }

    if (strcmp(whichcontest, "dxped") == 0) {
	dxped = 1;
	recall_mult = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (strcmp(whichcontest, "sprint") == 0) {
	sprint = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
	one_point = 1;
    }

    if (strcmp(whichcontest, "arrldx_usa") == 0) {
	arrldx_usa = 1;
	recall_mult = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (strcmp(whichcontest, "arrldx_dx") == 0) {
//      other_flg = 1;
	three_point = 1;
	recall_mult = 1;
	sectn_mult = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }
    if (strcmp(whichcontest, "arrl_ss") == 0) {
	arrlss = 1;
	other_flg = 1;
	two_point = 1;
	qso_once = 1;
	exchange_serial = 1;
	multlist = 1;
	recall_mult = 0;
//      sectn_mult = 1;
	noleadingzeros = 1;
	shortqsonr = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (strcmp(whichcontest, "arrl_fd") == 0) {
//      other_flg = 1;
	recall_mult = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
	arrl_fd = 1;
    }

    if (strcmp(whichcontest, "pacc_pa") == 0) {
	pacc_pa_flg = 1;
	one_point = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;

	ve_cty = getctydata(vecall);
	w_cty = getctydata(wcall);
	zl_cty = getctydata(zlcall);
	ja_cty = getctydata(jacall);
	py_cty = getctydata(pycall);
	ce_cty = getctydata(cecall);
	lu_cty = getctydata(lucall);
	vk_cty = getctydata(vkcall);
	zs_cty = getctydata(zscall);
	ua9_cty = getctydata(ua9call);
    }

    if (strcmp(whichcontest, "focmarathon") == 0) {
	foc_init();
    }

    if (strcmp(whichcontest, "other") == 0) {
	other_flg = 1;
	one_point = 1;
	recall_mult = 1;
	wysiwyg_multi = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (strcmp(whichcontest, "universal") == 0) {
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (dx_arrlsections == 1) {
	contest = 1;
//      universal = 1;
	showscore_flag = 1;
	searchflg = 1;
	w_cty = getctydata(wcall);
	ve_cty = getctydata(vecall);
    }

    if (strcmp(whichcontest, "stewperry") == 0) {
//      other_flg = 1;
	stewperry_flg = 1;
	contest = 1;
	showscore_flag = 1;
	searchflg = 1;
    }

    if (strcmp(whichcontest, "qso") == 0) {
	contest = 0;
	searchflg = 1;
    } else {
	searchflg = 1;		//dxpedition
	contest = 1;
	showscore_flag = 1;
	universal = 1;
    }

    return (0);
}
