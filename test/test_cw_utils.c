#include "test.h"

#include "../src/cw_utils.h"

// OBJECT ../src/cw_utils.o


void test_SetSpeed_success(void **state) {
    SetCWSpeed(30);
    assert_int_equal(speed, 10);
    SetCWSpeed(31);
    assert_int_equal(speed, 11);
}

void test_GetSpeed(void **state) {
    SetCWSpeed(43);
    assert_int_equal(GetCWSpeed(), 44);
    SetCWSpeed(60);
    assert_int_equal(GetCWSpeed(), 50);
}


