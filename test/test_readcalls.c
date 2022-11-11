#include "test.h"

#include <stdbool.h>

#include "../src/tlf.h"
#include "../src/dxcc.h"
#include "../src/readctydata.h"
#include "../src/globalvars.h"
#include "../src/getctydata.h"
#include "../src/get_time.h"
#include "../src/log_utils.h"
#include "../src/readcalls.h"
#include "../src/setcontest.h"
#include "../src/showscore.h"

// OBJECT ../src/log_utils.o
// OBJECT ../src/addcall.o
// OBJECT ../src/addmult.o
// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getexchange.o
// OBJECT ../src/getpx.o
// OBJECT ../src/get_time.o
// OBJECT ../src/readcalls.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/score.o
// OBJECT ../src/showscore.o
// OBJECT ../src/utils.o
// OBJECT ../src/zone_nr.o

// OBJECT ../src/makelogline.o
// OBJECT ../src/qsonr_to_str.o
// OBJECT ../src/store_qso.o
// OBJECT ../src/ui_utils.o

typedef void (*ExpandMacro_t) (void);

char thisnode = 'A';
bool lan_active = false;

// dummy functions
void readqtccalls() {}
void shownr(char *msg, int x) {}

void clusterinfo(void) {}
void clear_display(void) {}
void refresh_splitlayout() {}

void OnLowerSearchPanel(int x, char *str) {}
int recall_exchange() { return -1; }
void refresh_comment() {}
void time_update() {}
void show_rtty() {}
void keyer() {}
void send_standard_message(int msg) {}
void sendmessage_with_macro_expand(const char *msg, ExpandMacro_t expandMacro) {}
void send_standard_message_with_macro_expand(int msg, ExpandMacro_t expandMacro) {}
void ExpandMacro_PreviousQso(void) {}
void ExpandMacro_CurrentQso(void) {}
void stoptx() {}
void qtc_main_panel(int direction) {}
void add_local_spot() {}
void sendmessage(const char *msg) {}
void printcall(const char *msg) {}
unsigned int  GetCWSpeed() { return 10; }
int speedup() { return 12; }
int speeddown() { return 8; }
void rst_recv_up() {}
void rst_recv_down() {}
void vk_play_file(char *audiofile) {}
int send_lan_message(int opcode, char *message) { return 0; }

int last10() { return 0; }
void foc_show_scoring(int start_column) {}
int foc_total_score() { return 0; }


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

#define QSO1 " 80SSB 12-Jan-18 16:34 0006  PY9BBB         59   59   15            PY   15  3  14025.0\n"

#define NOTE "; Test note handling in logfile                                                        \n"

#define LOGFILE "test.log"

void append_log_line(char *logfile, char *line) {
    FILE *fp = fopen(logfile, "a");
    assert_non_null(fp);

    fputs(line, fp);

    fclose(fp);
}

void write_log(char *logfile) {
    FILE *fp = fopen(logfile, "w");
    assert_non_null(fp);
    fclose(fp);

    append_log_line(logfile, QSO1);
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

    strcpy(my.continent, "EU");

    showmsg_spy = STRING_NOT_SET;

    return 0;
}

int teardown_default(void **state) {
    unlink(logfile);
    free_qso_array();
    return 0;
}


/* test lookup country in pfxnummult */
void test_lookup_not_in_pfxnummult(void **state) {
    assert_int_equal(lookup_country_in_pfxnummult_array(1), -1);
}


void test_lookup_in_pfxnummult(void **state) {
    assert_int_equal(lookup_country_in_pfxnummult_array(42), 1);
}

/* test qso_array handling */
void test_qso_array_init(void **state) {
    assert_null(qso_array);
    init_qso_array();
    assert_non_null(qso_array);
    assert_int_equal(qso_array->len, 0);
}

void test_readcalls_simple_log(void **state) {
    int lines;
    gchar *qso_line = g_strndup(QSO1, LOGLINELEN - 1);

    write_log(LOGFILE);
    lines = readcalls(LOGFILE, true);
    assert_non_null(qso_array);
    assert_int_equal(lines, qso_array->len);
    assert_int_equal(lines, 1);
    struct qso_t *qso = g_ptr_array_index(qso_array, 0);
    assert_non_null(qso);
    assert_string_equal(qso->logline, qso_line);
    assert_string_equal(qso->call, "PY9BBB");
    assert_null(qso->normalized_comment);
    g_free(qso_line);
}

void test_readcalls_note(void **state) {
    gchar *note = g_strndup(NOTE, LOGLINELEN - 1);

    write_log(LOGFILE);
    append_log_line(LOGFILE, NOTE);
    assert_int_equal(readcalls(LOGFILE, true), 2);;

    struct qso_t *qso = g_ptr_array_index(qso_array, 1);
    assert_non_null(qso);
    assert_string_equal(qso->logline, note);
    assert_int_equal(qso->is_comment, true);

    g_free(note);
}


/* test readcalls */
void test_add_to_worked(void **state) {
    write_log(LOGFILE);
    readcalls(LOGFILE, true);
    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].call, "PY9BBB");
    assert_string_equal(worked[0].exchange, "15");
    time_t ts = parse_time(QSO1 + 7, DATE_TIME_FORMAT);
    assert_int_equal(worked[0].qsotime[SSBMODE][BANDINDEX_80], ts);
    assert_int_equal(get_nr_of_points(), 3);
    assert_int_equal(get_nr_of_mults(), 2);
    assert_string_equal(showmsg_spy, STRING_NOT_SET);   // log didn't change
}

void test_add_to_worked_dupe(void **state) {
    write_log(LOGFILE);
    append_log_line(LOGFILE, QSO1);     // add same line again
    readcalls(LOGFILE, true);
    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].call, "PY9BBB");
    assert_string_equal(worked[0].exchange, "15");
    time_t ts = parse_time(QSO1 + 7, DATE_TIME_FORMAT);
    assert_int_equal(worked[0].qsotime[SSBMODE][BANDINDEX_80], ts);
    assert_int_equal(get_nr_of_points(), 3);
    assert_int_equal(get_nr_of_mults(), 2);
    assert_string_equal(showmsg_spy,
			"Log changed due to rescoring. Do you want to save it? Y/(N)");
}

void test_add_to_worked_continentlistonly(void **state) {
    continentlist_only = true;
    write_log(LOGFILE);
    readcalls(LOGFILE, false);      // non-interactive
    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].call, "PY9BBB");
    assert_string_equal(worked[0].exchange, "15");
    assert_int_equal(get_nr_of_points(), 3);    // normal CQWW scoring
    assert_int_equal(get_nr_of_mults(), 0);     // but no mult due to continent list
    assert_string_equal(showmsg_spy,
			STRING_NOT_SET);   // non-interactive, no message
}

void test_add_to_worked_wpx(void **state) {
    setcontest("wpx");
    write_log(LOGFILE);
    readcalls(LOGFILE, true);
    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].call, "PY9BBB");
    assert_string_equal(worked[0].exchange, "15");
    time_t ts = parse_time(QSO1 + 7, DATE_TIME_FORMAT);
    assert_int_equal(worked[0].qsotime[SSBMODE][BANDINDEX_80], ts);
    assert_int_equal(get_nr_of_points(), 6);    // different continent, below 10 MHz
    assert_int_equal(get_nr_of_mults(), 1);
    assert_string_equal(showmsg_spy,
			"Log changed due to rescoring. Do you want to save it? Y/(N)");
}
