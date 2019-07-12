#include "test.h"

#include "../src/initial_exchange.h"

// OBJECT ../src/initial_exchange.o

struct ie_list *data = NULL;

int setup_default(void **state) {
    int result;
    showmsg_spy = showstring_spy1 = showstring_spy2 = STRING_NOT_SET;
    result = chdir(SRCDIR);
    if (result == -1)
	perror("chdir");
    return 0;
}

int teardown_default(void **state) {
    free_ie_list(data);
    return 0;
}

void test_no_file(void **state) {
    struct ie_list *data = make_ie_list("/nosuchfile");
    assert_string_equal(showmsg_spy, "Cannot find initial exchange file");
    assert_string_equal(showstring_spy1, STRING_NOT_SET);
    assert_string_equal(showstring_spy2, STRING_NOT_SET);
    assert_null(data);
}

static void check_data(struct ie_list *data) {
    assert_non_null(data);
    int size = 0;
    struct ie_list *r[3];
    struct ie_list *p = data;
    while (p) {
	if (size < 3) {
	    r[size] = p;    // save result for later checks
	}
	++size;
	p = p->next;
    }
    assert_int_equal(size, 3);
    // note: order is reverse as in the file
    assert_string_equal(r[0]->call, "YU5T");
    assert_string_equal(r[0]->exchange, "43N22O");
    assert_string_equal(r[1]->call, "2E0AAA");
    assert_string_equal(r[1]->exchange, "51N3W");
    assert_string_equal(r[2]->call, "2E0BBB");
    assert_string_equal(r[2]->exchange, "51N00W");
}

void test_ok(void **state) {
    data = make_ie_list("data/ie_ok.txt");
    assert_string_equal(showmsg_spy, STRING_NOT_SET);
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_ok.txt");
    check_data(data);
}

void test_ok_dos(void **state) {
    data = make_ie_list("data/ie_ok_dos.txt");
    assert_string_equal(showmsg_spy, STRING_NOT_SET);
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_ok_dos.txt");
    check_data(data);
}

void test_long_line(void **state) {
    data = make_ie_list("data/ie_long_line.txt");
    assert_string_equal(showmsg_spy, "Line 1: too long");
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_long_line.txt");
    assert_null(data);
}

void test_no_comma(void **state) {
    data = make_ie_list("data/ie_no_comma.txt");
    assert_string_equal(showmsg_spy, "Line 2: no comma found");
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_no_comma.txt");
    assert_null(data);
}

void test_ok_tab(void **state) {
    data = make_ie_list("data/ie_ok_tab.txt");
    assert_string_equal(showmsg_spy, STRING_NOT_SET);
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_ok_tab.txt");
    check_data(data);
}

void test_empty_call(void **state) {
    data = make_ie_list("data/ie_empty_call.txt");
    assert_string_equal(showmsg_spy,
			"Line 1: 0 or more than one token before comma");
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_empty_call.txt");
    assert_null(data);
}

void test_ok_multi_column(void **state) {
    data = make_ie_list("data/ie_ok_multi_column.txt");
    assert_string_equal(showmsg_spy, STRING_NOT_SET);
    assert_string_equal(showstring_spy1, "Using initial exchange file");
    assert_string_equal(showstring_spy2, "data/ie_ok_multi_column.txt");
    check_data(data);
}

