#include "test.h"
#include <stdio.h>

#include "../src/bandmap.h"
#include "../src/dxcc.h"
#include "../src/qtcutil.h"

// OBJECT ../src/bandmap.o
// OBJECT ../src/bands.o
 

bool grab_up=false;

int getctynr(char *checkcall) {
    return 0;
}

int lookup_worked(char *call) {
    return -1;	/* not found */
}

static dxcc_data dummy_dxcc = {
    "Noland",
    1,
    2,
    "NO",
    34,
    56,
    7,
    "QQ",
    false
};

dxcc_data *dxcc_by_index(unsigned int index) {
    return &dummy_dxcc;
}

char thisnode = 'A';

bool general_ismulti(spot *data) {
    return false;
}

bool worked_in_current_minitest_period(int found) {
    return false;
}

/* will not work for testing bm_isdupe and qtc_format */
struct t_qtc_store_obj *qtc_get(char callsign[QTC_CALL_SIZE]) {
    return NULL;
}

char qtc_get_value(struct t_qtc_store_obj *qtc_obj) {
    return 'N';
}

int modify_attr(int attr) {
    return attr;
}



int setup_default(void **state) {
    bm_init();

    return 0;
}

extern GPtrArray *spots;

int teardown_default(void **state) {
    /* empty array */
    int x = spots->len;
    for (int i = 0; i < x ; i++) {
	g_ptr_array_remove_index (spots, 0);
    }
    return 0;
}

void add_spot(char * call, freq_t freq) {
    spot *entry = g_new(spot, 1);
    /* partial filled spot entry, enough for bandmap_lookup */
    entry -> call = g_strdup(call);
    entry -> freq = freq;

    g_ptr_array_add(spots, entry);
}

void populate_spots() {
    add_spot("VK9XYZ", 7023000.);
    add_spot("ZL0OEE", 7123000.);
    add_spot("K0VKZ", 14023000.);
    add_spot("OE0ZZ", 14123000.);
}

void test_empty_spots(void **state) {
    assert_ptr_equal(NULL, bandmap_lookup("AA"));
}

void test_not_in_spots(void **state) {
    populate_spots();
    assert_ptr_equal(NULL, bandmap_lookup("AA"));
}

void test_find_anywhere_if_single(void **state) {
    populate_spots();

    assert_string_equal("K0VKZ", bandmap_lookup("KZ")->call);
    assert_string_equal("ZL0OEE", bandmap_lookup("ZL")->call);
}

/* old behaviour, should fail now */
//void test_find_always_first(void **state) {
//    populate_spots();
//
//    assert_string_equal("ZL0OEE", bandmap_lookup("OE")->call);
//    assert_string_equal("VK9XYZ", bandmap_lookup("VK")->call);
//}

void test_prefer_country(void **state) {
    populate_spots();

    assert_string_equal("OE0ZZ", bandmap_lookup("OE")->call);
    assert_string_equal("VK9XYZ", bandmap_lookup("VK")->call);
}
