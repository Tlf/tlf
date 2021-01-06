#include "test.h"

#include "../src/cabrillo_utils.h"
#include "../src/readcabrillo.h"
#include "../src/cqww_simulator.h"

// OBJECT ../src/cabrillo_utils.o
// OBJECT ../src/readcabrillo.o
// OBJECT ../src/writecabrillo.o
// OBJECT ../src/bands.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/get_time.o
// OBJECT ../src/sendbuf.o
// OBJECT ../src/log_utils.o

/* test stubs and dummies */
bool simulator = false;

int do_cabrillo = 0;	/* actually converting cabrillo file to Tlf log */
struct tm time_ptr_cabrillo;

int qsoflags_for_qtc[MAX_QSOS];

void addcall() { }
void store_qso() { }
void cleanup_qso() { }
void make_qtc_logline(struct read_qtc_t qtc_line, char *fname) { }
char *getgrid(char *comment) { return comment; }
void checkexchange(int x) { }

int get_total_score() {
    return 123;
}

void ask(char *buffer, char *what) {
}

void play_file(char *audiofile) {
}

bool lan_active = false;

int send_lan_message(int opcode, char *message) {
    return 0;
}

simstate_t get_simulator_state() {
    return IDLE;
}

void set_simulator_state(simstate_t s) {
}

void keyer_append(const char *string) {
}

int modify_attr(int attr) { // FIXME: remove once info() moved to UI code
    return 0;
}

/* some spies */
int bandinx_spy;

void makelogline() {
    bandinx_spy = bandinx;
}

char *error_details;

char formatfile[] = TOP_SRCDIR "/share/cabrillo.fmt" ;

int setup(void **state) {
    strcpy(my.call, "A1BCD");
    strcpy(exchange, "#");
    return 0;
}

/* export non public protoypes for test */
int starts_with(char *line, char *start);
void cab_qso_to_tlf(char *line, struct cabrillo_desc *cabdesc);
extern struct read_qtc_t qtc_line;	/* make global for testability */
gchar *get_nth_token(gchar *str, int n, const char *separator);
void prepare_line(struct qso_t *qso, struct cabrillo_desc *desc, char *buf);

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
    assert_null(desc->exchange_separator);
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

    static char formatfile1[] = TOP_SRCDIR "/share/cabrillo1.fmt";
    desc = read_cabrillo_format(formatfile1, "WAEDC");

    assert_null(desc);
}

void test_readCabrilloFormatNotFound(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "NOT_IN_FILE");
    assert_null(desc);
}

void test_prepare_line_universal(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    assert_non_null(desc);

    struct qso_t qso = {
	.year = 2021, .month = 1, .day = 2, .hour = 8, .min = 42,
	.qso_nr = 711, .mode = CWMODE,
	.call = "A2XYZ", .freq = 21012845.6,
	.rst_s = 599, .rst_r = 559,
	.comment = "007"
    };

    char buf[200];
    prepare_line(&qso, desc, buf);

    assert_string_equal(buf,
			"QSO: 21012 CW 2021-01-02 0842 A1BCD         599 0711   A2XYZ         559 007   \n");
}

void test_prepare_line_agcw(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "AGCW");
    assert_non_null(desc);
    assert_string_equal(desc->exchange_separator, "/");

    struct qso_t qso = {
	.year = 2021, .month = 1, .day = 2, .hour = 8, .min = 42,
	.qso_nr = 711, .mode = CWMODE,
	.call = "A2XYZ", .freq = 21012845.6,
	.rst_s = 599, .rst_r = 559,
	.comment = "007/1234"
    };

    char buf[200];
    prepare_line(&qso, desc, buf);

    assert_string_equal(buf,
			"QSO: 21012 CW 2021-01-02 0842 A1BCD         599 0711      A2XYZ         559 007  1234\n");
}

void test_prepare_line_agcw3(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "AGCW3");
    assert_non_null(desc);
    assert_string_equal(desc->exchange_separator, " /");

    struct qso_t qso = {
	.year = 2021, .month = 1, .day = 2, .hour = 8, .min = 42,
	.qso_nr = 711, .mode = CWMODE,
	.call = "A2XYZ", .freq = 21012845.6,
	.rst_s = 599, .rst_r = 559,
	.comment = "007 HANS/1234"  // mixed separators
    };

    char buf[200];
    prepare_line(&qso, desc, buf);

    assert_string_equal(buf,
			"QSO: 21012 CW 2021-01-02 0842 A1BCD         599 0711      A2XYZ         559 007  HANS 1234\n");
}


void test_get_nth_token(void **state) {
    char *str = "ab  cDe\tf";
    char *token;

    token = get_nth_token(str, 0, NULL);
    assert_non_null(token);
    assert_string_equal(token, "ab");

    token = get_nth_token(str, 1, NULL);
    assert_non_null(token);
    assert_string_equal(token, "cDe");

    token = get_nth_token(str, 2, NULL);
    assert_non_null(token);
    assert_string_equal(token, "f");
}

void test_get_nth_token_slash(void **state) {
    char *str = "ab/cDe/f";
    char *separator = "/";
    char *token;

    token = get_nth_token(str, 0, separator);
    assert_non_null(token);
    assert_string_equal(token, "ab");

    token = get_nth_token(str, 1, separator);
    assert_non_null(token);
    assert_string_equal(token, "cDe");

    token = get_nth_token(str, 2, separator);
    assert_non_null(token);
    assert_string_equal(token, "f");
}


/* tests for readcabrillo */
void test_cabToTlf_ParseQSO(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    bandinx_spy = 0;
    cab_qso_to_tlf("QSO:  7002 RY 2016-02-13 2033 HA2OS         589 0008   K6ND          599 044",
		   desc);
    assert_int_equal(bandinx_spy, 3);
    assert_int_equal((int)freq, 7002000);
    assert_int_equal(trxmode, DIGIMODE);
    assert_string_equal(hiscall, "K6ND");
    assert_string_equal(recvd_rst, "589");
    assert_string_equal(sent_rst, "599");
    assert_string_equal(comment, "044");
    assert_int_equal(time_ptr_cabrillo.tm_hour, 20);
    assert_int_equal(time_ptr_cabrillo.tm_min, 33);
    assert_int_equal(time_ptr_cabrillo.tm_year, 2016 - 1900); /* year-1900 */
    assert_int_equal(time_ptr_cabrillo.tm_mon, 2 - 1);	  /* 0-11 */
    assert_int_equal(time_ptr_cabrillo.tm_mday, 13);
}

void test_cabToTlf_ParseQSO_agcw(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "AGCW");
    bandinx_spy = 0;
    cab_qso_to_tlf("QSO:  7002 RY 2016-02-13 2033 HA2OS         589 0008      K6ND          599 044  ED",
		   desc);
    assert_int_equal(bandinx_spy, 3);
    assert_int_equal((int)freq, 7002000);
    assert_int_equal(trxmode, DIGIMODE);
    assert_string_equal(hiscall, "K6ND");
    assert_string_equal(recvd_rst, "589");
    assert_string_equal(sent_rst, "599");
    assert_string_equal(comment, "044/ED");
    assert_int_equal(time_ptr_cabrillo.tm_hour, 20);
    assert_int_equal(time_ptr_cabrillo.tm_min, 33);
    assert_int_equal(time_ptr_cabrillo.tm_year, 2016 - 1900); /* year-1900 */
    assert_int_equal(time_ptr_cabrillo.tm_mon, 2 - 1);	  /* 0-11 */
    assert_int_equal(time_ptr_cabrillo.tm_mday, 13);
}

void test_cabToTlf_ParseXQSO(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    bandinx_spy = 0;
    cab_qso_to_tlf("X-QSO:  7002 PH 2016-08-13 0033 HA2OS         589 0008   K6ND          599 044",
		   desc);
    assert_int_equal(bandinx_spy, 3);
    assert_int_equal((int)freq, 7002000);
    assert_int_equal(trxmode, SSBMODE);
    assert_string_equal(hiscall, "K6ND");
    assert_string_equal(recvd_rst, "589");
    assert_string_equal(sent_rst, "599");
    assert_string_equal(comment, "044");
    assert_int_equal(time_ptr_cabrillo.tm_hour, 00);
    assert_int_equal(time_ptr_cabrillo.tm_min, 33);
    assert_int_equal(time_ptr_cabrillo.tm_year, 2016 - 1900); /* year-1900 */
    assert_int_equal(time_ptr_cabrillo.tm_mon, 8 - 1);	  /* 0-11 */
    assert_int_equal(time_ptr_cabrillo.tm_mday, 13);
}

void test_cabToTlf_ParseQTC(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "WAEDC");
    qtcdirection = SEND;
    bandinx_spy = 0;
    cab_qso_to_tlf("QTC: 14084 CW 2016-11-12 1214 HA2OS          13/10     K4GM          0230 DL6UHD         074",
		   desc);
    assert_int_equal((int)qtc_line.freq, 14084000);
    assert_string_equal(qtc_line.mode, "CW ");
    assert_int_equal(time_ptr_cabrillo.tm_hour, 12);
    assert_int_equal(time_ptr_cabrillo.tm_min, 14);
    assert_int_equal(time_ptr_cabrillo.tm_year, 2016 - 1900); /* year-1900 */
    assert_int_equal(time_ptr_cabrillo.tm_mon, 11 - 1);	  /* 0-11 */
    assert_int_equal(time_ptr_cabrillo.tm_mday, 12);
    assert_string_equal(qtc_line.call, "K4GM");
    assert_int_equal(qtc_line.qtchead_serial, 13);
    assert_int_equal(qtc_line.qtchead_count, 10);
    assert_string_equal(qtc_line.qtc_time, "0230");
    assert_string_equal(qtc_line.qtc_call, "DL6UHD");
    assert_int_equal(qtc_line.qtc_serial, 74);
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


