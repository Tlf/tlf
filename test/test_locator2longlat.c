#include "test.h"

#include "../src/locator2longlat.h"

// OBJECT ../src/locator2longlat.o

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

