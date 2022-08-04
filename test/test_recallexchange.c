#include "test.h"

#include "../src/recall_exchange.h"
#include "../src/initial_exchange.h"
#include "../src/setcontest.h"
#include "../src/globalvars.h"
#include "../src/tlf.h"

// OBJECT ../src/recall_exchange.o
// OBJECT ../src/initial_exchange.o

contest_config_t config_any = {
    .id = 123,
    .name = QSO_MODE,
    .exchange_width = 10
};

int setup_default(void **state) {
    int result;
    strcpy(hiscall, "N0ONE");
    current_qso.comment = g_malloc0(COMMENT_SIZE);
    strcpy(proposed_exchange, "");

    strcpy(worked[0].call, "DL1ABC");
    strcpy(worked[0].exchange, "51N13E");
    nr_worked = 1;

    main_ie_list = NULL;
    contest = &config_any;

    result = chdir(SRCDIR);
    if (result == -1)
	perror("chdir");
    return 0;
}

void test_empty_call(void **state) {
    strcpy(hiscall, "");
    assert_int_equal(recall_exchange(), 0);
    assert_string_equal(current_qso.comment, "");
}

void test_respect_nonempty_comment(void **state) {
    strcpy(current_qso.comment, "Hi");
    assert_int_equal(recall_exchange(), 0);
    assert_string_equal(current_qso.comment, "Hi");
}

void test_not_found(void **state) {
    assert_int_equal(recall_exchange(), -1);
    assert_string_equal(current_qso.comment, "");
}

void test_from_worked(void **state) {
    strcpy(hiscall, "DL1ABC");
    assert_int_equal(recall_exchange(), 1);
    assert_string_equal(current_qso.comment, "51N13E");
}

void test_from_ielist(void **state) {
    main_ie_list = make_ie_list("data/ie_ok.txt");
    strcpy(hiscall, "2E0AAA");
    assert_int_equal(recall_exchange(), 1);
    assert_string_equal(current_qso.comment, "51N3W");
}

