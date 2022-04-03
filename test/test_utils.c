#include "test.h"

#include <hamlib/rotator.h>

#include "../src/utils.h"

// OBJECT ../src/utils.o

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


typedef struct {
    char *call1;
    char *call2;
    char *expected;
} partial_call_t;

static partial_call_t tests[] = {
    {"WB6A",    "W6BA",    "W6BA"},
    {"HA2OH",   "HA2OS",   "OS"},
    {"HA2MS",   "HA2OS",   "OS"},
    {"HA2MH",   "HA2OS",   "OS"},
    {"HA2MH",   "S52OS",   "S52OS"},
    {"S2MH",    "S52OS",   "S52OS"},
    {"S2MH",    "S52MH",   "S5"},
    {"OK2FHI",  "OK1FHI",  "OK1"},
    {"OK1FH",   "OK1FHI",  "FHI"},
    {"OK2FH",   "OK1FHI",  "1FHI"},
    {"WB6AB",   "WB6A",    "6A"},
    {"HG5N",    "HA5N",    "HA"},
    {"HG5N",    "HG55N",   "55"},
    {"HG5N",    "HG6N",    "HG6"},
    {"HG5N",    "HG5D",    "5D"},
    {"PA1234X", "PA1834X", "1834"},
    {"PA1234X", "PA1834K", "1834K"},
    {"PA1234X", "PA1234K", "4K"},
    {"PA1234X", "RA1234X", "RA"},
    {"F1234X",  "R1234X",  "R1"},
    {"F1234X",  "R1234C",  "R1234C"},
    {"K1A",     "K1W",     "1W"},
    {"K1A",     "W1A",     "W1"},
    {"K1A",     "K2A",     "K2"},
    {"DL1ABC",  "DL1ADC",  "ADC"},
    {"R1ABC/2", "R1ABC/3", "/3"},
    {"R1ABC/2", "R1ADC/2", "ADC"},
    {"R1ABC/2", "R4ADC/2", "4ADC"},
    {"R1ABC/2", "R1ADC/3", "ADC/3"},
    {"R1A/2",   "R1W/2",   "1W"},
    {"EA8/HB9ABC", "EA8/HB9ANC", "ANC"},
    {"EA8/HB9ABC", "EA6/HB9ABC", "EA6/"},
    {"EA8/HB9ABC", "EA6/HB9ANC", "EA6/HB9ANC"},
    {"EA8/DL1ABC", "EA8/DK1ABC", "DK"},
    {"EA8/DL1ABC", "EA6/DK1ABC", "EA6/DK"},
    {"G/DF1ABC",   "F/DF1ABC",   "F/"},
    {"LA/DF1ABC",  "PA/DF1ABC",  "PA/"},
    {"HA5ABC/P",   "HA5ABC",    "ABC"},
    {"HA5ABC",     "HA5ABC/P",  "/P"},
    {"HB9/OK1ABC", "OK1ABC",    "OK"},
    {"OK1ABC",     "HB9/OK1ABC",  "HB9/"},
    {"HA5ABE",     "HA5AB",     "AB"},
    {"HA5ABE",     "HW5AB",     "HW5AB"},
};


void test_get_partial_callsign(void **state) {
    char partial[20];
    for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
	printf("%s -> %s\n", tests[i].call1, tests[i].call2);
	get_partial_callsign(tests[i].call1, tests[i].call2, partial);
	assert_string_equal(tests[i].expected, partial);
    }
}

