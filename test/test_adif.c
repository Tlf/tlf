#include "test.h"
#include <glib.h>

#include "../src/log_utils.h"

// OBJECT ../src/writecabrillo.o
// OBJECT ../src/cabrillo_utils.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/get_time.o
// OBJECT ../src/bands.o

/* test stubs and dummies */
struct qso_t *parse_logline(char *buffer);
void prepare_adif_line(char *buffer, struct qso_t *qso, char *standardexchange);
void free_qso(struct qso_t *ptr);
void free_cabfmt();
void add_adif_field(char *adif_line, char *field, char *value);

void nicebox();

int stoptx() {
    return 0;
}

int modify_attr(int x) {
    return x;
}

int getsummary(FILE fp) {
    return 0;
}

void ask(char* buffer, char *what) {
}

char buffer[181];
char logline[181];
char adif_line[400];

#define ADIF "Test"

int setup_default(void **state) {
	strcpy(adif_line, ADIF);

	return 0;
}

#define LOGLINE1 " 20CW  23-Dec-15 13:16 0135  SV5K           599  599  20            SV5      1         "
#define LOGLINE2 " 20CW  23-Dec-15 13:16 0134  OE3NKJ         599  599  15                     1         "
#define RESULT1 "<CALL:4>SV5K<BAND:3>20M<MODE:2>CW<QSO_DATE:8>20151223<TIME_ON:4>1316<RST_SENT:3>599<STX_STRING:2>14<RST_RCVD:3>599<SRX_STRING:2>20<eor>\n"
#define RESULT2 "<CALL:6>OE3NKJ<BAND:3>20M<MODE:2>CW<QSO_DATE:8>20151223<TIME_ON:4>1316<RST_SENT:3>599<STX_STRING:2>14<RST_RCVD:3>599<SRX_STRING:2>15<eor>\n"


/* prepare:adif_line */
void test_keep_old_format(void **state) {
    char exch[4] = "14";

    struct qso_t *qso;
    strcpy(buffer, LOGLINE1);
    qso = parse_logline(buffer);
    prepare_adif_line(buffer, qso, exch);
    assert_string_equal(buffer, RESULT1);
    free_qso(qso);

    strcpy(buffer, LOGLINE2);
    qso = parse_logline(buffer);
    prepare_adif_line(buffer, qso, exch);
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
