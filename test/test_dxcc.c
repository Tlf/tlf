#include "test.h"

#include "../src/tlf.h"
#include "../src/dxcc.h"


// OBJECT ../src/dxcc.o

extern prefix_data dummy_pfx;

int setup_default(void **state) {
    dxcc_init();
    prefix_init();
    dxcc_add("Rotuma Island:            32:  56:  OC:  -12.48:  -177.08:   -12.0:  3D2/r:");
    dxcc_add("France:                   14:  27:  EU:   46.00:    -2.00:    -1.0:  F:");
    return 0;
}


void test_dxcc_empty_after_init(void **state) {
    dxcc_init();
    assert_int_equal(0, dxcc_count());
}

void test_prefix_empty_after_init(void **state) {
    prefix_init();
    assert_int_equal(0, prefix_count());
}

void test_prefix_by_index_out_of_bounds(void **state) {
    assert_ptr_equal(&dummy_pfx, prefix_by_index(dxcc_count()));
}

void test_dxcc_by_index_out_of_bounds(void **state) {
    assert_ptr_equal(dxcc_by_index(0), dxcc_by_index(dxcc_count()));
}

void test_add_dxcc_check_count(void **state) {
    assert_int_equal(2, dxcc_count());
}

void test_add_dxcc_check_parsed(void **state) {
    dxcc_data *mydx;
    mydx = dxcc_by_index(1);
    assert_string_equal(mydx->countryname, "France");
    assert_int_equal(mydx->cq, 14);
    assert_int_equal(mydx->itu, 27);
    assert_true(mydx->lat == 46.);
    assert_true(mydx->lon == -2.);
    assert_true(mydx->timezone == -1.0);
    assert_string_equal(mydx->continent, "EU");
    assert_string_equal(mydx->pfx, "F");
    assert_false(mydx->starred);
}

void test_add_dxcc_check_starred(void **state) {
    dxcc_data *mydx;
    dxcc_add("France:                   14:  27:  EU:   46.00:    -2.00:    -1.0:  *F:");
    mydx = dxcc_by_index(2);
    assert_true(mydx->starred);
}

void test_add_prefix_check_count(void **state) {
    prefix_add("F");
    prefix_add("HW");
    assert_int_equal(2, prefix_count());
}

void test_add_prefix_check_defaults(void **state) {
    prefix_data *pfx;
    prefix_add("HW");
    pfx = prefix_by_index(0);
    assert_string_equal(pfx->pfx, "HW");
    assert_int_equal(pfx->dxcc_ctynr, dxcc_count() - 1);
    dxcc_data *mydx = dxcc_by_index(dxcc_count() - 1);
    assert_int_equal(pfx->cq, mydx->cq);
    assert_int_equal(pfx->itu, mydx->itu);
    assert_string_equal(pfx->continent, mydx->continent);
    assert_float_equal(pfx->lat, mydx->lat, 1e-6);
    assert_float_equal(pfx->lon, mydx->lon, 1e-6);
    assert_float_equal(pfx->timezone, mydx->timezone, 1e-6);
}

void test_add_prefix_check_overrides(void **state) {
    prefix_data *pfx;
    // NOTE: overrides must be in the order below
    char *input = g_strdup("HW(11)[22]<33.3/44.4>{OC}~5.5~");
    prefix_add(input);
    g_free(input);
    pfx = prefix_by_index(0);
    assert_string_equal(pfx->pfx, "HW");
    assert_int_equal(pfx->dxcc_ctynr, dxcc_count() - 1);
    assert_int_equal(pfx->cq, 11);
    assert_int_equal(pfx->itu, 22);
    assert_string_equal(pfx->continent, "OC");
    assert_float_equal(pfx->lat, 33.3, 1e-6);
    assert_float_equal(pfx->lon, 44.4, 1e-6);
    assert_float_equal(pfx->timezone, 5.5, 1e-6);
}

