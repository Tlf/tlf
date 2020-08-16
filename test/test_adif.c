#include "test.h"
#include <glib.h>

#include "../src/log_utils.h"

// OBJECT ../src/writecabrillo.o
// OBJECT ../src/cabrillo_utils.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/get_time.o
// OBJECT ../src/bands.o

/* test stubs and dummies */
void prepare_adif_line(char *buffer, char *logline, char *standardexchange);
void free_cabfmt();
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

#define LOGLINE1 " 20CW  23-Dec-15 13:16 0135  SV5K           599  599  20            SV5      1         "
#define LOGLINE2 " 20CW  23-Dec-15 13:16 0134  OE3NKJ         599  599  15                     1         "
#define RESULT1 "<CALL:4>SV5K<BAND:3>20M<MODE:2>CW<QSO_DATE:8>20151223<TIME_ON:4>1316<RST_SENT:3>599<STX_STRING:2>14<RST_RCVD:3>599<SRX_STRING:2>20<eor>\n"
#define RESULT2 "<CALL:6>OE3NKJ<BAND:3>20M<MODE:2>CW<QSO_DATE:8>20151223<TIME_ON:4>1316<RST_SENT:3>599<STX_STRING:2>14<RST_RCVD:3>599<SRX_STRING:2>15<eor>\n"


/* prepare:adif_line */
void test_keep_old_format(void **state) {
    char exch[4] = "14";

    strcpy(logline, LOGLINE1);
    prepare_adif_line(buffer, logline, exch);
    assert_string_equal(buffer, RESULT1);
    strcpy(logline, LOGLINE2);
    prepare_adif_line(buffer, logline, exch);
    assert_string_equal(buffer, RESULT2);
}
