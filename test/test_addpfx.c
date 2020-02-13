#include "test.h"

#include "../src/addpfx.h"
#include "../src/tlf.h"

// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o

extern int pfxmultab;

int setup_default(void **state) {
    pfxmultab = 0;
    InitPfx();

    return 0;
}

/* test initialization */
void test_initPfx(void **state) {
    assert_int_equal(GetNrOfPfx_once(), 0);
    assert_int_equal(GetNrOfPfx_multiband(), 0);
    int i, temp = 0;
    for (i = 0; i<NBANDS; i++)
	temp += GetNrOfPfx_OnBand(i);
    assert_int_equal(temp, 0);
}

void test_outOfBand(void ** state){
    assert_int_equal(GetNrOfPfx_OnBand(NBANDS) , 0);
}

/* test adding prefixes */
void test_addOne(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    assert_int_equal(GetNrOfPfx_once(), 1);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_40), 1);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_80), 0);
}

void test_addOneTwoTimes(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    add_pfx("DL0", BANDINDEX_40);
    assert_int_equal(GetNrOfPfx_once(), 1);
    assert_int_equal(GetNrOfPfx_multiband(), 1);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_40), 1);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_80), 0);
}

void test_addOneTwoBands(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    add_pfx("DL0", BANDINDEX_80);
    assert_int_equal(GetNrOfPfx_once(), 1);
    assert_int_equal(GetNrOfPfx_multiband(), 2);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_40), 1);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_80), 1);
}

void test_addTwoSameBand(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    add_pfx("OE3", BANDINDEX_40);
    assert_int_equal(GetNrOfPfx_once(), 2);
    assert_int_equal(GetNrOfPfx_multiband(), 2);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_40), 2);
}

void test_addTwoTwoBands(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    add_pfx("OE3", BANDINDEX_80);
    assert_int_equal(GetNrOfPfx_once(), 2);
    assert_int_equal(GetNrOfPfx_multiband(), 2);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_40), 1);
    assert_int_equal(GetNrOfPfx_OnBand(BANDINDEX_80), 1);
}

/* test return value of add_pfx() */
void test_addKnown(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    assert_int_equal(add_pfx("DL0", BANDINDEX_40), 1);
    assert_int_equal(add_pfx("DL0", BANDINDEX_80), 1);
}

void test_addUnknown(void **state) {
    add_pfx("DL0", BANDINDEX_40);
    assert_int_equal(add_pfx("OE3", BANDINDEX_40), 0);
}

void test_addKnownMultiband(void **state) {
    pfxmultab = 1;
    add_pfx("DL0", BANDINDEX_40);
    assert_int_equal(add_pfx("DL0", BANDINDEX_40), 1);
    assert_int_equal(add_pfx("DL0", BANDINDEX_80), 0);
}

