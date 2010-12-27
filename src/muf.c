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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "muf.h"
#include "tlf.h"
#include "get_time.h"

extern int use_rxvt;
extern double yt;
extern double xt;
extern double yr;
extern double xr;
extern double r;
extern int m;
extern struct tm *time_ptr;

int t = 21;
double n = 0.0;
double se, xn, xs, ff, rd, ls, h, ff, x, yn_, d, q, x, k, lm, u, a, ab, k,
    lm;

static double power(man, ex)
double man, ex;
{
    return exp(ex * log(man));

}

static void interlat()
{
    extern double q;
    extern double rd;
    extern double x;
    extern double xn;
    extern double yn_;
    extern double xt;
    extern double k;
    extern double lm;
    extern double d;
    extern double u;

    double yi;
    /* Intermediate Latitude & Longitude calculations */
    q = cos(u * rd) * cos(xt * rd) * sin(k * lm * rd);
    x = q + sin(xt * rd) * cos(k * lm * rd);
    xn = atan(x / sqrt(1 - x * x + 1e-12)) * d;
    q = cos(k * lm * rd) - sin(xt * rd) * sin(xn * rd);
    yi = (PI / 2 - atan(x / sqrt(1 - x * x + 1e-12))) * d;

    if (u < 180.0)
	yi = -yi;
    yn_ = yt + yi;
    if (yn_ > 180.0)
	yn_ -= 360.0;
    if (yn_ < -180.0)
	yn_ += 360.0;
}

static void mini_f2()
{
    extern double yn_;
    extern double xn;
    extern double rd;
    extern double x;
    extern double ff;
    extern double a;
    extern int t;
    extern double h;

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

    xh = cos(30.0 * (mh - 6.5) * rd);	/* 1 week delay on equinoxes */
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
    fo = 6.5 * x * cos(yf * rd) *
	sqrt(cos((z - sx * 5.0 + wx * 5.0) * rd));
    ex = -0.5;
    temp = cos(a * rd) * 6367.0 / (6367.0 + h);
    sf = power(1.0 - temp * temp, ex);
    ff = fo * sf;
    ff *= 1.15;

}

static void e_layer()
{
    extern double q;
    extern double xn;
    extern double rd;
    extern double xs;
    extern double x;
    extern double yn_;
    extern int t;
    extern double d;
    extern double a;
    extern double ls;

    double temp, fe, se, ex, xz;

    q = sin(xn * rd) * sin(xs * rd);
    x = q +
	cos(xn * rd) * cos(xs * rd) * cos((yn_ - 15.0 * (t - 12.0)) * rd);
    xz = (PI / 2 - atan(x / sqrt(1 - x * x + 1e-12))) * d;

    if (xz <= 85.0) {
	ex = 1.0 / 3.0;
	fe = 3.4 * (1.0 + 0.0016 * r) * power(cos(xz * rd), ex);
    } else {
	ex = -0.5;
	fe = 3.4 * (1.0 + 0.0016 * r) * power(xz - 80.0, ex);
    }

    temp = cos(a * rd);
    se = power(1.0 - (0.965 * temp * temp), ex);
//se /= 4;
//se += 1;
    ls = 0.028 * fe * fe * se;
// ls *= 15;

}

int muf(void)
{
    extern double rd;
    extern double q;
    extern double u;
    extern double a;
    extern double ab;
    extern double k;
    extern double lm;
    extern double d;
    extern double ff;
    extern double n;
    extern int t;
    extern double h;
    extern double xs;
    extern double ls;
    extern int m;
    extern char C_QTH_Lat[];
    extern char C_QTH_Long[];
    extern char C_DEST_Lat[];
    extern char C_DEST_Long[];
    extern double sunrise;
    extern double sundown;

    extern int mycountrynr;
    extern int countrynr;
    extern char datalines[MAX_DATALINES][81];
    extern char lastwwv[];

    int row;
    static double x, la, l, mf, lh;
    static long ve, ho;
    static int correct;
    int key;
    char mycountry[40];
    char country[40];
    int i;
    char time_buf[25];
    int su, sd, su_min, sd_min, iv;
    double td;
    char timediffstr[7];

    rd = PI / 180;
    d = 180 / PI;
    correct = 0;
    n = 0;

    xt = atof(C_QTH_Lat);
    yt = atof(C_QTH_Long);
    xr = atof(C_DEST_Lat);
    yr = atof(C_DEST_Long);

    get_time();
//strftime(time_buf, 60, "    %d-%b-%Y %H:%M ",  time_ptr);     ### bug fix
    strftime(time_buf, sizeof(time_buf), "    %d-%b-%Y %H:%M ", time_ptr);

    q = sin(xt * rd) * sin(xr * rd);
    x = q + cos(xt * rd) * cos(xr * rd) * cos(yt * rd - yr * rd);
    la = (PI / 2 - atan(x / sqrt(1 - x * x + 1e-12))) * d;
    l = 111.1 * la;
    q = sin(xr * rd) - sin(xt * rd) * cos(la * rd);
    x = q / cos(xt * rd) / sin(la * rd);
    u = (PI / 2 - atan(x / sqrt(1 - x * x + 1e-12))) * d;
    if (yt - yr <= 0)
	u = 360 - u;
    h = 275 + r / 2;
    xs = 23.4 * cos(30 * (m - 6.25) * rd);
    n++;
    lh = l / n;
    while (lh > 4000.0) {
	n++;
	lh = l / n;
    }

    lm = la / n;
    a = atan((cos(0.5 * lm * rd) -
	      6367.0 / (h + 6367.0)) / sin(0.5 * lm * rd)) * d;

    while (a < 1.5) {
	n++;
	lh /= n;
	while (lh > 4000.0) {
	    n++;
	    lh = l / n;
	}
	lm = la / n;
	a = atan((cos(0.5 * lm * rd) -
		  6367.0 / (h + 6367.0)) / sin(0.5 * lm * rd)) * d;
    }

    clear();
    strncpy(mycountry, datalines[mycountrynr], 25);
    mycountry[25] = '\0';
    for (i = 2; i <= 25; i++) {
	if (mycountry[i] == ':') {
	    mycountry[i] = '\0';
	    break;
	}
    }
    strncpy(country, datalines[countrynr], 25);
    country[25] = '\0';
    for (i = 5; i <= 25; i++) {
	if (country[i] == ':') {
	    country[i] = '\0';
	    break;
	}
    }


    if (use_rxvt == 0)
	attron(COLOR_PAIR(COLOR_CYAN) | A_BOLD | A_STANDOUT);
    else
	attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);

    for (i = 0; i <= 24; i++)
	mvprintw(i, 0,
		 "                                                                                ");

    mvprintw(1, 40, "%s", country);
    mvprintw(1, 0, "        SSN: %3.0f ", r);
    mvprintw(3, 40, "Dist  : %5ld KM", (long) floor(l + 0.5));

    mvprintw(4, 40, "Azim  :   %3ld degrees.", (long) floor(u + 0.5));
    mvprintw(5, 40, "F-hops:    %2.0f", n);

    sunup(xr);	/* calculate local sunup and down at destination lattitude */

    strncpy(timediffstr, datalines[countrynr] + 60, 6);	/* GMT difference */
    timediffstr[6] = '\0';
    td = atof(timediffstr);

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

    su = (int) (sunrise);
    sd = (int) (sundown);

    su_min = (int) ((sunrise - su) * 60);
    sd_min = (int) ((sundown - sd) * 60);

    mvprintw(3, 0, time_buf);
    mvprintw(7, 40, "sun   : %02d:%02d-%02d:%02d UTC", su, su_min, sd, sd_min);

    lastwwv[75] = '\0';		/* cut the bell chars */
    if ((strlen(lastwwv) >= 28) && (r != 0))
	mvprintw(10, 40, "Condx: %s", lastwwv + 26);	/* print WWV info  */

    q = 34.0;
    row = 4;
    while (q >= 2.0) {
	if ((row == 7) || (row == 10) || (row == 14) || (row == 17)) {
	    mvprintw(row, 0, "|_________________________|%2.0f", q);

	} else
	    mvprintw(row, 0, "|                         |%2.0f", q);	/* 25 spaces */
	q -= 2.0;
	row++;
    }
    mvprintw(20, 0, "---------------------------");	/* 27 dashes */
    mvprintw(21, 0, " 0 2 4 6 8 10  14  18  22 H (UTC)");
    mvprintw(4, 30, "MHz");
    refresh();
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
	mvprintw((int) ve, (int) ho, "+");
	refresh();
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

	mvprintw((int) ve, (int) ho, "-");
	refresh();
    }
    mvprintw(23, 0, " --- Press a key to continue --- ");
    refresh();

    key = getch();

    if (use_rxvt == 0)
	attron(COLOR_PAIR(COLOR_WHITE) | A_BOLD | A_STANDOUT);
    else
	attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);
    for (i = 0; i <= 24; i++)
	mvprintw(i, 0,
		 "                                                                                ");

    return (0);
}
