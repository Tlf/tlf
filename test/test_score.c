#include "test.h"

#include "../src/dxcc.h"
#include "../src/getctydata.h"
#include "../src/score.h"
#include "../src/setcontest.h"
#include "../src/tlf.h"

#include "../src/globalvars.h"

// OBJECT ../src/addpfx.o
// OBJECT ../src/score.o
// OBJECT ../src/addmult.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/focm.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/plugin.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/qrb.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/utils.o

void checkexchange(struct qso_t *qso, bool interactive) {}

char *calc_continent(int zone);

struct qso_t qso = { };

#define check_points(point) \
    do{ assert_int_equal(score(&qso), point); }while(0)

#define check_call_points(thecall,point) \
    do{ qso.call = g_strdup(thecall); \
	assert_int_equal(score(&qso), point); \
	g_free(qso.call); \
	qso.call = NULL; }while(0)

void clear_display() {}

#if 0
void __wrap_qrb(char *x, char *y, char *u, char *v, double *a, double *b) {
}

int __wrap_foc_score() {
    return 0;
}
#endif


int setup_default(void **state) {

    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    current_qso.call = g_malloc0(CALL_SIZE);

    strcpy(my.qra, "jo60lx");
    strcpy(my.continent, "EU");
    my.countrynr = getctynr("DL");

    qso.mode = CWMODE;

    setcontest("qso");

    pfxmult = false;
    dupe = 0;

    my_country_points = -1;
    my_cont_points = -1;
    dx_cont_points = -1;

    countrylist_only = false;
    countrylist_points = -1;
    strcpy(countrylist[0], "");

    continentlist_only = false;
    continentlist_points = -1;
    strcpy(continent_multiplier_list[0], "");

    lowband_point_mult = false;
    portable_x2 = false;

    ssbpoints = 0;
    cwpoints = 0;

    return 0;
}

void test_dupe(void **state) {
    dupe = 1;
    check_points(0);
}

int teardown_default(void **state) {
    return 0;
}

void check_score_to_continent(int i, char *cont) {
    strcpy(continent, "abc");
    assert_string_equal(calc_continent(i), cont);
    assert_string_equal(continent, "abc"); /* do not touch 'continent' */
}

void test_calc_continent(void **state) {
    check_score_to_continent(0, "??");
    for (int i = 1; i < 9; i++) {
	check_score_to_continent(i, "NA");
    }
    for (int i = 9; i < 14; i++) {
	check_score_to_continent(i, "SA");
    }
    for (int i = 14; i < 17; i++) {
	check_score_to_continent(i, "EU");
    }
    for (int i = 17; i < 33; i++) {
	check_score_to_continent(i, "AS");
    }
    for (int i = 33; i < 40; i++) {
	check_score_to_continent(i, "AF");
    }
    check_score_to_continent(40, "EU");
    check_score_to_continent(41, "??");
}


void test_wpx(void **state) {
    setcontest("wpx");
    pfxmult = false;

    /* same country */
    check_call_points("DL3ABC", 1);

    /* different continents */
    qso.bandindex = BANDINDEX_20;
    check_call_points("ZS6ABC", 3);

    qso.bandindex = BANDINDEX_40;
    check_call_points("ZS6ABC", 6);

    /* same continent, not NA */
    qso.bandindex = BANDINDEX_20;
    check_call_points("HB9ABC", 1);

    qso.bandindex = BANDINDEX_40;
    check_call_points("HB9ABC", 2);

    /* same continent, NA */
    strcpy(my.continent, "NA");
    qso.bandindex = BANDINDEX_20;
    check_call_points("VE3ABC", 2);

    qso.bandindex = BANDINDEX_40;
    check_call_points("VE3ABC", 4);

}


void test_cqww(void **state) {
    setcontest("cqww");

    check_call_points("DL3ABC", 0);

    strcpy(my.continent, "EU");
    check_call_points("HB9ABC", 1);

    strcpy(my.continent, "NA");
    check_call_points("XE1ABC", 2);

    strcpy(my.continent, "NA");
    check_call_points("PY2ABC", 3);

    qso.comment = "19";		    /* CQ Zone 19 => AS */
    check_call_points("HB9ABC/mm", 3);
}

void test_arrl_fd(void **state) {
    setcontest("arrl_fd");

    qso.mode = CWMODE;
    check_points(2);

    qso.mode = SSBMODE;
    check_points(1);

}


void test_simple_points(void **state) {
    setcontest("pointtest");

    contest->points.type = FIXED;
    contest->points.point = 1;
    check_points(1);

    contest->points.point = 2;
    check_points(2);

    contest->points.point = 3;
    check_points(3);
}

void test_arrldx_usa(void **state) {
    setcontest("arrldx_usa");

    check_call_points("W3ABC", 0);

    check_call_points("VE2ABC", 0);

    check_call_points("DL3ABC", 3);
}

void test_ssbcw(void **state) {
    check_points(0);

    ssbpoints = 3;
    check_points(0);

    cwpoints = 4;
    qso.mode = CWMODE;
    check_points(4);
    qso.mode = SSBMODE;
    check_points(3);

    lowband_point_mult = true;
    qso.bandindex = BANDINDEX_30;
    check_points(3);
    qso.bandindex = BANDINDEX_40;
    check_points(6);

    portable_x2 = true;
    check_call_points("DL3XYZ", 6);
    check_call_points("DL3XYZ/P", 12);
    portable_x2 = false;

    qso.mode = DIGIMODE;
    check_points(0);
    ssbpoints = 0;
    check_points(0);
}

static void init_countrylist() {
    strcpy(countrylist[0], "OE");
    strcpy(countrylist[1], "DL");
    strcpy(countrylist[2], "W");
    strcpy(countrylist[3], "");
}


/* test is_in_countrylist() */
void test_in_countrylist(void **state) {
    init_countrylist();
    assert_int_equal(is_in_countrylist(getctynr("DL")), true);
}

void test_not_in_countrylist(void **state) {
    init_countrylist();
    assert_int_equal(is_in_countrylist(getctynr("CE")), false);
}

void test_in_countrylist_keeps_countrynr(void **state) {
    init_countrylist();
    countrynr = 42;
    assert_int_equal(is_in_countrylist(getctynr("DL")), true);
    assert_int_equal(is_in_countrylist(getctynr("CE")), false);
    assert_int_equal(countrynr, 42);
}


void test_country_found(void **state) {
    /* nothing to find in empty list */
    strcpy(current_qso.call, "LZ1AB");
    assert_int_equal(country_found(""), 0);
    strcpy(current_qso.call, "DL3XYZ");
    assert_int_equal(country_found(""), 0);

    init_countrylist();
    strcpy(current_qso.call, "LZ1AB");
    assert_int_equal(country_found(""), 0);
    strcpy(current_qso.call, "DL3XYZ");
    assert_int_equal(country_found(""), 1);
    strcpy(current_qso.call, "K3LA");
    assert_int_equal(country_found(""), 1);
}


static void init_continentlist() {
    strcpy(continent_multiplier_list[0], "EU");
    strcpy(continent_multiplier_list[1], "NA");
    strcpy(continent_multiplier_list[2], "");
}

void test_empty_continentlist(void **state) {
    assert_int_equal(is_in_continentlist(""), false);
    assert_int_equal(is_in_continentlist("NA"), false);
}

void test_not_in_continentlist(void **state) {
    init_continentlist();
    assert_int_equal(is_in_continentlist("SA"), false);
}

void test_in_continentlist(void **state) {
    init_continentlist();
    assert_int_equal(is_in_continentlist("NA"), true);
}


void test_scoreByCorC_countrylistOnly(void **state) {
    countrylist_only = true;
    check_call_points("LZ1AB", 0);
    check_call_points("DL3XYZ", 0);

    init_countrylist();
    check_call_points("LZ1AB", 0);
    check_call_points("DL3XYZ", 0);

    countrylist_points = 4;
    check_call_points("LZ1AB", 0);
    check_call_points("DL3XYZ", 4);
}

void test_scoreByCorC_continentlistOnly(void **state) {
    continentlist_only = true;
    /* empty list */
    check_call_points("LZ1AB", 0);

    init_continentlist();

    /* no list points given */
    check_call_points("LZ1AB", 0);
    check_call_points("JA4BB", 0);

    /* same, but my_cont_points set */
    my_cont_points = 1;
    check_call_points("LZ1AB", 1);
    check_call_points("XE2AAA", 0);
    check_call_points("JA4BB", 0);

    /* my_cont_points, cont_list_points given */
    continentlist_points = 2;
    check_call_points("LZ1AB", 1);
    check_call_points("XE2AAA", 2);
    check_call_points("JA4BB", 0);

    /* only cont_list_points given */
    my_cont_points = -1;
    check_call_points("LZ1AB", 2);
    check_call_points("XE2AAA", 2);
    check_call_points("JA4BB", 0);
}


void test_scoreByCorC_continentlistOnly_ignoreDxContPoints(void **state) {
    continentlist_only = true;
    check_call_points("LZ1AB", 0);

    dx_cont_points = 2;
    init_continentlist();
    check_call_points("LZ1AB", 0);
    check_call_points("XE2AAA", 0);
}


void test_scoreByCorC_notInList(void **state) {

    /* my_country/cont_points and dx_cont_points not set */
    check_call_points("DL3XYZ", 0);
    check_call_points("LZ1AB", 0);
    check_call_points("K3XX", 0);

    dx_cont_points = 4;
    check_call_points("DL3XYZ", 0);
    check_call_points("LZ1AB", 0);
    check_call_points("K3XX", 4);

    my_cont_points = 3;
    check_call_points("DL3XYZ", 3);
    check_call_points("LZ1AB", 3);
    check_call_points("K3XX", 4);

    my_country_points = 1;
    check_call_points("DL3XYZ", 1);
    check_call_points("LZ1AB", 3);
    check_call_points("K3XX", 4);
}


void test_scoreByCorC_InList(void **state) {
    init_countrylist();

    /* countrylist_points, my_country_points and my_cont_points
     * not set -> 0 points for all */
    check_call_points("OE2BL", 0);
    check_call_points("DL3XYZ", 0);
    check_call_points("K3XX", 0);

    /* only countrylist_points set -> use my_country/cont_points for
     * my own country , otherwise 0 ??? */
    countrylist_points = 3;
    check_call_points("OE2BL", 3);
    check_call_points("DL3XYZ", 0);
    check_call_points("K3XX", 3);

    my_cont_points = 2;
    check_call_points("OE2BL", 3);
    check_call_points("DL3XYZ", 2);
    check_call_points("K3XX", 3);

    my_country_points = 1;
    check_call_points("OE2BL", 3);
    check_call_points("DL3XYZ", 1);
    check_call_points("K3XX", 3);
}

