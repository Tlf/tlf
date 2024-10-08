#include "test.h"

#include "../src/lancode.h"
#include "../src/tlf.h"
#include "../src/err_utils.h"

#include "../src/globalvars.h"

// OBJECT ../src/lancode.o

extern bool cl_send_inhibit;

void handle_logging(enum log_lvl lvl, ...) {
    // empty
}

time_t get_time() {
    return 1728323637;   // ~ 7Oct24 19:19
}

void clear_line(int row) {
}

int setup_default(void **state) {

    trx_control = true;
    nodes = 1;
    thisnode = 'A';
    lan_active = true;
    cl_send_inhibit = false;

    sendto_call_count = 0;
    FREE_DYNAMIC_STRING(sendto_last_message);

    return 0;
}

void check_line_format(int code, char *msg) {
    assert_non_null(sendto_last_message);
    assert_int_equal(sendto_last_message[0], thisnode);
    assert_int_equal(sendto_last_message[1], code);
    assert_int_equal(sendto_last_message[strlen(sendto_last_message) - 1], '\n');
    sendto_last_message[strlen(sendto_last_message) - 1] = '\0'; // drop NL
    assert_string_equal(sendto_last_message+2, msg);
}

void check_msg(char *msg) {
    assert_int_equal(sendto_call_count, 1);
    assert_non_null(sendto_last_message);
    assert_string_equal(sendto_last_message, msg);
}


void test_lan_send_message_base(void **state) {
    thisnode = 'B';
    send_lan_message(TLFMSG, "Hello");
    assert_int_equal(sendto_call_count, 1);
    check_line_format(TLFMSG, "Hello");
}

void test_lan_send_message_all_codes(void **state) {
    for (int i = 0; i < NR_LAN_CODES; i++) {
	send_lan_message(LOGENTRY + i, "test string");
	assert_int_equal(sendto_call_count, i + 1);
	check_line_format(LOGENTRY + i, "test string");
    }
}

void test_lan_send_message_inhibit_cluster(void **state) {
    cl_send_inhibit = true;
    send_lan_message(CLUSTERMSG, "DX de ...");
    assert_int_equal(sendto_call_count, 0);
    assert_null(sendto_last_message);

    send_lan_message(LOGENTRY, "160....");
    assert_int_equal(sendto_call_count, 1);
    check_line_format(LOGENTRY, "160....");

}


void test_send_freq_80(void **state) {

    send_freq(3567891.0);

    check_msg("A5 3567.9\n");
}

void test_send_freq_10(void **state) {

    send_freq(28123456.0);

    check_msg("A528123.5\n");
}

void test_send_freq_80_notrx(void **state) {

    trx_control = false;

    bandinx = BANDINDEX_80;
    send_freq(0);

    check_msg("A5   80.0\n");
}

void test_send_freq_10_notrx(void **state) {

    trx_control = false;

    bandinx = BANDINDEX_10;
    send_freq(0);

    check_msg("A5   10.0\n");
}

void test_send_time(void **state) {
    send_time();

    check_msg("A71728323637\n");
}
