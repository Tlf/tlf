#include "test.h"

#include "../src/log_utils.h"
#include "../src/globalvars.h"

// OBJECT ../src/log_utils.o
// OBJECT ../src/bands.o


#define QSO1 " 40SSB 12-Jan-18 16:34 0006  SP9ABC         599  599  15                    10         "
#define QSO2 " 60SSB 12-Jan-18 16:34 0006  SP9ABC         599  599  15                     1         "
#define QSO_BAD1 "180SSB 12-Jan-18 16:34 0006  SP9ABC         599  599  15                     1         "


/* check test for commented logline */
void test_iscomment(void **state) {
    assert_int_equal(log_is_comment(";Hi"), 1);
}

void test_isnocomment(void **state) {
    assert_int_equal(log_is_comment("Hi"), 0);
}


/* test reading used band index */
void test_getband_ok(void **state) {
    assert_int_equal(log_get_band(QSO1), BANDINDEX_40);
    assert_int_equal(log_get_band(QSO2), BANDINDEX_60);
}

void test_getband_wrongband(void **state) {
    assert_int_equal(log_get_band(QSO_BAD1), BANDINDEX_OOB);
}

/* test reading mode */
void test_getmode_ok(void **state) {
    assert_int_equal(log_get_mode(QSO1), SSBMODE);
}

void test_getmode_ignores_case(void **state) {
    char *tmp = g_strdup(QSO1);
    memcpy(tmp + 3, "ssb", 3);
    assert_int_equal(log_get_mode(tmp), SSBMODE);
    g_free(tmp);
}

void test_getmode_error(void **state) {
    char *tmp = g_strdup(QSO1);
    memcpy(tmp + 3, "x y", 3);
    assert_int_equal(log_get_mode(tmp), -1);
    g_free(tmp);
}


/* test reading points */
void test_getpoints(void **state) {
    assert_int_equal(log_get_points(QSO1), 10);
}
