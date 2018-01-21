#include "test.h"

#include "../src/score.h"
#include "../src/tlf.h"
#include "../src/dxcc.h"

#include "../src/globalvars.h"

// OBJECT ../src/score.o
// OBJECT ../src/addmult.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/focm.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/qrb.o
// OBJECT ../src/locator2longlat.o
// OBJECT ../src/getpx.o

// ===========
// these are missing from globalvars
extern int dupe;
extern int cwpoints;
extern int ssbpoints;
extern int mycountrynr;
extern int my_country_points;
extern int my_cont_points;
extern int dx_cont_points;
extern int countrylist_only;
extern int countrylist_points;
extern char countrylist[][6];
extern int continentlist_only;
extern int continentlist_points;
extern char mycontinent[];
extern char continent_multiplier_list[7][3];
extern int lowband_point_mult;
extern int portable_x2;
// ===========

#define check_points(point) \
    do{ assert_int_equal(score(), point); }while(0)

#define check_call_points(call,point) \
    do{ strcpy(hiscall, call); \
	assert_int_equal(score(), point); }while(0)

#if 0
void __wrap_qrb(char *x, char *y, char *u, char *v, double *a, double *b) {
}

int __wrap_foc_score() {
    return 0;
}
#endif

int setup(void **state) {

    strcpy(myqra, "jo60lx");

    strcpy(mycontinent, "EU");

    mycountrynr = 95;   /* DL */
    w_cty = 184;        /* W */
    ve_cty = 283;       /* VE */

    trxmode = CWMODE;

    return 0;
}

int setup_default(void **state) {

    cqww = 0;
    wpx = 0;
    pfxmult = 0;
    arrl_fd = 0;
    dupe = 0;
    arrldx_usa = 0;
    one_point = 0;
    two_point = 0;
    three_point = 0;

    my_country_points = -1;
    my_cont_points = -1;
    dx_cont_points = -1;

    countrylist_only = 0;
    countrylist_points = -1;
    strcpy(countrylist[0], "");

    continentlist_only = 0;
    continentlist_points = -1;
    strcpy(continent_multiplier_list[0], "");

    strcpy(mycontinent, "EU");

    lowband_point_mult = 0;
    portable_x2 = 0;

    return 0;
}

int setup_ssbcw(void **state) {
    char filename[100];

    setup_default(state);
    /* TODO */
    /* load_ctydata needs means to destroy the database */
    strcpy(filename, TOP_SRCDIR);
    strcat(filename, "/share/cty.dat");
    assert_int_equal(load_ctydata(filename), 0);

    return 0;
}


void test_dupe(void **state) {
    dupe = 1;
    check_points(0);
}

int teardown_default(void **state) {
    return 0;
}


void test_wpx(void **state) {
    wpx = 1;
    pfxmult = 0;

    /* same country */
    countrynr = mycountrynr;
    check_points(1);

    /* different continents */
    countrynr = 2;
    strcpy(continent, "AF");
    bandinx = BANDINDEX_20;
    check_points(3);

    bandinx = BANDINDEX_40;
    check_points(6);

    /* same continent, not NA */
    strcpy(continent, mycontinent);
    bandinx = BANDINDEX_20;
    check_points(1);

    bandinx = BANDINDEX_40;
    check_points(2);

    /* same continent, NA */
    strcpy(mycontinent, "NA");
    strcpy(continent, mycontinent);
    bandinx = BANDINDEX_20;
    check_points(2);

    bandinx = BANDINDEX_40;
    check_points(4);

}


void test_cqww(void **state) {
    cqww = 1;

    countrynr = mycountrynr;
    check_points(0);

    countrynr = 2;
    strcpy(continent, "EU");
    strcpy(mycontinent, "EU");
    check_points(1);

    strcpy(continent, "NA");
    strcpy(mycontinent, "NA");
    check_points(2);

    strcpy(continent, "EU");
    strcpy(mycontinent, "NA");
    check_points(3);
}

void test_arrl_fd(void **state) {
    arrl_fd = 1;

    trxmode = CWMODE;
    check_points(2);

    trxmode = SSBMODE;
    check_points(1);

}


void test_simple_points(void **state) {
    one_point = 1;
    check_points(1);
    one_point = 0;

    two_point = 1;
    check_points(2);
    two_point = 0;

    three_point = 1;
    check_points(3);
}

void test_arrldx_usa(void **state) {
    arrldx_usa = 1;

    countrynr = w_cty;
    check_points(0);

    countrynr = ve_cty;
    check_points(0);

    countrynr = mycountrynr;
    check_points(3);
}

void test_ssbcw(void **state) {
    check_points(0);

    ssbpoints = 3;
    check_points(0);

    cwpoints = 4;
    trxmode = CWMODE;
    check_points(4);
    trxmode = SSBMODE;
    check_points(3);

    lowband_point_mult = 1;
    bandinx = BANDINDEX_30;
    check_points(3);
    bandinx = BANDINDEX_40;
    check_points(6);

    portable_x2 = 1;
    check_call_points("DL3XYZ", 6);
    check_call_points("DL3XYZ/P", 12);

    trxmode = DIGIMODE;
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

void test_country_found(void **state) {
    /* nothing to find in empty list */
    strcpy(hiscall, "LZ1AB");
    assert_int_equal(country_found(""), 0);
    strcpy(hiscall, "DL3XYZ");
    assert_int_equal(country_found(""), 0);

    init_countrylist();
    strcpy(hiscall, "LZ1AB");
    assert_int_equal(country_found(""), 0);
    strcpy(hiscall, "DL3XYZ");
    assert_int_equal(country_found(""), 1);
    strcpy(hiscall, "K3LA");
    assert_int_equal(country_found(""), 1);
}

void test_scoreByCorC_listOnly(void **state) {
    countrylist_only = 1;
    check_call_points("LZ1AB", 0);
    check_call_points("DL3XYZ", 0);

    init_countrylist();
    check_call_points("LZ1AB", 0);
    check_call_points("DL3XYZ", 0);

    countrylist_points = 4;
    check_call_points("LZ1AB", 0);
    check_call_points("DL3XYZ", 4);
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

