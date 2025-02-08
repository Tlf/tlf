#include "test.h"

#include "../src/lancode.h"
#include "../src/tlf.h"
#include "../src/err_utils.h"

#include "../src/globalvars.h"

// OBJECT ../src/lancode.o

void handle_logging(enum log_lvl lvl, ...) {
    // empty
}

time_t get_time() {
    return 0;   // TBD
}
void clear_line(int row) {
}

int setup_default(void **state) {

    trx_control = true;
    nodes = 1;
    lan_active = true;
    using_named_nodes = false;
    strcpy(bc_hostaddress[0], "host0");

    sendto_call_count = 0;
    FREE_DYNAMIC_STRING(sendto_last_message);

    return 0;
}

void test_send_freq_80(void **state) {

    send_freq(3567891.0);

    assert_int_equal(sendto_call_count, 1);
    assert_non_null(sendto_last_message);
    assert_string_equal(sendto_last_message, "A5 3567.9\n");
}

void test_send_freq_10(void **state) {

    send_freq(28123456.0);

    assert_int_equal(sendto_call_count, 1);
    assert_non_null(sendto_last_message);
    assert_string_equal(sendto_last_message, "A528123.5\n");
}

void test_send_freq_80_notrx(void **state) {

    trx_control = false;

    bandinx = BANDINDEX_80;
    send_freq(0);

    assert_int_equal(sendto_call_count, 1);
    assert_non_null(sendto_last_message);
    assert_string_equal(sendto_last_message, "A5   80.0\n");
}

void test_send_freq_10_notrx(void **state) {

    trx_control = false;

    bandinx = BANDINDEX_10;
    send_freq(0);

    assert_int_equal(sendto_call_count, 1);
    assert_non_null(sendto_last_message);
    assert_string_equal(sendto_last_message, "A5   10.0\n");
}
