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


#include <math.h>
#include <string.h>
#include <time.h>

#include "dxcc.h"
#include "get_time.h"
#include "sunup.h"
#include "tlf.h"
#include "tlf_panel.h"
#include "ui_utils.h"

#define RADIAN		(180.0 / M_PI)


extern double r;
extern int m;
extern struct tm *time_ptr;

double yt;
double xt;
double yr;
double xr;

int t = 21;

double xn, xs, ls, h, ff, x, yn_, q, k, lm, u, a;
//FIXME: q should be local variable

static double power(man, ex)
double man, ex;
{
    return exp(ex * log(man));
}

static void interlat() {
    double yi;
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

    mh = m;
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
    yf = (ty - 14.0 - sx * 2.0 + wx * 2.0 - r / 175.0) *
	 (7.0 - sx * 3.0 + wx * 4.0 - r / (150.0 - wx * 75.0));

    if (fabs(yf) > 60.0)
	yf = 60.0;
    x = 1 + r / (175.0 + sx * 175.0);
    fo = 6.5 * x * cos(yf / RADIAN) *
	 sqrt(cos((z - sx * 5.0 + wx * 5.0) / RADIAN));
    ex = -0.5;
    temp = cos(a / RADIAN) * 6367.0 / (6367.0 + h);
    sf = power(1.0 - temp * temp, ex);
    ff = fo * sf;
    ff *= 1.15;

}

static void e_layer() {
    double temp, fe, se, ex, xz;

    q = sin(xn / RADIAN) * sin(xs / RADIAN);
    x = q +
	cos(xn / RADIAN) * cos(xs / RADIAN) * cos((yn_ - 15.0 * (t - 12.0)) / RADIAN);
    xz = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;

    if (xz <= 85.0) {
	ex = 1.0 / 3.0;
	fe = 3.4 * (1.0 + 0.0016 * r) * power(cos(xz / RADIAN), ex);
    } else {
	ex = -0.5;
	fe = 3.4 * (1.0 + 0.0016 * r) * power(xz - 80.0, ex);
    }

    temp = cos(a / RADIAN);
    se = power(1.0 - (0.965 * temp * temp), ex);
//se /= 4;
//se += 1;
    ls = 0.028 * fe * fe * se;
// ls *= 15;

}

int muf(void) {
    extern double QTH_Lat;
    extern double QTH_Long;
    extern double DEST_Lat;
    extern double DEST_Long;
    double sunrise;
    double sundown;

    extern int mycountrynr;
    extern int countrynr;
    extern char lastwwv[];

    dxcc_data *dx;
    int row;
    static double la, l, mf, lh;
    static long ve, ho;
    static int correct;
    char mycountry[40];
    char country[40];
    int i;
    char time_buf[25];
    int su, sd, su_min, sd_min, iv;
    double ab;
    double n;
    double td;

    PANEL *pan;
    WINDOW *win;

    win = newwin(25, 80, 0, 0);
    pan = new_panel(win);

    correct = 0;
    n = 0.0;

    xt = QTH_Lat;
    yt = QTH_Long;
    xr = DEST_Lat;
    yr = DEST_Long;

    get_time();
//strftime(time_buf, 60, "    %d-%b-%Y %H:%M ",  time_ptr);     ### bug fix
    strftime(time_buf, sizeof(time_buf), "    %d-%b-%Y %H:%M ", time_ptr);

    q = sin(xt / RADIAN) * sin(xr / RADIAN);
    x = q + cos(xt / RADIAN) * cos(xr / RADIAN) * cos(yt / RADIAN - yr / RADIAN);
    la = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;
    l = 111.1 * la;
    q = sin(xr / RADIAN) - sin(xt / RADIAN) * cos(la / RADIAN);
    x = q / cos(xt / RADIAN) / sin(la / RADIAN);
    u = (M_PI_2 - atan(x / sqrt(1 - x * x + 1e-12))) * RADIAN;
    if (yt - yr <= 0)
	u = 360 - u;
    h = 275 + r / 2;
    xs = 23.4 * cos(30 * (m - 6.25) / RADIAN);
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

    dx = dxcc_by_index(mycountrynr);
    strncpy(mycountry, dx->countryname, 25);

    dx = dxcc_by_index(countrynr);
    strncpy(country, dx->countryname, 25);

    wclear(win);
    wattron(win, modify_attr(COLOR_PAIR(C_WINDOW) | A_STANDOUT));

    for (i = 0; i < 25; i++)
	mvwprintw(win, i, 0,
		  "                                                                                ");

    mvwprintw(win, 1, 40, "%s", country);
    mvwprintw(win, 1, 0, "        SSN: %3.0f ", r);
    mvwprintw(win, 3, 40, "Dist  : %5ld KM", (long) floor(l + 0.5));

    mvwprintw(win, 4, 40, "Azim  :   %3ld degrees.", (long) floor(u + 0.5));
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
    mvwprintw(win, 7, 40, "sun   : %02d:%02d-%02d:%02d UTC", su, su_min, sd,
	      sd_min);

    lastwwv[75] = '\0';		/* cut the bell chars */
    if ((strlen(lastwwv) >= 28) && (r != 0))
	mvwprintw(win, 10, 40, "Condx: %s", lastwwv + 26);	/* print WWV info  */

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
    refreshp();
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
	iv = (int) floor(mf / 2.0 + 0.5);
	ve = 21 - (long) floor(mf / 2.0 + 0.5);

	ho = t + 1;
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
    mvwprintw(win, 23, 0, " --- Press a key to continue --- ");
    refreshp();

    (void)key_get();

    hide_panel(pan);
    del_panel(pan);
    delwin(win);

    return (0);
}
