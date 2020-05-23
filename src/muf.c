/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *                    2010-2011 Thomas Beierlein <tb@forth-ev.de>
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

#include <glib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "dxcc.h"
#include "get_time.h"
#include "globalvars.h"
#include "getwwv.h"
#include "sunup.h"
#include "qrb.h"
#include "tlf_panel.h"
#include "ui_utils.h"


// message splitters:
// line[0] - original line, content can be modified in-place
// line[1],line[2] - pointers to subsequent lines within line[0], initially NULL

static void split_condx(const int len, char **line) {
    if (len < 40) {
	return;
    }
    char *p = line[0] + 39;
    while (*p != ' ' && p > line[0]) {    // find a space before index 39
	--p;
    }
    if (p > line[0]) {
	*p = 0;
	line[1] = p + 1;
    } else {    // no space found, truncate (should not happen)
	line[0][40] = 0;
    }
}


/*
WCY de DK0WCY-1 <12> : K=2 expK=0 A=19 R=11 SFI=74 SA=qui GMF=qui Au=no
                      ^                           ^

WCY de DK0WCY-1 <12> :
K=2 expK=0 A=19 R=11 SFI=74
SA=qui GMF=qui Au=no
*/
static void split_wcy(const int len, char **line) {

    char *p = strchr(line[0], ':'); // first colon
    if (p != NULL && p + 2 < line[0] + len)  {
	p[1] = 0;
	line[1] = p + 2;
	p = strstr(line[1], "SFI");
	if (p != NULL) {
	    p = strchr(p, ' ');     // the space after SFI
	}
	if (p != NULL) {
	    p[0] = 0;
	    line[2] = p + 1;
	}
    }
}

/*
WWV de VE7CC <21>:   SFI=74, A=9, K=1, No Storms -> No Storms
                    ^                 ^

WWV de VE7CC <21>:
SFI=74, A=9, K=1,
No Storms -> No Storms
*/
static void split_wwv(const int len, char **line) {
    char *p = strstr(line[0], "SFI");
    if (p != NULL && p - 1 >= line[0])  {
	p[-1] = 0;  // the space before SFI
	line[1] = p;
	p = strstr(line[1], "K=");
	if (p != NULL) {
	    p = strchr(p, ' '); // the space after K
	}
	if (p != NULL) {
	    p[0] = 0;
	    line[2] = p + 1;
	}
    }
}

static void show_condx(WINDOW *win) {
    if (lastwwv[0] == 0) {
	mvwaddstr(win, 10, 40, "Condx: n/a");  // no data
	return;
    }

    static GRegex *squash_regex = NULL;
    if (squash_regex == NULL) {
	squash_regex = g_regex_new(" {2,}", 0, 0, NULL);
    }

    // squash spaces and show condx
    gchar *condx = g_regex_replace_literal(squash_regex, lastwwv, -1, 0, "  ", 0,
					   NULL);
    char *line[3] = {condx, NULL, NULL};
    split_condx(strlen(condx), line);

    mvwaddstr(win, 10, 40, line[0]);
    if (line[1] != NULL) {
	mvwaddstr(win, 11, 40, line[1]);
    }

    g_free(condx);

    // show original WWV message, split into up to 3 lines if needed
    line[0] = g_strdup(lastwwv_raw);
    line[1] = line[2] = NULL;
    const int len = strlen(lastwwv_raw);
    if (strncmp(lastwwv_raw, "WCY", 3) == 0) {
	split_wcy(len, line);
    } else {
	split_wwv(len, line);
    }

    for (int i = 0; i < 3; ++i) {
	if (line[i] != NULL) {
	    mvwaddstr(win, 14 + i, 40, line[i]);
	}
    }

    g_free(line[0]);
}

/*********************************************************************
 Code below is based on MICROMUF.PAS

http://www.classiccmp.org/cpmarchives/cpm/Software/WalnutCD/cpm/hamradio/micromuf.pas

 Original attribution:
---------------------------------------------------------------------
  This program uses 'MINI-F2' devised by R. Fricker (BBC external
  services) for FO-F calculations and L.M. Muggleton's formula for
  FO-E calculations.


  For the L.U.F. a minimum useable fieldstrength of 30 DBUV at the
  receiver and 250 KW of transmitter power (aerial gain: 18 DBI) are
  assumed.  The L.U.F. is derived from absorption calculations based
  on the work of Piggot, George, Samuel, and Bradley.  In spite of the
  program's simplicity it gives a good impression of the ionosphere's
  behaviour and can be used for propagation predictions.

            Hans Bakhuizen
            Propagation Unit; Frequency Bureau
            Radio Netherlands
            P.O. Box 222
            1200 JG Hilversum Holland

(C) Copyright Media Network June 1984

Translation from Basic into TURBO Pascal by Jonathan D Ogden on
September 26, 1986.

       Jonathan D Ogden
       402 e Daniel
       Champaign, Il 61820 USA
---------------------------------------------------------------------
*********************************************************************/

int month;


double yt;
double xt;
double yr;
double xr;

int t;

double xn, xs, ls, h, ff, x, yn_, k, lm, u, a;


static double power(man, ex)
double man, ex;
{
    return exp(ex * log(man));
}

static void interlat() {
    double yi, q;
    /* Intermediate Latitude & Longitude calculations */
    q = cos(u / RADIAN) * cos(xt / RADIAN) * sin(k * lm / RADIAN);
    x = q + sin(xt / RADIAN) * cos(k * lm / RADIAN);
    xn = atan(x / sqrt(1 - x * x + 1e-12)) * RADIAN;
    q = cos(k * lm / RADIAN) - sin(xt / RADIAN) * sin(xn / RADIAN);
    yi = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;

    if (u < 180.0)
	yi = -yi;
    yn_ = yt + yi;
    if (yn_ > 180.0)
	yn_ -= 360.0;
    if (yn_ < -180.0)
	yn_ += 360.0;
}

static void mini_f2() {
    double temp, tl, yf, ex, yz, yg, zo, z, mh, xh, wx, sx, ty, fo, sf;

    yz = yn_;
    if (yn_ < -160.0)
	yz = yn_ + 360.0;
    yg = (20.0 - yz) / 50;
    temp = 1 - (yg / 7.0);
    zo = 20.0 * yg / (1 + yg + yg * yg) + 5.0 * (temp * temp);
    z = xn - zo;
    tl = t - yn_ / 15.0;
    if (tl > 24.0)
	tl -= 24.0;
    if (tl < 0.0)
	tl += 24.0;

    mh = month;
    if (z <= 0.0) {
	z = -z;
	mh += 6;
    }

    xh = cos(30.0 * (mh - 6.5) / RADIAN);	/* 1 week delay on equinoxes */
    sx = (fabs(xh) + xh) / 2.0;	/* F-layer local summer variance */
    wx = (fabs(xh) - xh) / 2.0;	/* F-layer local winter variance */

    if (z > 77.5)
	z = 77.5;
    ty = tl;
    if (ty < 5.0)
	ty = tl + 24.0;
    yf = (ty - 14.0 - sx * 2.0 + wx * 2.0 - ssn_r / 175.0) *
	 (7.0 - sx * 3.0 + wx * 4.0 - ssn_r / (150.0 - wx * 75.0));

    if (fabs(yf) > 60.0)
	yf = 60.0;
    x = 1 + ssn_r / (175.0 + sx * 175.0);
    fo = 6.5 * x * cos(yf / RADIAN) *
	 sqrt(cos((z - sx * 5.0 + wx * 5.0) / RADIAN));
    ex = -0.5;
    temp = cos(a / RADIAN) * 6367.0 / (6367.0 + h);
    sf = power(1.0 - temp * temp, ex);
    ff = fo * sf;
    ff *= 1.15;

}

static void e_layer() {
    double temp, fe, se, ex, xz, q;

    q = sin(xn / RADIAN) * sin(xs / RADIAN);
    x = q +
	cos(xn / RADIAN) * cos(xs / RADIAN) * cos((yn_ - 15.0 * (t - 12.0)) / RADIAN);
    xz = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;

    if (xz <= 85.0) {
	ex = 1.0 / 3.0;
	fe = 3.4 * (1.0 + 0.0016 * ssn_r) * power(cos(xz / RADIAN), ex);
    } else {
	ex = -0.5;
	fe = 3.4 * (1.0 + 0.0016 * ssn_r) * power(xz - 80.0, ex);
    }

    temp = cos(a / RADIAN);
    se = power(1.0 - (0.965 * temp * temp), ex);
//se /= 4;
//se += 1;
    ls = 0.028 * fe * fe * se;
// ls *= 15;

}

void muf(void) {
    extern double QTH_Lat;
    extern double QTH_Long;
    extern double DEST_Lat;
    extern double DEST_Long;
    double sunrise;
    double sundown;

    int row;
    double la, l, mf, lh;
    dxcc_data *dx;
    char country[40];
    int i;
    char time_buf[25];
    int su, sd, su_min, sd_min;
    double ab;
    double n;
    double td;
    double q;

    n = 0.0;

    xt = QTH_Lat;
    yt = QTH_Long;
    xr = DEST_Lat;
    yr = DEST_Long;

    dx = dxcc_by_index(countrynr);
    strncpy(country, dx->countryname, 25);

    time_t now = format_time(time_buf, sizeof(time_buf),
			     "    " DATE_TIME_FORMAT " ");

    struct tm time_tm;
    gmtime_r(&now, &time_tm);
    month = time_tm.tm_mon;

    q = sin(xt / RADIAN) * sin(xr / RADIAN);
    x = q + cos(xt / RADIAN) * cos(xr / RADIAN) * cos(yt / RADIAN - yr / RADIAN);
    la = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;
    l = ARC_IN_KM * la;
    q = sin(xr / RADIAN) - sin(xt / RADIAN) * cos(la / RADIAN);
    x = q / cos(xt / RADIAN) / sin(la / RADIAN);
    u = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;
    if (yt - yr <= 0)
	u = 360 - u;
    h = 275 + ssn_r / 2;
    xs = 23.4 * cos(30 * (month - 6.25) / RADIAN);
    n++;
    lh = l / n;
    while (lh > 4000.0) {
	n++;
	lh = l / n;
    }

    lm = la / n;
    a = atan((cos(0.5 * lm / RADIAN) -
	      6367.0 / (h + 6367.0)) / sin(0.5 * lm / RADIAN)) * RADIAN;

    while (a < 1.5) {
	n++;
	lh /= n;
	while (lh > 4000.0) {
	    n++;
	    lh = l / n;
	}
	lm = la / n;
	a = atan((cos(0.5 * lm / RADIAN) -
		  6367.0 / (h + 6367.0)) / sin(0.5 * lm / RADIAN)) * RADIAN;
    }

    WINDOW *win = newwin(LINES, 80, 0, 0);
    PANEL *pan = new_panel(win);

    wclear(win);
    wattron(win, modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (i = 0; i < LINES; i++)
	mvwprintw(win, i, 0, backgrnd_str);

    mvwprintw(win, 1, 0, "        SSN: %3.0f ", ssn_r);

    if (countrynr != 0) {
	mvwprintw(win, 1, 40, "%s", country);
	mvwprintw(win, 3, 40, "Dist  : %5.0f km", l);
	mvwprintw(win, 4, 40, "Azim  :   %3.0f degrees", u);
	mvwprintw(win, 5, 40, "F-hops:    %2.0f", n);

	sunup(xr, &sunrise, &sundown);	/* calculate local sunup and down
                                               at destination lattitude */
	/* transform to UTC based on longitude from country description */
	td = (yr * 4) / 60 ; 	/* 4 degree/min */
	sunrise += td;
	sundown += td;

	if (sunrise >= 24.0)
	    sunrise -= 24.0;
	else if (sunrise <= 0.0)
	    sunrise += 24.0;

	if (sundown >= 24.0)
	    sundown -= 24.0;
	else if (sundown <= 0.0)
	    sundown += 24.0;

	su = (int)(sunrise);
	sd = (int)(sundown);

	su_min = (int)((sunrise - su) * 60);
	sd_min = (int)((sundown - sd) * 60);

	mvwprintw(win, 3, 0, time_buf);
	mvwprintw(win, 7, 40, "Sun   : %02d:%02d-%02d:%02d UTC", su, su_min, sd,
		  sd_min);
    }


    show_condx(win);

    // show frequency chart
    q = 34.0;
    row = 4;
    while (q >= 2.0) {
	if ((row == 7) || (row == 10) || (row == 14) || (row == 17)) {
	    mvwprintw(win, row, 0, "|_________________________|%2.0f", q);

	} else
	    mvwprintw(win, row, 0, "|                         |%2.0f", q);	/* 25 spaces */
	q -= 2.0;
	row++;
    }
    mvwprintw(win, 20, 0, "---------------------------");	/* 27 dashes */
    mvwprintw(win, 21, 0, " 0 2 4 6 8 10  14  18  22 H (UTC)");
    mvwprintw(win, 4, 30, "MHz");

    if (countrynr != 0) {
	for (t = 1; t <= 24; t++) {
	    ab = 0.0;
	    k = 0.5;
	    interlat();
	    mini_f2();
	    mf = ff;

	    k = n - 0.5;
	    interlat();
	    mini_f2();

	    if (ff < mf)
		mf = ff;
	    double ve = 21 - (long) floor(mf / 2.0 + 0.5);

	    double ho = t + 1;
	    if (ve < 4)
		ve = 4;
	    mvwprintw(win, (int) ve, (int) ho, "+");

	    while (k <= n - 0.25) {
		interlat();
		e_layer();
		ab += ls;
		k += 0.5;
	    }
	    ve = 20 - (long) floor(ab + 0.5);
	    if (ve < 4)
		ve = 4;
	    if (ve > 20)
		ve = 20;

	    mvwprintw(win, (int) ve, (int) ho, "-");
	}
    }

    mvwprintw(win, 23, 0, " --- Press a key to continue --- ");
    refreshp();
    key_get();

    hide_panel(pan);
    del_panel(pan);
    delwin(win);
}
