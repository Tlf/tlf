#include "test.h"

#include <time.h>

#include "../src/searchcallarray.h"
#include "../src/bands.h"
#include "../src/globalvars.h"

// OBJECT ../src/searchcallarray.o
// OBJECT ../src/bands.o


int getctynr(char *call) {
    return 42;
}

time_t get_time() {
    return (time_t)mock();
}

void fill_qsotime(long time) {
    for (int i = 0; i < 3; i++)
	for (int j = 0; j < NBANDS; j++)
	    worked[nr_worked].qsotime[i][j] = time + 10 * j + i;
}

void insert_call(char *call, long time) {
    strcpy(worked[nr_worked].call, call);
    worked[nr_worked].band = inxes[BANDINDEX_40] | inxes[BANDINDEX_15];
    fill_qsotime(time);
    nr_worked ++;
}

int setup_default(void **state) {
    init_worked();
    insert_call("W1AA", 80000);
    insert_call("OE3XYZ", 80500);
    worked[0].qsotime[SSBMODE][BANDINDEX_40] = 0; /* not worked here */

    minitest = 0;
    bandinx = BANDINDEX_40;
    trxmode = CWMODE;

    qso_once = false;
    mixedmode = false;

    return 0;
}

void test_init(void **state) {
    init_worked();
    assert_int_equal(nr_worked, 0);
    assert_int_equal(worked[0].country, -1);
}

void test_lookup_emptylist(void **state) {
    init_worked();
    assert_int_equal(searchcallarray("DL1ABC"),-1);
}

void test_lookup_notfound(void **state) {
    assert_int_equal(searchcallarray("DL1ABC"),-1);
}

void test_lookup_found(void **state) {
    assert_int_equal(nr_worked,2);
    assert_int_equal(searchcallarray("W1AA"),0);
    assert_int_equal(searchcallarray("OE3XYZ"),1);
}


/* test worked_in_current_minitest_period */
void test_not_found(void **state) {
    assert_int_equal(worked_in_current_minitest_period(-1), false);
}

void test_no_minitest(void **state) {
    int index = searchcallarray("OE3XYZ");
    assert_int_equal(worked_in_current_minitest_period(index), true);
}

void test_minitest_in_period(void **state) {
    int index = searchcallarray("OE3XYZ");
    minitest = 500;

    will_return(get_time, 80500 + minitest - 1);
    assert_int_equal(worked_in_current_minitest_period(index), true);
}

void test_minitest_not_in_period(void **state) {
    int index = searchcallarray("OE3XYZ");
    minitest = 500;

    will_return(get_time, 80500 + minitest);
    assert_int_equal(worked_in_current_minitest_period(index), false);
}


/* test is_dupe */
void test_new_station(void **state) {
    assert_int_equal(is_dupe("DL1ABC", BANDINDEX_80, CWMODE), false);
}

void test_wrong_band(void **state) {
    assert_int_equal(is_dupe("OE3XYZ", BANDINDEX_80, CWMODE), false);
}

void test_ignore_band_if_qso_once_set(void **state) {
    qso_once = 1;
    assert_int_equal(is_dupe("OE3XYZ", BANDINDEX_80, CWMODE), true);
}

void test_ignore_mode(void **state) {
    assert_int_equal(is_dupe("OE3XYZ", BANDINDEX_40, SSBMODE), true);
    assert_int_equal(is_dupe("W1AA", BANDINDEX_40, SSBMODE), true);
}

void test_check_mode_if_mixedmode_set(void **state) {
    mixedmode = true;
    assert_int_equal(is_dupe("OE3XYZ", BANDINDEX_40, SSBMODE), true);
    assert_int_equal(is_dupe("W1AA", BANDINDEX_40, SSBMODE), false);
}

void test_out_of_current_period(void **state) {
    minitest = 500;

    will_return(get_time, 80500 + minitest);
    assert_int_equal(is_dupe("OE3XYZ", BANDINDEX_40, SSBMODE), false);
}

