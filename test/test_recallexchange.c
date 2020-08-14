#include "test.h"

#include "../src/recall_exchange.h"
#include "../src/initial_exchange.h"
#include "../src/tlf.h"

// OBJECT ../src/recall_exchange.o
// OBJECT ../src/initial_exchange.o

extern char comment[];
extern char hiscall[];
extern struct ie_list *main_ie_list;
extern worked_t worked[];
extern int nr_worked;

int setup_default(void **state) {
    int result;
    strcpy(hiscall, "N0ONE");
    strcpy(comment, "");

    strcpy(worked[0].call, "DL1ABC");
    strcpy(worked[0].exchange, "51N13E");
    nr_worked = 1;

    result = chdir(SRCDIR);
    if (result == -1)
	perror("chdir");
    return 0;
}

void test_empty_call(void **state) {
    strcpy(hiscall, "");
    assert_int_equal(recall_exchange(), 0);
    assert_string_equal(comment, "");
}

void test_respect_nonempty_comment(void **state) {
    strcpy(comment, "Hi");
    assert_int_equal(recall_exchange(), 0);
    assert_string_equal(comment, "Hi");
}

void test_not_found(void **state) {
    assert_int_equal(recall_exchange(), -1);
    assert_string_equal(comment, "");
}

void test_from_worked(void **state) {
    strcpy(hiscall, "DL1ABC");
    assert_int_equal(recall_exchange(), 1);
    assert_string_equal(comment, "51N13E");
}

void test_from_ielist(void **state) {
    main_ie_list = make_ie_list("data/ie_ok.txt");
    strcpy(hiscall, "2E0AAA");
    assert_int_equal(recall_exchange(), 1);
    assert_string_equal(comment, "51N3W");
}

