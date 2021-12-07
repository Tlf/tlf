#include <inttypes.h>
#include "test.h"

#include "../src/tlf.h"
#include "../src/dxcc.h"
#include "../src/readctydata.h"
#include "../src/globalvars.h"
#include "../src/setcontest.h"

#include "../src/getctydata.h"

// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/score.o
// OBJECT ../src/plugin.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/utils.o

/* export internal function */
int location_unknown(char *call);
int getpfxindex(char *checkcallptr, char **normalized_call);
int find_full_match(const char *call);
int find_best_match(const char *call);

extern char countrylist[255][6];

contest_config_t config_focm;

int setup_default(void **state) {

    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    setcontest("qso");
    pfxmult = false;
    strcpy(countrylist[0], "");

    return 0;
}

char *best_prefix(char *call) {
    prefix_data *pfx;
    int index;

    index = find_best_match(call);
    if (index < 0)
	return "";

    pfx = prefix_by_index(index);
    return pfx->pfx;
}

char *full_prefix(char *call) {
    prefix_data *pfx;
    int index;

    index = find_full_match(call);
    if (index < 0)
	return "";

    pfx = prefix_by_index(index);
    return pfx->pfx;
}

#undef TEST_LOOKUP_SPEED

void test_xspeed(void **state) {
#ifndef TEST_LOOKUP_SPEED
    assert_true(1);
#else
    static char callmaster[] =  TOP_SRCDIR "/share/callmaster";
    char call[36000][16];

    FILE *f = fopen(callmaster, "r");
    char line[99];
    int n = 0;
    while (fgets(line, sizeof(line), f)) {
	if (line[0] == '#' || strlen(line) < 3) {
	    continue;
	}
	for (int i = 0; ; ++i) {
	    if (line[i] <= ' ') {
		call[n][i] = 0;
		++n;
		break;
	    }
	    call[n][i] = line[i];
	}
    }
    fclose(f);

    printf("got %d calls from %s\n", n, callmaster);
    //printf("0: |%s|\n", call[0]);
    //printf("5: |%s|\n", call[5]);


    const int N =  20000000;
    srand48(time(NULL));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    for (int i = 0; i < N; ++i) {
	int j = (int)(n * drand48());
	int w = find_best_match(call[j]);
	w = w;				/* quell warning about unused var */
#if 0
	printf("%d -> %s: %d\n", j, call[j], w);
	prefix_data *pfx = prefix_by_index(w);
	dxcc_data *dx = dxcc_by_index(pfx->dxcc_index);
	printf("--> %s\n", dx->countryname);
#endif
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 +
			(end.tv_nsec - start.tv_nsec) / 1000;

    printf("took %" PRIu64 " us, rate is %g us/call\n",
	   delta_us, (double)delta_us / N);
#endif
}

void test_full_match(void **state) {
    assert_string_equal(full_prefix("DL1XXX"), "");
    assert_string_equal(full_prefix("4U1UN"), "4U1UN");
    assert_string_equal(full_prefix("4U1U"), "");
}

void test_best_match(void **state) {
    assert_string_equal(best_prefix("DL1XXX"), "DL");
    assert_string_equal(best_prefix("4U1UN"), "4U1UN");
    assert_string_equal(best_prefix("4U1UNA"), "");;
    assert_string_equal(best_prefix("4U1U"), "");;
    assert_string_equal(best_prefix("EA3XYZ"), "EA");
    assert_string_equal(best_prefix("EA8XYZ"), "EA8");
    assert_string_equal(best_prefix("W3A"), "W");
    assert_string_equal(best_prefix("KL7ND"), "KL");
}

void test_location_known(void **state) {
    assert_int_equal(location_unknown("LA3BB"), 0);
    assert_int_equal(location_unknown("LA3BB/P"), 0);
}

void test_location_unknown(void **state) {
    assert_int_equal(location_unknown("LA3BB/MM"), 1);
    assert_int_equal(location_unknown("LA3BB/AM"), 1);
}

void test_suffix_empty(void **state) {
    assert_int_equal(getpfxindex(NULL, NULL), -1);
    assert_int_equal(getpfxindex("", NULL), -1);
    assert_int_equal(getctynr(""), 0);
    assert_int_equal(getctydata(""), 0);
}

void test_location_unknown_used(void **state) {
    assert_int_equal(getpfxindex("LA3BB/AM", NULL), -1);
    assert_int_equal(getctynr("LA3BB/AM"), 0);
    assert_int_equal(getctydata("LA3BB/AM"), 0);
}

/* getctynr */
void test_suffix_getctynr(void **state) {
    int cty_la = getctynr("LA3BB");
    assert_int_not_equal(cty_la, 0);
    assert_int_equal(getctynr("LA3BB/QRP"), cty_la);
    assert_int_equal(getctynr("LA3BB/P"), cty_la);
}

/* getctydata */
void test_suffix_getctydata(void **state) {
    int cty_la = getctydata("LA3BB");
    assert_int_not_equal(cty_la, 0);
    assert_int_equal(getctydata("LA3BB/QRP"), cty_la);
    assert_int_equal(getctydata("LA3BB/P"), cty_la);
}

void test_someidea(void **data) {
    assert_int_not_equal(getpfxindex("DJ0LN/A", NULL), -1);
    assert_int_not_equal(getpfxindex("PA/DJ0LN/P", NULL), -1);
    assert_int_not_equal(getpfxindex("DJ0LN/P", NULL), -1);
    assert_int_not_equal(getpfxindex("DJ0LN/PA/P", NULL), -1);
    assert_int_not_equal(getpfxindex("K32A/4", NULL), -1);
    assert_int_not_equal(getpfxindex("R3A/PA", NULL), -1);
    assert_int_not_equal(getpfxindex("DJ/PA3LM", NULL), -1);
}

#define check(x) (assert_int_equal(getctynr(x), getctydata(x)))

void test_same_result(void **data) {
    check("DJ0LN/A");
    check("PA/DJ0LN/P");
    check("DJ0LN/P");
    check("DJ0LN/PA/P");
    check("K32A/4");
    check("R3A/PA");
    check("DJ/PA3LM");
    check("9A70DP/KA");
}

void test_no_wpx(void **state) {
    int nr;
    wpx_prefix[0] = '\0';
    nr = getctydata("DJ/PA3LM");
    assert_string_equal(wpx_prefix, "");
    assert_int_equal(getctydata("DL"), nr);
}

void test_is_wpx(void **state) {
    int nr;

    setcontest("wpx");
    wpx_prefix[0] = '\0';
    nr = getctydata("DJ/PA3LM");
    assert_string_equal(wpx_prefix, "DJ0");
    assert_int_equal(getctydata("DL"), nr);
}

void test_pfxmult_set(void **state) {
    int nr;

    pfxmult = true;
    wpx_prefix[0] = '\0';
    nr = getctydata("DJ/PA3LM");
    assert_string_equal(wpx_prefix, "DJ0");
    assert_int_equal(getctydata("DL"), nr);
}

void test_abnormal_call(void **state) {
    int nr;
    nr = getctydata("ET3AA/YOTA");
    assert_int_equal(getctydata("ET"), nr);
}


