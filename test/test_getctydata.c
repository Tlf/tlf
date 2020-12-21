#include "test.h"

#include "../src/tlf.h"
#include "../src/dxcc.h"
#include "../src/readctydata.h"
#include "../src/globalvars.h"
#include "../src/setcontest.h"

#include "../src/getctydata.h"

// OBJECT ../src/dxcc.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/setcontest.o

/* export internal function */
int location_unknown(char *call);
int getpfxindex(char *checkcallptr, char **normalized_call);

extern char countrylist[255][6];

contest_config_t config_focm;

int setup_default(void **state) {

    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    setcontest("qso");
    pfxmult = 0;
    strcpy(countrylist[0], "");

    return 0;
}

void test_location_known(void **state) {
    assert_int_equal(location_unknown("LA3BB"), 0);
    assert_int_equal(location_unknown("LA3BB/P"), 0);
}

void test_location_unknown(void **state) {
    assert_int_equal(location_unknown("LA3BB/MM"), 1);
    assert_int_equal(location_unknown("LA3BB/AM"), 1);
}

void test_suffix_empty(void **state) {
    assert_int_equal(getpfxindex("", NULL), -1);
    assert_int_equal(getctynr(""), 0);
    assert_int_equal(getctydata(""), 0);
}

void test_location_unknown_used(void **state) {
    assert_int_equal(getpfxindex("LA3BB/AM", NULL), -1);
    assert_int_equal(getctynr("LA3BB/AM"), 0);
    assert_int_equal(getctydata("LA3BB/AM"), 0);
}

/* getctynr */
void test_suffix_getctynr(void **state) {
    assert_int_not_equal(getctynr("LA3BB"), 0);
    assert_int_not_equal(getctynr("LA3BB/QRP"), 0);
    assert_int_equal(getctynr("LA3BB/QRP"), getctynr("LA3BB"));
}

/* getctydata */
void test_suffix_getctydata(void **state) {
    assert_int_not_equal(getctydata("LA3BB"), 0);
    assert_int_not_equal(getctydata("LA3BB/QRP"), 0);
}

void test_someidea(void **data) {
    assert_int_not_equal(getpfxindex("DJ0LN/A", NULL), -1);
    assert_int_not_equal(getpfxindex("PA/DJ0LN/P", NULL), -1);
    assert_int_not_equal(getpfxindex("DJ0LN/P", NULL), -1);
    assert_int_not_equal(getpfxindex("DJ0LN/PA/P", NULL), -1);
    assert_int_not_equal(getpfxindex("K32A/4", NULL), -1);
    assert_int_not_equal(getpfxindex("R3A/PA", NULL), -1);
    assert_int_not_equal(getpfxindex("DJ/PA3LM", NULL), -1);
}

#define check(x) (assert_int_equal(getctynr(x), getctydata(x)))

void test_same_result(void **data) {
    check("DJ0LN/A");
    check("PA/DJ0LN/P");
    check("DJ0LN/P");
    check("DJ0LN/PA/P");
    check("K32A/4");
    check("R3A/PA");
    check("DJ/PA3LM");
    check("9A70DP/KA");
}

void test_no_wpx(void **state) {
    int nr;
    pxstr[0] = '\0';
    nr = getctydata("DJ/PA3LM");
    assert_string_equal(pxstr, "");
    assert_int_equal(getctydata("DL"), nr);
}

void test_is_wpx(void **state) {
    int nr;

    setcontest("wpx");
    pxstr[0] = '\0';
    nr = getctydata("DJ/PA3LM");
    assert_string_equal(pxstr, "DJ0");
    assert_int_equal(getctydata("DL"), nr);
}

void test_pfxmult_set(void **state) {
    int nr;

    pfxmult = 1;
    pxstr[0] = '\0';
    nr = getctydata("DJ/PA3LM");
    assert_string_equal(pxstr, "DJ0");
    assert_int_equal(getctydata("DL"), nr);
}

void test_abnormal_call(void **state) {
    int nr;
    nr = getctydata("ET3AA/YOTA");
    assert_int_equal(getctydata("ET"), nr);
}


