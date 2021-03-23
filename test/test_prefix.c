#include "test.h"

#include "../src/getpx.h"

// OBJECT ../src/getpx.o

/* internal function, not exported */
int letters_only(const char *call);

#define check(call, pfx) \
    do{ getpx(call); assert_string_equal(wpx_prefix, pfx); }while(0)


extern char wpx_prefix[];

void test_letters_only_yes(void **state) {
    assert_int_equal(letters_only("RAEM"), 1);
    assert_int_equal(letters_only("XEFTJW"), 1);
}


void test_letters_only_no(void **state) {
    assert_int_equal(letters_only("DL1JBE"), 0);
    assert_int_equal(letters_only("XEFTJW/3"), 0);
}

void test_simple_calls(void **state) {
    check("DL1JBE", "DL1");
    check("K9W", "K9");
    check("Z35LA", "Z35");
    check("ZS66DW", "ZS66");
    check("U3NBL", "U3");
    check("3D2AB", "3D2");
    check("3D20AB", "3D20");
    check("LY1000IARU", "LY1000");
    check("DL60CHILD", "DL60");
    check("9A800VZ", "9A800");
    check("DR2006Q", "DR2006");
    check("R3A", "R3");
}

void test_other_callarea(void **state) {
    check("DL1JBE/3", "DL3");
    check("WN5N/7", "WN7");
    check("KH6XXX/4", "KH4");
}

void test_other_country(void **state) {
    check("PA0/DL1JBE", "PA0");
    check("PA/DL1JBE", "PA0");
    check("PA2/DL1JBE", "PA2");
    check("LX/WN5N", "LX0");
    check("J6/DL1JBE", "J6");
    check("KH4/KH6XX", "KH4");
    check("OE/K5ZD", "OE0");
}

void test_nodigit(void **state) {
    check("RAEM", "RA0");
    check("XEFTJW", "XE0");
}

void test_reported(void **state) {
    check("K3A/2", "K2");
//    check("KL32A/4", "KL32");
}

