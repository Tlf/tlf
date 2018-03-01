#include "test.h"

#include "../src/globalvars.h"

#include "../src/tlf_curses.h"
#include "../src/tlf_panel.h"
#include "../src/searchlog.h"
#include "../src/dxcc.h"

// OBJECT ../src/searchlog.o
// OBJECT ../src/zone_nr.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/nicebox.o
// OBJECT ../src/qtcutil.o
// OBJECT ../src/printcall.o

extern WINDOW *search_win;
extern int nr_bands;

const char *NOT_SET = "__NOT_SET__";

static const char *msg, *msg1, *msg2;

void showmsg(char *message) {
    msg = message;
}

void showstring(char *message1, char *message2) {
    msg1 = message1;
    msg2 = message2;
}

/*********************/
// mocks

// stoptx.c
int stoptx() {
    return 0;
}

// ui_utils.c
int modify_attr(int attr) {
    return attr;
}

// get_time.c
void get_time(void) {
}

// getpx.c
void getpx(char *checkcall) {
}

// dxcc.c
dxcc_data *dxcc_by_index(unsigned int index) {
    return NULL; //oops
}

// getctydata.c
int getctydata(char *checkcallptr) {
    return 0;
}
/*********************/

int setup_default(void **state) {
    msg = msg1 = msg2 = NOT_SET;
    arrlss = 0;
    dxped = 0;
    contest = 0;
    search_win = NULL;
    return 0;
}

// callmaster is checked first in current directory,
// create it there
static void write_callmaster(const char *content) {
    FILE *f = fopen("callmaster", "w");
    assert_non_null(f);
    fputs(content, f);
    fclose(f);
}

static void remove_callmaster() {
    unlink("callmaster");   // no need to check if file exists
}

int teardown_default(void **state) {
    remove_callmaster();
    return 0;
}

void test_callmaster_no_file(void **state) {
    remove_callmaster();
    int n = load_callmaster();
    assert_int_equal(n, 0);
    assert_string_equal(msg, NOT_SET);
    assert_string_equal(msg1, NOT_SET);
    assert_string_equal(msg2, NOT_SET);
}

void test_callmaster_ok(void **state) {
    write_callmaster("# data\nA1AA\nA2BB\n\n");
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "A2BB");
}

void test_callmaster_ok_dos(void **state) {
    write_callmaster("# data\r\nA1AA\r\nA2BB\r\n\r\n");
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "A2BB");
}

void test_callmaster_ok_spaces(void **state) {
    write_callmaster(" # data \n A1AA \n A2BB \n\n");
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "A2BB");
}

void test_callmaster_ok_arrlss(void **state) {
    write_callmaster("# data\nA1AA\nG0CC\nN2BB\n\n");
    arrlss = 1;
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "N2BB");
}

void test_init_search_panel_no_contest(void **state) {
    InitSearchPanel();
    assert_int_equal(nr_bands, 9);
}

void test_init_search_panel_contest(void **state) {
    contest = 1;
    InitSearchPanel();
    assert_int_equal(nr_bands, 6);
}

void test_init_search_panel_dxped(void **state) {
    dxped = 1;
    InitSearchPanel();
    assert_int_equal(nr_bands, 9);
}

