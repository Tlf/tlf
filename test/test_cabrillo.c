#include "test.h"

#include "../src/cabrillo_utils.h"
#include "../src/readcabrillo.h"

// OBJECT ../src/cabrillo_utils.o
// OBJECT ../src/readcabrillo.o
// OBJECT ../src/bands.o

/* test stubs and dummies */
int do_cabrillo = 0;	/* actually converting cabrillo file to Tlf log */
struct tm *time_ptr, time_ptr_cabrillo;

int qsoflags_for_qtc[MAX_QSOS];
float freq;

void addcall() { }
void store_qso() { }
void cleanup_qso() { }
void make_qtc_logline(struct read_qtc_t qtc_line, char *fname) { }
char *getgrid(char *comment) { return comment; }
void checkexchange(int x) { }

/* some spies */
int bandinx_spy;

void makelogline() {
    bandinx_spy = bandinx;
}

char formatfile[100];

int setup(void **state) {
    strcpy(formatfile, TOP_SRCDIR);
    strcat(formatfile, "/share/cabrillo.fmt");
    return 0;
}

/* export non public protoypes for test */
int starts_with(char *line, char *start);
void cab_qso_to_tlf(char *line, struct cabrillo_desc *cabdesc);

/* Test of helper functions */
void test_starts_with_succeed(void **state) {
    char line[] = "Test string";

    assert_int_equal(starts_with(line, "Test"), 1);
}

void test_starts_with_fails(void **state) {
    char line[] = "Test string";

    assert_int_equal(starts_with(line, "Tes "), 0);
}


/* tests for cabrillo_utils.c */
void test_translateItem(void **state) {
    assert_int_equal(translate_item_name(""), NO_ITEM);
    assert_int_equal(translate_item_name("NONSENSE"), NO_ITEM);
    assert_int_equal(translate_item_name("FREQ"), FREQ);
    assert_int_equal(translate_item_name("QTC"), QTC);
}

void test_freeCabfmt(void **state) {
    /* please test with valgrind that all memory is correctly given back */

    // no desc given
    struct cabrillo_desc *desc = NULL;
    free_cabfmt(desc);

    // empty desc given
    desc = g_new0(struct cabrillo_desc, 1);
    free_cabfmt(desc);

    // only QSO entries
    desc = g_new0(struct cabrillo_desc, 1);
    desc->name = strdup("hallo");
    desc->item_array = g_ptr_array_new();
    desc->item_count = 0;
    free_cabfmt(desc);
    //
    // QSO and QTC entries
    desc = g_new0(struct cabrillo_desc, 1);
    desc->name = strdup("hallo");
    desc->item_array = g_ptr_array_new();
    desc->item_count = 0;
    desc->qtc_item_array = g_ptr_array_new();
    desc->qtc_item_count = 0;
    free_cabfmt(desc);
}

void test_parseLine(void **state) {
    struct line_item *line;

    line = parse_line_entry("");
    assert_int_equal(line->tag, NO_ITEM);
    g_free(line);

    line = parse_line_entry("hallo");
    assert_int_equal(line->tag, NO_ITEM);
    g_free(line);

    line = parse_line_entry("DATE,10");
    assert_int_equal(line->tag, DATE);
    assert_int_equal(line->len, 10);
    g_free(line);
}

void test_readCabrilloFormatUniversal(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    assert_non_null(desc);
    assert_string_equal(desc->name, "UNIVERSAL");
    assert_int_equal(desc->item_count, 10);
    assert_non_null(desc->item_array);
    assert_int_equal(desc->qtc_item_count, 0);
    assert_null(desc->qtc_item_array);
    free_cabfmt(desc);
}

void test_readCabrilloFormatWAE(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "WAEDC");
    assert_non_null(desc);
    assert_string_equal(desc->name, "WAEDC");
    assert_int_equal(desc->item_count, 10);
    assert_non_null(desc->item_array);
    assert_int_equal(desc->qtc_item_count, 8);
    assert_non_null(desc->qtc_item_array);
    free_cabfmt(desc);
}

void test_readCabrilloFileNotFound(void **state) {
    struct cabrillo_desc *desc;
    char formatfile1[100];
    strcpy(formatfile1, TOP_SRCDIR);
    strcat(formatfile1, "/share/cabrillo1.fmt");
    desc = read_cabrillo_format(formatfile1, "WAEDC");
    assert_null(desc);
}

void test_readCabrilloFormatNotFound(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "NOT_IN_FILE");
    assert_null(desc);
}


/* tests for readcabrillo */
void test_cabToTlf_ParseQSO(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    bandinx_spy = 0;
    cab_qso_to_tlf("QSO:  7002 RY 2016-08-13 0033 HA2OS         589 0008   K6ND          599 044",
		   desc);
    assert_int_equal(bandinx_spy, 2);
    assert_int_equal((int)freq, 7002);
    assert_int_equal(trxmode, DIGIMODE);
    assert_string_equal(hiscall, "K6ND");
    assert_string_equal(my_rst, "589");
    assert_string_equal(his_rst, "599");
    assert_string_equal(comment, "044");
    assert_int_equal(time_ptr_cabrillo.tm_hour, 00);
    assert_int_equal(time_ptr_cabrillo.tm_min, 33);
    assert_int_equal(time_ptr_cabrillo.tm_year, 2016 - 1900); /* year-1900 */
    assert_int_equal(time_ptr_cabrillo.tm_mon, 8 - 1);	  /* 0-11 */
    assert_int_equal(time_ptr_cabrillo.tm_mday, 13);
}

#if 0
static void test_cabToTlf_KeepUnrelated(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format("../share/cabrillo.fmt", "UNIVERSAL");

    bandinx = 0;
    strcpy(qsonrstr, "1234");
    qsonum = 1234;

    cab_qso_to_tlf("QSO:  7002 RY 2016-08-13 0033 HA2OS         589 0008   K6ND          599 044",
		   desc);

    assert_string_equal(qsonrstr, "1234");
    assert_int_equal(qsonum, 1234);
    assert_int_equal(bandinx, 0);
}
#endif


