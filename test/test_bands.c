#include "test.h"

#include "../src/bands.h"
#include "../src/globalvars.h"

// OBJECT ../src/bands.o

/* test conversion from bandindex to bandmask */
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

/* test IsWarcIndex */
void test_IsWarcIndex(void **state) {
    assert_int_equal(IsWarcIndex(BANDINDEX_160), 0);
    assert_int_equal(IsWarcIndex(BANDINDEX_80), 0);
    assert_int_equal(IsWarcIndex(BANDINDEX_40), 0);
    assert_int_equal(IsWarcIndex(BANDINDEX_30), 1);
    assert_int_equal(IsWarcIndex(BANDINDEX_20), 0);
    assert_int_equal(IsWarcIndex(BANDINDEX_17), 1);
    assert_int_equal(IsWarcIndex(BANDINDEX_15), 0);
    assert_int_equal(IsWarcIndex(BANDINDEX_12), 1);
    assert_int_equal(IsWarcIndex(BANDINDEX_10), 0);
}

/* test switch to next band UP or DOWN */
void test_nextBandUp(void **state) {
    bandinx = BANDINDEX_12;
    next_band(BAND_UP);
    assert_int_equal(bandinx, BANDINDEX_10);
}

void test_nextBandDown(void **state) {
    bandinx = BANDINDEX_12;
    next_band(BAND_DOWN);
    assert_int_equal(bandinx, BANDINDEX_15);
}

void test_nextBandWrapUpwards(void **state) {
    bandinx = BANDINDEX_10;
    next_band(BAND_UP);
    assert_int_equal(bandinx, BANDINDEX_160);
}

void test_nextBandWrapDownwards(void **state) {
    bandinx = BANDINDEX_160;
    next_band(BAND_DOWN);
    assert_int_equal(bandinx, BANDINDEX_10);
}

