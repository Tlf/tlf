#include "test.h"

#include "../src/bands.h"

// OBJECT ../src/bands.o

void test_toBandMask(void **state) {
    assert_int_equal(inxes[BANDINDEX_160], BAND160);
    assert_int_equal(inxes[BANDINDEX_80], BAND80);
    assert_int_equal(inxes[BANDINDEX_40], BAND40);
    assert_int_equal(inxes[BANDINDEX_30], BAND30);
    assert_int_equal(inxes[BANDINDEX_20], BAND20);
    assert_int_equal(inxes[BANDINDEX_17], BAND17);
    assert_int_equal(inxes[BANDINDEX_15], BAND15);
    assert_int_equal(inxes[BANDINDEX_12], BAND12);
    assert_int_equal(inxes[BANDINDEX_10], BAND10);
    assert_int_equal(inxes[BANDINDEX_OOB], 0);
}

