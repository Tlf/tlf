#include "test.h"

#include "../src/dxcc.h"
#include "../src/globalvars.h"
#include "../src/setcontest.h"



// OBJECT ../src/dxcc.o
// OBJECT ../src/setcontest.o

/* dummys */
int getctynr() {
    return 42;
}

void foc_init() {
}

int setup_default(void **state) {
    char filename[100];

    strcpy(filename, TOP_SRCDIR);
    strcat(filename, "/share/cty.dat");
    assert_int_equal(load_ctydata(filename), 0);

    contest = NULL;

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
