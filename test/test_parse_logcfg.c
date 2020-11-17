#include "test.h"
#include <stdio.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../src/parse_logcfg.h"
#include "../src/lancode.h"
#include "../src/bandmap.h"
#include "../src/qtcvars.h"
#include "../src/tlf.h"
#include "../src/globalvars.h"

// OBJECT ../src/parse_logcfg.o
// OBJECT ../src/get_time.o
// OBJECT ../src/getpx.o
// OBJECT ../src/getwwv.o
// OBJECT ../src/locator2longlat.o
// OBJECT ../src/score.o
// OBJECT ../src/qrb.o

extern char *editor_cmd;
extern int weight;

// lancode.c
int nodes = 0;
int node;
struct sockaddr_in bc_address[MAXNODES];
int lan_port = 6788;
int landebug = 0;
char thisnode = 'A';
int time_master;
char bc_hostaddress[MAXNODES][16];
char bc_hostservice[MAXNODES][16] = {
    [0 ... MAXNODES - 1] = { [0 ... 15] = 0 }
};


char netkeyer_hostaddress[16] = "127.0.0.1";
int netkeyer_port = 6789;

bm_config_t bm_config;

extern rig_model_t myrig_model;

char *callmaster_filename = NULL;

int call_update = 0;

t_qtc_ry_line qtc_ry_lines[QTC_RY_LINE_NR];

extern char keyer_device[10];
extern int partials;
extern int use_part;

void setcontest() {
    // TBD
}

bool fldigi_isenabled(void) {
    return false;   // TBD
}

void SetCWSpeed(unsigned int wpm) {
    // TBD
}

void fldigi_toggle() {
    // TBD
}

void rst_init(char *init_string) {
    // TBD
}

int getctydata(char *checkcallptr) {
    // TBD
    return 0;
}

int getctynr(char *checkcall) {
    // TBD
    return 0;
}

int foc_score(char *a) {
    // TBD
    return 0;
}

/* setup/teardown */
int setup_default(void **state) {
    *keyer_device = 0;
    partials = 0;
    use_part = 0;

    showmsg_spy = STRING_NOT_SET;

    return 0;
}

int teardown_default(void **state) {
    return 0;
}

/*
    call parse_logcfg with a dynamically allocated string
    as g_strstip() fails on a statically allocated one
*/
static int call_parse_logcfg(const char *input) {
    char *line = g_strdup(input);
    int rc = parse_logcfg(line);
    g_free(line);
    return rc;
}

void test_unknown_keyword(void **state) {
    int rc = call_parse_logcfg("UNKNOWN\r\n");   // DOS line ending
    assert_int_equal(rc, PARSE_CONFIRM);
    assert_string_equal(showmsg_spy,
			"Keyword 'UNKNOWN' not supported. See man page.\n");
}

void test_logfile(void **state) {
    int rc = call_parse_logcfg("LOGFILE= mylog\r\n");   // space before arg, DOS line ending
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(logfile, "mylog");
}

void test_logfile_no_arg(void **state) {
    int rc = call_parse_logcfg("LOGFILE\r\n");   // DOS line ending
    assert_int_equal(rc, PARSE_CONFIRM);
    assert_string_equal(showmsg_spy,
			"Keyword 'LOGFILE' must be followed by an parameter ('=....'). See man page.\n");
}

void test_keyer_device(void **state) {
    int rc = call_parse_logcfg("KEYER_DEVICE =/dev/tty0\r\n");   // space after keyword, DOS line ending
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(keyer_device, "/dev/tty0");
}

void test_editor(void **state) {
    int rc = call_parse_logcfg("EDITOR= pico \n");   // space around argument
    assert_int_equal(rc, 0);
    assert_string_equal(editor_cmd, "pico");
}

void test_weight(void **state) {
    int rc = call_parse_logcfg("WEIGHT=12");    // no line ending
    assert_int_equal(rc, 0);
    assert_int_equal(weight, 12);
}

void test_partials(void **state) {
    int rc = call_parse_logcfg("  PARTIALS\n");  // leading space
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(partials, 1);
}

void test_usepartials(void **state) {
    int rc = call_parse_logcfg("USEPARTIALS  \n");  // trailing space
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(use_part, 1);
}

void test_usepartials_with_arg(void **state) {
    int rc = call_parse_logcfg("USEPARTIALS=no\n");
    assert_int_equal(rc, 0); // FIXME: this should be 1
}


