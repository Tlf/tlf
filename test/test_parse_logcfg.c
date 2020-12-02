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

extern char keyer_device[10];
extern int partials;
extern int use_part;
extern char *editor_cmd;
extern int weight;
extern bool mixedmode;
extern bool ignoredupe;
extern bool continentlist_only;
extern int recall_mult;
extern int wysiwyg_multi;
extern int wysiwyg_once;
extern int trx_control;
extern int rit;
extern int shortqsonr;
extern int showscore_flag;
extern int searchflg;
extern int demode;
extern int exchange_serial;
extern int country_mult;
extern int portable_x2;
extern int cqwwm2;
extern int landebug;
extern int call_update;
extern int time_master;
extern int ctcomp;
extern int serial_section_mult;
extern int sectn_mult;
extern int nob4;
extern int show_time;
extern int use_rxvt;
extern int wazmult;
extern int itumult;
extern int exc_cont;
extern int noautocq;
extern int no_arrows;
extern int sc_sidetone;
extern int lowband_point_mult;
extern int clusterlog;
extern int serial_grid4_mult;
extern int logfrequency;
extern int no_rst;
extern int serial_or_section;
extern int pfxmultab;
extern int qtcrec_record;
extern int qtc_auto_filltime;
extern int bmautograb;
extern int bmautoadd;
extern int qtc_recv_lazy;
extern int sprint_mode;
extern int keyer_backspace;
extern int sectn_mult_once;
extern int lan_port;
extern char rigconf[];
extern char ph_message[14][80];

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
    mixedmode = false;
    ignoredupe = false;
    continentlist_only = false;
    lan_port = 0;

    for (int i = 0; i < SP_CALL_MSG; ++i) {
	message[i][0] = 0;
    }
    for (int i = 0; i < 14; ++i) {
	ph_message[i][0] = 0;
    }

    rigconf[0] = 0;

    callmaster_filename = NULL;

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

void test_unknown_keyword2(void **state) {
    int rc = call_parse_logcfg("F19=CQ\n");   // starts with an existing keyword
    assert_int_equal(rc, PARSE_CONFIRM);
    assert_string_equal(showmsg_spy,
			"Keyword 'F19' not supported. See man page.\n");
}

void test_deprecated_keyword(void **state) {
    int rc = call_parse_logcfg("CW_TU_MSG=TU\n");
    assert_int_equal(rc, PARSE_CONFIRM);
    assert_string_equal(showmsg_spy,
			"Keyword 'CW_TU_MSG' not supported. See man page.\n");
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

typedef struct {
    char *keyword;
    bool *var;
} bool_true_t;

static bool_true_t bool_trues[] = {
    {"CONTEST_MODE", &iscontest},
    {"MIXED", &mixedmode},
    {"IGNOREDUPE", &ignoredupe},
    {"USE_CONTINENTLIST_ONLY", &continentlist_only},
};

void test_bool_trues(void **state) {
    char line[80];
    for (int i = 0; i < sizeof(bool_trues) / sizeof(bool_true_t); ++i) {
	*bool_trues[i].var = false;
	sprintf(line, "%s\n", bool_trues[i].keyword);
	puts(line);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_true(*bool_trues[i].var);
    }
}

// F1 .. F12
void test_fn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	message[j][0] = 0;
	sprintf(msg, "MSG%d ABC", i);
	sprintf(line, "F%d= %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	sprintf(msg, "MSG%d ABC \n", i);   // trailing space+NL are kept, FIXME
	assert_string_equal(message[j], msg);
    }
}

void test_alt_n(void **state) {
    char line[80], msg[30];
    for (int i = 0; i <= 9; ++i) {
	int j = CQ_TU_MSG + 1 + i;
	message[j][0] = 0;
	sprintf(msg, "MSG%d ALT", i);
	sprintf(line, "ALT_%d= %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	sprintf(msg, "MSG%d ALT \n", i);   // trailing space+NL are kept, FIXME
	assert_string_equal(message[j], msg);
    }
}

void test_sp_tu_msg(void **state) {
    int rc = call_parse_logcfg("S&P_TU_MSG=TU\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(message[SP_TU_MSG], "TU\n");
}

void test_cq_tu_msg(void **state) {
    int rc = call_parse_logcfg("CQ_TU_MSG=TU QRZ?\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(message[CQ_TU_MSG], "TU QRZ?\n");
}

void test_sp_call_msg(void **state) {
    int rc = call_parse_logcfg("S&P_CALL_MSG=DE AB1CD\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(message[SP_CALL_MSG], "DE AB1CD\r\n");  // FIXME line end...
}

typedef struct {
    char *keyword;
    int *var;
} int_one_t;

static int_one_t int_ones[] = {
    {"RECALL_MULTS", &recall_mult},
    {"WYSIWYG_MULTIBAND", &wysiwyg_multi},
    {"WYSIWYG_ONCE", &wysiwyg_once},
    {"RADIO_CONTROL", &trx_control},
    {"RIT_CLEAR", &rit},
    {"SHORT_SERIAL", &shortqsonr},
    {"SCOREWINDOW", &showscore_flag},
    {"CHECKWINDOW", &searchflg},
    {"SEND_DE", &demode},
    {"SERIAL_EXCHANGE", &exchange_serial},
    {"COUNTRY_MULT", &country_mult},
    {"PORTABLE_MULT_2", &portable_x2},
    {"CQWW_M2", &cqwwm2},
    {"LAN_DEBUG", &landebug},
    {"CALLUPDATE", &call_update},
    {"TIME_MASTER", &time_master},
    {"CTCOMPATIBLE", &ctcomp},
    {"SERIAL+SECTION", &serial_section_mult},
    {"SECTION_MULT", &sectn_mult},
    {"NOB4", &nob4},
    {"SHOW_TIME", &show_time},
    {"RXVT", &use_rxvt},
    {"WAZMULT", &wazmult},
    {"ITUMULT", &itumult},
    {"CONTINENT_EXCHANGE", &exc_cont},
    {"NOAUTOCQ", &noautocq},
    {"NO_BANDSWITCH_ARROWKEYS", &no_arrows},
    {"SOUNDCARD", &sc_sidetone},
    {"LOWBAND_DOUBLE", &lowband_point_mult},
    {"CLUSTER_LOG", &clusterlog},
    {"SERIAL+GRID4", &serial_grid4_mult},
    {"LOGFREQUENCY", &logfrequency},
    {"NO_RST", &no_rst},
    {"SERIAL_OR_SECTION", &serial_or_section},
    {"PFX_MULT_MULTIBAND", &pfxmultab},
    {"QTCREC_RECORD", &qtcrec_record},
    {"QTC_AUTO_FILLTIME", &qtc_auto_filltime},
    {"BMAUTOGRAB", &bmautograb},
    {"BMAUTOADD", &bmautoadd},
    {"QTC_RECV_LAZY", &qtc_recv_lazy},
    {"SPRINTMODE", &sprint_mode},
    {"KEYER_BACKSPACE", &keyer_backspace},
    {"SECTION_MULT_ONCE", &sectn_mult_once},
};

void test_int_ones(void **state) {
    char line[80];
    for (int i = 0; i < sizeof(int_ones) / sizeof(int_one_t); ++i) {
	*int_ones[i].var = 0;
	sprintf(line, "%s\n", int_ones[i].keyword);
	puts(line);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_int_equal(*int_ones[i].var, 1);
    }
}

void test_lan_port(void **state) {
    int rc = call_parse_logcfg("LAN_PORT=1234\n");
    assert_int_equal(rc, 0);
    assert_int_equal(lan_port, 1234);
}

void test_rigconf(void **state) {
    int rc = call_parse_logcfg("RIGCONF= ABCD\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(rigconf, "ABCD");
}

void test_callmaster(void **state) {
    int rc = call_parse_logcfg("CALLMASTER=calls.txt\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(callmaster_filename, "calls.txt");
    g_free(callmaster_filename);
}

void test_vkmn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	ph_message[j][0] = 0;
	sprintf(msg, "MSG%d.wav", i);
	sprintf(line, "VKM%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_string_equal(ph_message[j], msg);
    }
}

void test_vkspm(void **state) {
    int rc = call_parse_logcfg("VKSPM=a.wav\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(ph_message[SP_TU_MSG], "a.wav");
}

void test_vkcqm(void **state) {
    int rc = call_parse_logcfg("VKCQM=b.wav\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(ph_message[CQ_TU_MSG], "b.wav");
}

