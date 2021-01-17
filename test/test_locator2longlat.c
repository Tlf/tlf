#include "test.h"

#include <hamlib/rotator.h>
#include "../src/locator2longlat.h"

// OBJECT ../src/locator2longlat.o

int setup_default(void **state) {
    rig_set_debug(RIG_DEBUG_NONE);
    return 0;
}

void test_qra_short(void **state) {
    assert_int_equal(check_qra("JN"), 0);
}

void test_qra_wrong(void **state) {
    assert_int_equal(check_qra("AB3C"), 0);
}

void test_qra_ok4(void **state) {
    assert_int_equal(check_qra("JN97"), 1);
}

void test_qra_ok6(void **state) {
    assert_int_equal(check_qra("JN97ab"), 1);
}

typedef struct {
    double latitude, longitude;
    char *loc6;
} ll2loc_t;

// Test data from "Conversion Between Geodetic and Grid Locator Systems",
// by Edmund T. Tyson N5JTY QST January 1989
ll2loc_t ll2loc_data[] = {
    {.latitude = 48.147, .longitude = 11.608, .loc6 = "JN58TD"}, // Munich
    {.latitude = -34.91, .longitude = -56.212, .loc6 = "GF15VC"}, // Montevideo
    {.latitude = 38.92, .longitude = -77.065, .loc6 = "FM18LW"}, // Washington, DC
    {.latitude = -41.283, .longitude = 174.745, .loc6 = "RE78IR"}, // Wellington
};

void test_ll2loc(void **state) {
    char buffer[20];
    for (int i = 0; i < sizeof(ll2loc_data) / sizeof(ll2loc_t); ++i) {

	memset(buffer, 0, sizeof(buffer));

	int rc = longlat2locator(ll2loc_data[i].longitude,
				 ll2loc_data[i].latitude,
				 buffer, 3);

	assert_int_equal(rc, 0); // = RIG_OK
	assert_string_equal(buffer, ll2loc_data[i].loc6);
    }
}

