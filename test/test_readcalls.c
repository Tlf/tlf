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
// OBJECT ../src/get_time.o
// OBJECT ../src/locator2longlat.o
// OBJECT ../src/readcalls.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/score.o
// OBJECT ../src/zone_nr.o

/* missing from globalvar.h */
extern pfxnummulti_t pfxnummulti[MAXPFXNUMMULT];
extern int pfxnummultinr;
extern char continent_multiplier_list[7][3];
extern char countrylist[][6];
extern int exclude_multilist_type;
extern bool continentlist_only;

// dummy functions
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
bool check_veto();


int setup_default(void **state) {

    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    strcpy(countrylist[0], "DL");
    strcpy(countrylist[1], "CE");
    strcpy(countrylist[2], "");

    strcpy(continent_multiplier_list[0], "EU");
    strcpy(continent_multiplier_list[1], "NA");
    strcpy(continent_multiplier_list[2], "");

    exclude_multilist_type = EXCLUDE_NONE;
    continentlist_only = false;

    memset(pfxnummulti, 0, sizeof(pfxnummulti));
    pfxnummulti[0].countrynr = 12;
    pfxnummulti[1].countrynr = 42;
    pfxnummultinr = 2;

    return 0;
}


/* test lookup country in pfxnummult */
void test_lookup_not_in_pfxnummult(void **state) {
    assert_int_equal(lookup_country_in_pfxnummult_array(1), -1);
}


void test_lookup_in_pfxnummult(void **state) {
    assert_int_equal(lookup_country_in_pfxnummult_array(42), 1);
}


/* test check_veto() */
void test_veto_eclude_none(void **state) {
    assert_int_equal(check_veto(), false);
}

void test_veto_exclude_country(void **state) {
    exclude_multilist_type = EXCLUDE_COUNTRY;
    countrynr = getctynr("HB9ABC");
    assert_int_equal(check_veto(), false);
    countrynr = getctynr("DL1AAA");
    assert_int_equal(check_veto(), true);
}

void test_veto_exclude_continent_contlist_only(void **state) {
    continentlist_only = true;
    exclude_multilist_type = EXCLUDE_CONTINENT;
    strcpy(continent, "EU");
    assert_int_equal(check_veto(), false);
    strcpy(continent, "AF");
    assert_int_equal(check_veto(), false);
}

void test_veto_exclude_continent(void **state) {
    exclude_multilist_type = EXCLUDE_CONTINENT;
    strcpy(continent, "EU");
    assert_int_equal(check_veto(), true);
    strcpy(continent, "AF");
    assert_int_equal(check_veto(), false);
}
