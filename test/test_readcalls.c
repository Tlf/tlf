#include "test.h"

#include <stdbool.h>

#include "../src/tlf.h"
#include "../src/dxcc.h"
#include "../src/readctydata.h"
#include "../src/globalvars.h"
#include "../src/getctydata.h"
#include "../src/get_time.h"
#include "../src/readcalls.h"
#include "../src/setcontest.h"

// OBJECT ../src/log_utils.o
// OBJECT ../src/addcall.o
// OBJECT ../src/addmult.o
// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/get_time.o
// OBJECT ../src/locator2longlat.o
// OBJECT ../src/readcalls.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/score.o
// OBJECT ../src/utils.o
// OBJECT ../src/zone_nr.o

/* missing from globalvar.h */

// dummy functions
void readqtccalls() {}
void shownr(char *msg, int x) {}
void spaces(int n) {} /* needs more care */

int qrb(double a, double b, double c, double d) {
    return 1;
}

int foc_score(char *a) {
    return 1;
}

contest_config_t config_focm;

int pacc_pa(void) {
    return 0;
}

/* private prototypes */
bool check_veto();

#define QSO1 " 40SSB 12-Jan-18 16:34 0006  PY9BBB         599  599  15                    10         \n"

#define LOGFILE "test.log"

void write_log(char *logfile) {
    FILE *fp = fopen(logfile, "w");
    assert_non_null(fp);

    fputs(QSO1,fp);

    fclose(fp);
}


int setup_default(void **state) {

    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    setcontest("CQWW");

    strcpy(countrylist[0], "DL");
    strcpy(countrylist[1], "CE");
    strcpy(countrylist[2], "");

    strcpy(continent_multiplier_list[0], "EU");
    strcpy(continent_multiplier_list[1], "NA");
    strcpy(continent_multiplier_list[2], "");

    exclude_multilist_type = EXCLUDE_NONE;
    continentlist_only = false;

    memset(pfxnummulti, 0, sizeof(pfxnummulti));
    pfxnummulti[0].countrynr = 12;
    pfxnummulti[1].countrynr = 42;
    pfxnummultinr = 2;

    return 0;
}

int teardown_default(void **state) {
    unlink(logfile);
    return 0;
}


/* test lookup country in pfxnummult */
void test_lookup_not_in_pfxnummult(void **state) {
    assert_int_equal(lookup_country_in_pfxnummult_array(1), -1);
}


void test_lookup_in_pfxnummult(void **state) {
    assert_int_equal(lookup_country_in_pfxnummult_array(42), 1);
}


/* test readcalls */
void test_add_to_worked(void **state) {
    write_log(LOGFILE);
    readcalls(LOGFILE);
    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].call, "PY9BBB");
    assert_string_equal(worked[0].exchange, "15");
    time_t ts = parse_time(QSO1+7, DATE_TIME_FORMAT);
    assert_int_equal(worked[0].qsotime[SSBMODE][BANDINDEX_40], ts);
}

void test_add_to_worked_continentlistonly(void **state) {
    continentlist_only = true;
    write_log(LOGFILE);
    readcalls(LOGFILE);
    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].exchange, "15");
}
