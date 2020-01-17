#include "test.h"

#include <stdbool.h>

#include "../src/tlf.h"
#include "../src/dxcc.h"
#include "../src/readctydata.h"
#include "../src/globalvars.h"
#include "../src/getctydata.h"
#include "../src/readcalls.h"

// OBJECT ../src/log_utils.o
// OBJECT ../src/addmult.o
// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/locator2longlat.o
// OBJECT ../src/readcalls.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/score.o
// OBJECT ../src/zone_nr.o

/* missing from globalvar.h */
extern char continent_multiplier_list[7][3];
extern char countrylist[][6];

// dummy functions
void get_time(void) {}
void readqtccalls() {}
void shownr(char *msg, int x) {}
void spaces(int n) {} /* needs more care */

int qrb(double a, double b, double c, double d) {
    return 1;
}

int foc_score(char *a) {
    return 1;
}

int pacc_pa(void) {
    return 0;
}

/* private prototypes */
bool is_in_countrylist(int x);


int setup_default (void **state) {
    char filename[100];

    strcpy(filename, TOP_SRCDIR);
    strcat(filename, "/share/cty.dat");
    assert_int_equal(load_ctydata(filename), 0);

    strcpy(countrylist[0], "DL");
    strcpy(countrylist[1], "CE");
    strcpy(countrylist[2], "");
    return 0;
}

/* test is_in_countrylist() */
void test_in_countrylist(void **state) {
    assert_int_equal(is_in_countrylist(getctynr("DL")), true);
}

void test_not_in_countrylist(void **state) {
    assert_int_equal(is_in_countrylist(getctynr("OE")), false);
}

void test_in_countrylist_keeps_countrynr(void **state) {
    countrynr = 42;
    assert_int_equal(is_in_countrylist(getctynr("DL")), true);
    assert_int_equal(is_in_countrylist(getctynr("OE")), false);
    assert_int_equal(countrynr, 42);
}
