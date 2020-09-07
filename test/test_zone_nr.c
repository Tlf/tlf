#include "test.h"

#include "../src/zone_nr.h"

// OBJECT ../src/zone_nr.o


void test_ok_08(void **state) {
    assert_int_equal(zone_nr("08"), 8);
}

void test_ok__8(void **state) {
    assert_int_equal(zone_nr(" 8"), 8);
}

void test_ok_14(void **state) {
    assert_int_equal(zone_nr("14"), 14);
}

void test_bad_empty(void **state) {
    assert_int_equal(zone_nr(""), 0);
    assert_int_equal(zone_nr("\0001"), 0);
}

void test_bad_nan(void **state) {
    assert_int_equal(zone_nr("foo"), 0);
}

void test_bad_short(void **state) {
    assert_int_equal(zone_nr("7"), 0);
}
