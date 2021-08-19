#include "test.h"

#include "../src/dxcc.h"
#include "../src/globalvars.h"
#include "../src/setcontest.h"


// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/getpx.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/score.o
// OBJECT ../src/plugin.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/utils.o

/* dummys */
int getctynr(char *checkcall) {
    return 42;
}

int getctydata(char *checkcall) {
    return 0;
}

contest_config_t config_focm;

int setup_default(void **state) {
    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    contest = NULL;
    strcpy(whichcontest, "");

    return 0;
}

void test_lookup_contest(void **state) {
    contest = lookup_contest("CQWW");
    assert_int_equal(contest->id, CQWW);
}

void test_lookup_contest_ignore_case(void **state) {
    contest = lookup_contest("cqww");
    assert_int_equal(contest->id, CQWW);
}

void test_lookup_contest_not_found(void **state){
    contest = lookup_contest("A23b9");
    assert_int_equal(contest->id, UNKNOWN);
}

void test_lookup_contest_ignore_incomplete(void **state) {
    config_qso.name = NULL;
    contest = lookup_contest("A23b9");
    assert_int_equal(contest->id, UNKNOWN);
}

void test_set_whichcontest(void **state) {
    setcontest("Hello");
    assert_string_equal(whichcontest, "Hello");
}
