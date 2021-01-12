#include "test.h"
#include <glib.h>

#include "../src/log_utils.h"
#include "../src/globalvars.h"
#include "../src/cqww_simulator.h"

// OBJECT ../src/writecabrillo.o
// OBJECT ../src/cabrillo_utils.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/get_time.o
// OBJECT ../src/bands.o
// OBJECT ../src/sendbuf.o
// OBJECT ../src/utils.o

/* test stubs and dummies */
struct qso_t *parse_logline(char *buffer);
void prepare_adif_line(char *buffer, struct qso_t *qso);
void free_qso(struct qso_t *ptr);
void free_cabfmt();
void add_adif_field(char *adif_line, char *field, char *value);

bool simulator = false;

void nicebox();

int stoptx() {
    return 0;
}

int modify_attr(int x) {
    return x;
}

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

char *error_details;

contest_config_t empty = { };

char logline[181];
char adif_line[400];

#define ADIF "Test"

int setup_default(void **state) {
    contest = &empty;

    strcpy(adif_line, ADIF);

    return 0;
}

#define LOGLINE1 " 20CW  23-Dec-15 13:16 0135  SV5K           599  599  20            SV5      1         "
#define LOGLINE2 " 20SSB 23-Dec-15 13:16 0134  OE3NKJ         59   59   15                     1  14187.6"
#define RESULT1 "<CALL:4>SV5K<BAND:3>20M<MODE:2>CW<QSO_DATE:8>20151223<TIME_ON:4>1316<RST_SENT:3>599<STX_STRING:2>14<RST_RCVD:3>599<SRX_STRING:2>20<eor>\n"
#define RESULT2 "<CALL:6>OE3NKJ<BAND:3>20M<FREQ:7>14.1876<MODE:3>SSB<QSO_DATE:8>20151223<TIME_ON:4>1316<RST_SENT:2>59<STX_STRING:2>14<RST_RCVD:2>59<SRX_STRING:2>15<eor>\n"


/* prepare_adif_line */
void test_keep_old_format(void **state) {
    char buffer[181];

    strcpy(exchange, "14");

    struct qso_t *qso;
    strcpy(buffer, LOGLINE1);
    qso = parse_logline(buffer);
    prepare_adif_line(buffer, qso);
    assert_string_equal(buffer, RESULT1);
    free_qso(qso);

    strcpy(buffer, LOGLINE2);
    qso = parse_logline(buffer);
    prepare_adif_line(buffer, qso);
    assert_string_equal(buffer, RESULT2);
    free_qso(qso);
}

/* test add_adif_field and co */
void test_add_adif_noField(void **state) {
    add_adif_field(adif_line, "", "Hi");
    assert_string_equal(adif_line, ADIF);
}


void test_add_adif_noValue(void **state) {
    add_adif_field(adif_line, "Field1", NULL);
    assert_string_equal(adif_line, ADIF"<Field1>");
}

void test_add_adif_emptyValue(void **state) {
    add_adif_field(adif_line, "Field1", "");
    assert_string_equal(adif_line, ADIF"<Field1:0>");
}

void test_add_adif_Value(void **state) {
    add_adif_field(adif_line, "Field1", "Hi");
    assert_string_equal(adif_line, ADIF"<Field1:2>Hi");
}
