#include "test.h"

#include "../src/cabrillo_utils.h"
#include "../src/dxcc.h"
#include "../src/readcabrillo.h"
#include "../src/cqww_simulator.h"
#include "../src/log_utils.h"

// OBJECT ../src/cabrillo_utils.o
// OBJECT ../src/readcabrillo.o
// OBJECT ../src/writecabrillo.o
// OBJECT ../src/bands.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/get_time.o
// OBJECT ../src/sendbuf.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/utils.o

/* test stubs and dummies */
bool simulator = false;

char section[8] = "";       // defined in getexchange.c

int qsoflags_for_qtc[MAX_QSOS];

void addcall(struct qso_t *qso) { }
void store_qso(char *logline) { nr_qsos++; }
void cleanup_qso() { }
void make_qtc_logline(struct read_qtc_t qtc_line, char *fname) { }
char *getgrid(char *comment) { return comment; }
void checkexchange(int x) { }
void add_to_keyer_terminal(char *buffer) {}

int get_total_score() {
    return 123;
}

void score_qso(struct qso_t *qso) {
}

void ask(char *buffer, char *what) {
}

void vk_play_file(char *audiofile) {
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

int getctynr(char *call) {
    return 42;
}

prefix_data *getctyinfo(char *call) {
    return NULL;
}

/* some spies */
struct qso_t *qso_spy = NULL;

char *makelogline(struct qso_t *qso) {
    qso_spy = g_malloc0(sizeof(struct qso_t));
    memcpy(qso_spy, qso, sizeof(struct qso_t));
    // make private copies of string members
    qso_spy->call = g_strdup(qso->call);
    qso_spy->comment = g_strdup(qso->comment);
    qso_spy->logline = g_strdup(qso->logline);
    return g_strdup("dummy");
}

char *error_details;

char formatfile[] = TOP_SRCDIR "/share/cabrillo.fmt" ;

int setup_default(void **state) {
    strcpy(my.call, "A1BCD");
    strcpy(exchange, "#");

    init_qso_array();
    nr_qsos = 0;

    return 0;
}

int teardown_default(void **state) {
    free_qso_array();

    g_free(qso_spy);
    qso_spy = NULL;
    return 0;
}


static char datetime_buf[80];
static char *get_datetime(struct qso_t *qso) {
    strftime(datetime_buf, sizeof(datetime_buf), DATE_TIME_FORMAT, gmtime(&qso->timestamp));
    return datetime_buf;    // note: returns a static buffer
}

/* export non public prototypes for test */
int starts_with(char *line, char *start);
void cab_qso_to_tlf(char *line, struct cabrillo_desc *cabdesc);
extern struct read_qtc_t qtc_line;	/* make global for testability */
gchar *get_nth_token(gchar *str, int n, const char *separator);
void prepare_line(struct linedata_t *qso,
		  struct cabrillo_desc *desc, char *buf);

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

    struct linedata_t qso = {
	.year = 2021, .month = 1, .day = 2, .hour = 8, .min = 42,
	.qso_nr = 711, .mode = CWMODE,
	.call = "A2XYZ", .freq = 21012845.6,
	.rst_s = 599, .rst_r = 559,
	.comment = "007"
    };

    char buf[200];
    prepare_line(&qso, desc, buf);
    free_cabfmt(desc);

    assert_string_equal(buf,
			"QSO: 21012 CW 2021-01-02 0842 A1BCD         599 0711   A2XYZ         559 007   \n");
}

void test_prepare_line_agcw(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "AGCW");
    assert_non_null(desc);
    assert_string_equal(desc->exchange_separator, "/");

    struct linedata_t qso = {
	.year = 2021, .month = 1, .day = 2, .hour = 8, .min = 42,
	.qso_nr = 711, .mode = CWMODE,
	.call = "A2XYZ", .freq = 21012845.6,
	.rst_s = 599, .rst_r = 559,
	.comment = "007/1234"
    };

    char buf[200];
    prepare_line(&qso, desc, buf);
    free_cabfmt(desc);

    assert_string_equal(buf,
			"QSO: 21012 CW 2021-01-02 0842 A1BCD         599 0711      A2XYZ         559 007  1234\n");
}

void test_prepare_line_agcw3(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "AGCW3");
    assert_non_null(desc);
    assert_string_equal(desc->exchange_separator, " /");

    struct linedata_t qso = {
	.year = 2021, .month = 1, .day = 2, .hour = 8, .min = 42,
	.qso_nr = 711, .mode = CWMODE,
	.call = "A2XYZ", .freq = 21012845.6,
	.rst_s = 599, .rst_r = 559,
	.comment = "007 HANS/1234"  // mixed separators
    };

    char buf[200];
    prepare_line(&qso, desc, buf);
    free_cabfmt(desc);

    assert_string_equal(buf,
			"QSO: 21012 CW 2021-01-02 0842 A1BCD         599 0711      A2XYZ         559 007  HANS 1234\n");
}


void test_get_nth_token(void **state) {
    char *str = "ab  cDe\tf";
    char *token;

    token = get_nth_token(str, 0, NULL);
    assert_non_null(token);
    assert_string_equal(token, "ab");
    g_free(token);

    token = get_nth_token(str, 1, NULL);
    assert_non_null(token);
    assert_string_equal(token, "cDe");
    g_free(token);

    token = get_nth_token(str, 2, NULL);
    assert_non_null(token);
    assert_string_equal(token, "f");
    g_free(token);
}

void test_get_nth_token_slash(void **state) {
    char *str = "ab/cDe/f";
    char *separator = "/";
    char *token;

    token = get_nth_token(str, 0, separator);
    assert_non_null(token);
    assert_string_equal(token, "ab");
    g_free(token);

    token = get_nth_token(str, 1, separator);
    assert_non_null(token);
    assert_string_equal(token, "cDe");
    g_free(token);

    token = get_nth_token(str, 2, separator);
    assert_non_null(token);
    assert_string_equal(token, "f");
    g_free(token);
}


/* tests for readcabrillo */
void test_cabToTlf_ParseQSO(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    cab_qso_to_tlf("QSO:  7002 RY 2016-02-13 2033 HA2OS         589 0008   K6ND          599 044",
		   desc);
    free_cabfmt(desc);
    assert_non_null(qso_spy);
    assert_int_equal(qso_spy->bandindex, 3);
    assert_int_equal((int)qso_spy->freq, 7002000);
    assert_int_equal(qso_spy->mode, DIGIMODE);
    assert_string_equal(qso_spy->call, "K6ND");
    assert_int_equal(qso_spy->rst_s, 589);
    assert_int_equal(qso_spy->rst_r, 599);
    assert_string_equal(qso_spy->comment, "044");
    assert_string_equal(get_datetime(qso_spy), "13-Feb-16 20:33");
}

void test_cabToTlf_ParseQSO_agcw(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "AGCW");
    cab_qso_to_tlf("QSO:  7002 RY 2016-02-13 2033 HA2OS         589 0008      K6ND          599 044  ED",
		   desc);
    free_cabfmt(desc);
    assert_non_null(qso_spy);
    assert_int_equal(qso_spy->bandindex, 3);
    assert_int_equal((int)qso_spy->freq, 7002000);
    assert_int_equal(qso_spy->mode, DIGIMODE);
    assert_string_equal(qso_spy->call, "K6ND");
    assert_int_equal(qso_spy->rst_s, 589);
    assert_int_equal(qso_spy->rst_r, 599);
    assert_string_equal(qso_spy->comment, "044/ED");
    assert_string_equal(get_datetime(qso_spy), "13-Feb-16 20:33");
}

void test_cabToTlf_ParseXQSO(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "UNIVERSAL");
    cab_qso_to_tlf("X-QSO: 14002 PH 2016-08-13 0033 HA2OS         589 0008   K6ND          599 044",
		   desc);
    free_cabfmt(desc);
    assert_non_null(qso_spy);
    assert_int_equal(qso_spy->bandindex, 5);
    assert_int_equal((int)qso_spy->freq, 14002000);
    assert_int_equal(qso_spy->mode, SSBMODE);
    assert_string_equal(qso_spy->call, "K6ND");
    assert_int_equal(qso_spy->rst_s, 589);
    assert_int_equal(qso_spy->rst_r, 599);
    assert_string_equal(qso_spy->comment, "044");
    assert_string_equal(get_datetime(qso_spy), "13-Aug-16 00:33");
}

void test_cabToTlf_ParseQTC(void **state) {
    struct cabrillo_desc *desc;
    desc = read_cabrillo_format(formatfile, "WAEDC");
    qtcdirection = SEND;
    cab_qso_to_tlf("QTC: 14084 CW 2016-11-12 1214 HA2OS          13/10     K4GM          0230 DL6UHD         074",
		   desc);
    free_cabfmt(desc);
    assert_int_equal((int)qtc_line.freq, 14084000);
    assert_string_equal(qtc_line.date, "12-Nov-16");
    assert_string_equal(qtc_line.time, "12:14");
    assert_string_equal(qtc_line.mode, "CW ");
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


