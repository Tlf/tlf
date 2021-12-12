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
#include "../src/getwwv.h"
#include "../src/change_rst.h"
#include "../src/setcontest.h"
#include "../src/set_tone.h"
#include "../src/cabrillo_utils.h"

// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/parse_logcfg.o
// OBJECT ../src/get_time.o
// OBJECT ../src/getpx.o
// OBJECT ../src/getwwv.o
// OBJECT ../src/locator2longlat.o
// OBJECT ../src/score.o
// OBJECT ../src/qrb.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/cabrillo_utils.o

// lancode.c
int nodes = 0;
struct sockaddr_in bc_address[MAXNODES];
int lan_port = 6788;
bool lan_active;
bool landebug = false;
char thisnode = 'A';
bool time_master;
char bc_hostaddress[MAXNODES][16];
char bc_hostservice[MAXNODES][16] = {
    [0 ... MAXNODES - 1] = { [0 ... 15] = 0 }
};


char netkeyer_hostaddress[16] = "127.0.0.1";
int netkeyer_port = 6789;

bm_config_t bm_config;

char *callmaster_filename = NULL;

bool call_update = false;

t_qtc_ry_line qtc_ry_lines[QTC_RY_LINE_NR];

contest_config_t config_focm;

static bool fldigi_on;
bool fldigi_isenabled(void) {
    return fldigi_on;
}

void fldigi_toggle() {
    fldigi_on = !fldigi_on;
}

int get_total_score() {
    return 123;
}

void ask(char *buffer, char *what) {
}

static int wpm_spy;
void SetCWSpeed(unsigned int wpm) {
    wpm_spy = wpm;
}

static char rst_init_spy[100];
void rst_init(char *init_string) {
    if (init_string != NULL) {
	strcpy(rst_init_spy, init_string);
    } else {
	strcpy(rst_init_spy, "(NULL)");
    }
}

int getctydata(char *checkcallptr) {
    // used for "PFX_NUM_MULTIS=W,VE,VK,ZL,ZS,JA,PY,UA9"
    if (g_str_has_prefix(checkcallptr, "W")) return 18;
    if (g_str_has_prefix(checkcallptr, "VE")) return 17;
    if (g_str_has_prefix(checkcallptr, "VK")) return 16;
    if (g_str_has_prefix(checkcallptr, "ZL")) return 15;
    if (g_str_has_prefix(checkcallptr, "ZS")) return 14;
    if (g_str_has_prefix(checkcallptr, "JA")) return 13;
    if (g_str_has_prefix(checkcallptr, "PY")) return 12;
    if (g_str_has_prefix(checkcallptr, "UA9")) return 11;
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
    int result = chdir(SRCDIR);
    if (result == -1)
	perror("chdir");

    memset(&my, 0, sizeof(my));
    memset(&bm_config, 0, sizeof(bm_config));
    memset(&pfxnummulti, 0, sizeof(pfxnummulti));

    *keyer_device = 0;
    partials = false;
    use_part = false;
    mixedmode = false;
    ignoredupe = false;
    continentlist_only = false;
    lan_port = 0;
    timeoffset = 0;
    cwkeyer = 0;
    digikeyer = NO_KEYER;
    packetinterface = 0;
    portnum = 0;
    shortqsonr = 0;
    cluster = 0;
    cqdelay = 8;
    ssbpoints = 1;
    cwpoints = 1;
    trxmode = CWMODE;
    use_bandoutput = 0;
    thisnode = 'A';
    nodes = 0;
    xplanet = MARKER_NONE;
    dx_arrlsections = false;
    mult_side = false;
    countrylist_points = -1;
    my_country_points = -1;
    my_cont_points = -1;
    dx_cont_points = -1;
    continentlist_points = -1;
    cw_bandwidth = 0;
    change_rst = false;
    fixedmult = 0.0;
    exclude_multilist_type = EXCLUDE_NONE;
    rigptt = 0;
    minitest = 0;

    setcontest(QSO_MODE);

    tonestr[0] = 0;
    multsfile[0] = 0;
    markerfile[0] = 0;
    synclogfile[0] = 0;
    sc_volume[0] = 0;
    sc_device[0] = 0;
    modem_mode[0] = 0;
    controllerport[0] = 0;
    clusterlogin[0] = 0;
    exchange_list[0] = 0;
    rttyoutput[0] = 0;
    qtcrec_record_command[0][0] = 0;
    qtcrec_record_command[1][0] = 0;
    qtcrec_record_command_shutdown[0] = 0;
    unique_call_multi = 0;
    digi_mode = -1;

    for (int i = 0; i < SP_CALL_MSG; ++i) {
	message[i][0] = 0;
    }
    for (int i = 0; i < CQ_TU_MSG; ++i) {
	ph_message[i][0] = 0;
	qtc_phrecv_message[i][0] = 0;
	qtc_phsend_message[i][0] = 0;
    }
    for (int i = 0; i < SP_TU_MSG; ++i) {
	qtc_send_msgs[i][0] = 0;
	qtc_recv_msgs[i][0] = 0;
    }
    for (int i = 0; i < 12; ++i) {
	FREE_DYNAMIC_STRING(digi_message[i]);
    }
    for (int i = 0; i < 10; ++i) {
	bandindexarray[i] = 0;
    }
    for (int i = 0; i < MAXNODES; ++i) {
	bc_hostaddress[i][0] = 0;
	bc_hostservice[i][0] = 0;
    }
    for (int i = 0; i < 255; ++i) {
	countrylist[i][0] = 0;
    }
    for (int i = 0; i < 7; ++i) {
	continent_multiplier_list[i][0] = 0;
    }
    for (int i = 0; i < NBANDS; ++i) {
	bandweight_points[i] = 0;
	bandweight_multis[i] = 0;
    }

    rigconf[0] = 0;
    pr_hostaddress[0] = 0;

    FREE_DYNAMIC_STRING(editor_cmd);
    FREE_DYNAMIC_STRING(cabrillo);
    FREE_DYNAMIC_STRING(callmaster_filename);
    FREE_DYNAMIC_STRING(rigportname);

    showmsg_spy = STRING_NOT_SET;
    rst_init_spy[0] = 0;
    fldigi_on = false;

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
    assert_int_equal(rc, PARSE_ERROR);
    assert_string_equal(showmsg_spy,
			"Keyword 'UNKNOWN' not supported. See man page.\n");
}

void test_unknown_keyword2(void **state) {
    int rc = call_parse_logcfg("F19=CQ\n");   // starts with an existing keyword
    assert_int_equal(rc, PARSE_ERROR);
    assert_string_equal(showmsg_spy,
			"Keyword 'F19' not supported. See man page.\n");
}

void test_deprecated_keyword(void **state) {
    int rc = call_parse_logcfg("CW_TU_MSG=TU\n");
    assert_int_equal(rc, PARSE_ERROR);
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
    assert_int_equal(rc, PARSE_ERROR);
    assert_string_equal(showmsg_spy,
			"Keyword 'LOGFILE' must be followed by a parameter ('=....'). See man page.\n");
}

void test_keyer_device(void **state) {
    int rc = call_parse_logcfg("KEYER_DEVICE =/dev/tty0\r\n");   // space after keyword, DOS line ending
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(keyer_device, "/dev/tty0");
}

void test_keyer_device_too_long(void **state) {
    int rc = call_parse_logcfg("KEYER_DEVICE=/dev/bus/usb/002/001");
    assert_int_equal(rc, PARSE_ERROR);
    assert_string_equal(showmsg_spy,
			"Wrong parameter for keyword 'KEYER_DEVICE': value too long.\n");
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
    assert_int_equal(rc, PARSE_ERROR);
    assert_string_equal(showmsg_spy,
			"Keyword 'USEPARTIALS' can't have a parameter. See man page.\n");
}

typedef struct {
    char *keyword;
    bool *var;
} bool_true_t;

static bool_true_t bool_trues[] = {
    {"RIT_CLEAR", &rit},
    {"SCOREWINDOW", &showscore_flag},
    {"CHECKWINDOW", &searchflg},
    {"CONTEST_MODE", &iscontest},
    {"CALLUPDATE", &call_update},
    {"SOUNDCARD", &sc_sidetone},
    {"TIME_MASTER", &time_master},
    {"SEND_DE", &demode},
    {"CTCOMPATIBLE", &ctcomp},
    {"NOB4", &nob4},
    {"SHOW_TIME", &show_time},
    {"RXVT", &use_rxvt},
    {"NOAUTOCQ", &noautocq},
    {"NO_BANDSWITCH_ARROWKEYS", &no_arrows},
    {"NO_RST", &no_rst},
    {"KEYER_BACKSPACE", &keyer_backspace},
    {"CLUSTER_LOG", &clusterlog},
    {"LAN_DEBUG", &landebug},
    {"BMAUTOGRAB", &bmautograb},
    {"BMAUTOADD", &bmautoadd},
    {"CQWW_M2", &cqwwm2},
    {"CONTINENT_EXCHANGE", &exc_cont},
    {"MIXED", &mixedmode},
    {"IGNOREDUPE", &ignoredupe},
    {"USE_CONTINENTLIST_ONLY", &continentlist_only},
    {"RADIO_CONTROL", &trx_control},
    {"PORTABLE_MULT_2", &portable_x2},
    {"WYSIWYG_MULTIBAND", &wysiwyg_multi},
    {"WYSIWYG_ONCE", &wysiwyg_once},
    {"SERIAL+SECTION", &serial_section_mult},
    {"SERIAL_OR_SECTION", &serial_or_section},
    {"SECTION_MULT", &sectn_mult},
    {"SECTION_MULT_ONCE", &sectn_mult_once},
    {"SERIAL+GRID4", &serial_grid4_mult},
    {"COUNTRY_MULT", &country_mult},
    {"ITUMULT", &itumult},
    {"WAZMULT", &wazmult},
    {"SPRINTMODE", &sprint_mode},
    {"PFX_MULT", &pfxmult},
    {"PFX_MULT_MULTIBAND", &pfxmultab},
    {"LOWBAND_DOUBLE", &lowband_point_mult},
    {"LOGFREQUENCY", &logfrequency},
    {"QTCREC_RECORD", &qtcrec_record},
    {"QTC_AUTO_FILLTIME", &qtc_auto_filltime},
    {"QTC_RECV_LAZY", &qtc_recv_lazy},
};

void test_bool_trues(void **state) {
    char line[80];
    for (int i = 0; i < sizeof(bool_trues) / sizeof(bool_true_t); ++i) {
	*bool_trues[i].var = false;
	sprintf(line, "%s\n", bool_trues[i].keyword);
	fputs(line, stdout);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_true(*bool_trues[i].var);
    }
}

typedef struct {
    char *keyword;
    size_t offset;
} bool_contest_true_t;

static bool_contest_true_t bool_contest_trues[] = {
    {"RECALL_MULTS", offsetof(contest_config_t, recall_mult)},
    {"SERIAL_EXCHANGE", offsetof(contest_config_t, exchange_serial)},
};


void test_bool_contest_trues(void **state) {
    char line[80];
    for (int i = 0;
	    i < sizeof(bool_contest_trues) / sizeof(bool_contest_true_t);
	    ++i) {
	bool *target = (bool *)((char *)contest +
				bool_contest_trues[i].offset);
	*target = false;
	sprintf(line, "%s\n", bool_contest_trues[i].keyword);
	fputs(line, stdout);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_true(*target);
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
	sprintf(msg, "MSG%d ABC", i);
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
	sprintf(msg, "MSG%d ALT", i);
	assert_string_equal(message[j], msg);
    }
}

void test_sp_tu_msg(void **state) {
    int rc = call_parse_logcfg("S&P_TU_MSG=TU\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(message[SP_TU_MSG], "TU");
}

void test_cq_tu_msg(void **state) {
    int rc = call_parse_logcfg("CQ_TU_MSG=TU QRZ?\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(message[CQ_TU_MSG], "TU QRZ?");
}

void test_sp_call_msg(void **state) {
    int rc = call_parse_logcfg("S&P_CALL_MSG=DE AB1CD\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(message[SP_CALL_MSG], "DE AB1CD");
}

typedef struct {
    char *keyword;
    int *var;
} int_one_t;

static int_one_t int_ones[] = {
    {"SHORT_SERIAL", &shortqsonr},
};

void test_int_ones(void **state) {
    char line[80];
    for (int i = 0; i < sizeof(int_ones) / sizeof(int_one_t); ++i) {
	*int_ones[i].var = 0;
	sprintf(line, "%s\n", int_ones[i].keyword);
	fputs(line, stdout);
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

// DKF1..DKF12
void test_dkfn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	ph_message[j][0] = 0;
	sprintf(msg, "DMSG%d JKL", i);
	sprintf(line, "DKF%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_non_null(digi_message[j]);
	sprintf(msg, "DMSG%d JKL  ", i);    // FIXME converts NL to space...
	assert_string_equal(digi_message[j], msg);
    }
}

void test_alt_dkn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 10; ++i) { // FIXME why DK1..DK10 ??
	int j = CQ_TU_MSG + i;
	sprintf(msg, "ADMSG%d JKL", i);
	sprintf(line, "ALT_DK%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_non_null(digi_message[j]);
	sprintf(msg, "ADMSG%d JKL  ", i);    // FIXME converts NL to space...
	assert_string_equal(digi_message[j], msg);
    }
}


void test_dkcqm(void **state) {
    int rc = call_parse_logcfg("DKCQM=DCQM\n");
    assert_int_equal(rc, PARSE_OK);
    assert_non_null(digi_message[CQ_TU_MSG]);
    assert_string_equal(digi_message[CQ_TU_MSG],
			"DCQM "); // FIXME converts NL to space...
}

void test_dkspm(void **state) {
    int rc = call_parse_logcfg("DKSPM=DSPM\n");
    assert_int_equal(rc, PARSE_OK);
    assert_non_null(digi_message[SP_TU_MSG]);
    assert_string_equal(digi_message[SP_TU_MSG],
			"DSPM "); // FIXME converts NL to space...
}

void test_dkspc(void **state) {
    int rc = call_parse_logcfg("DKSPC=DSPC\n");
    assert_int_equal(rc, PARSE_OK);
    assert_non_null(digi_message[SP_CALL_MSG]);
    assert_string_equal(digi_message[SP_CALL_MSG],
			"DSPC "); // FIXME converts NL to space...
}

// QR_F1..QR_F12
void test_qr_fn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	qtc_recv_msgs[j][0] = 0;
	sprintf(msg, "QRMSG%d MNO", i);
	sprintf(line, "QR_F%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	sprintf(msg, "QRMSG%d MNO", i);
	assert_string_equal(qtc_recv_msgs[j], msg);
    }
}

// QS_F1..QS_F12
void test_qs_fn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	qtc_send_msgs[j][0] = 0;
	sprintf(msg, "QSMSG%d MNO", i);
	sprintf(line, "QS_F%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	sprintf(msg, "QSMSG%d MNO", i);
	assert_string_equal(qtc_send_msgs[j], msg);
    }
}

// QR_VKM1..QR_VKM12
void test_qr_vkmn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	qtc_phrecv_message[j][0] = 0;
	sprintf(msg, "QRVK%d.wav", i);
	sprintf(line, "QR_VKM%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_string_equal(qtc_phrecv_message[j], msg);
    }
}

// QS_VKM1..QS_VKM12
void test_qs_vkmn(void **state) {
    char line[80], msg[30];
    for (int i = 1; i <= 12; ++i) {
	int j = i - 1;
	qtc_phsend_message[j][0] = 0;
	sprintf(msg, "QSVK%d.wav", i);
	sprintf(line, "QS_VKM%d = %s \n", i, msg);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, PARSE_OK);
	assert_string_equal(qtc_phsend_message[j], msg);
    }
}

void test_qr_vkspm(void **state) {
    int rc = call_parse_logcfg("QR_VKSPM=a.wav\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(qtc_phrecv_message[SP_TU_MSG], "a.wav");
}

void test_qr_vkcqm(void **state) {
    int rc = call_parse_logcfg("QR_VKCQM=b.wav\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(qtc_phrecv_message[CQ_TU_MSG], "b.wav");
}

void test_qs_vkspm(void **state) {
    int rc = call_parse_logcfg("QS_VKSPM=a.wav\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(qtc_phsend_message[SP_TU_MSG], "a.wav");
}

void test_qs_vkcqm(void **state) {
    int rc = call_parse_logcfg("QS_VKCQM=b.wav\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(qtc_phsend_message[CQ_TU_MSG], "b.wav");
}

void test_fkey_header(void **state) {
    int rc = call_parse_logcfg("FKEY-HEADER = F1=CQ F2=XYZ \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(fkey_header, "F1=CQ F2=XYZ");
}

void test_call(void **state) {
    int rc = call_parse_logcfg("CALL = AB1cd\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(my.call, "AB1CD");
}

void test_cabrillo(void **state) {
    int rc = call_parse_logcfg("CABRILLO = test.cab \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(cabrillo, "test.cab");
}

void test_cabrillo_power(void **state) {
    cbr_field_t *power = find_cabrillo_field("CATEGORY-POWER");
    FREE_DYNAMIC_STRING(power->value);
    assert_int_equal(power->disabled, false);
    assert_int_equal(power->value_is_hint, false);

    int rc = call_parse_logcfg("CABRILLO-CATEGORY-POWER = HIGH \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_non_null(power->value);
    assert_string_equal(power->value, "HIGH");
    assert_int_equal(power->disabled, false);
    assert_int_equal(power->value_is_hint, false);
}

void test_cabrillo_power_hint(void **state) {
    cbr_field_t *power = find_cabrillo_field("CATEGORY-POWER");
    FREE_DYNAMIC_STRING(power->value);
    assert_int_equal(power->disabled, false);
    assert_int_equal(power->value_is_hint, false);

    int rc = call_parse_logcfg("CABRILLO-CATEGORY-POWER = (QRP, LOW) \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_non_null(power->value);
    assert_string_equal(power->value, "(QRP, LOW)");
    assert_int_equal(power->disabled, false);
    assert_int_equal(power->value_is_hint, true);
}

void test_cabrillo_power_disable(void **state) {
    cbr_field_t *power = find_cabrillo_field("CATEGORY-POWER");
    FREE_DYNAMIC_STRING(power->value);
    assert_int_equal(power->disabled, false);

    int rc = call_parse_logcfg("CABRILLO-CATEGORY-POWER = - \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_null(power->value);
    assert_int_equal(power->disabled, true);
}

void test_cabrillo_address3_enable(void **state) {
    cbr_field_t *addr3 = find_cabrillo_field("ADDRESS(3)");
    FREE_DYNAMIC_STRING(addr3->value);
    assert_int_equal(addr3->disabled, true);

    int rc = call_parse_logcfg("CABRILLO-ADDRESS(3)\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_null(addr3->value);
    assert_int_equal(addr3->disabled, false);
}

void test_time_offset(void **state) {
    int rc = call_parse_logcfg("TIME_OFFSET = -4\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(timeoffset, -4);
}

void test_netkeyer(void **state) {
    int rc = call_parse_logcfg("NETKEYER\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(cwkeyer, NET_KEYER);
}

void test_netkeyerport(void **state) {
    int rc = call_parse_logcfg("NETKEYERPORT = 16789\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(netkeyer_port, 16789);
}

void test_netkeyerhost(void **state) {
    int rc = call_parse_logcfg("NETKEYERHOST = host.net \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(netkeyer_hostaddress, "host.net");
}

void test_rigport(void **state) {
    int rc = call_parse_logcfg("RIGPORT = /dev/rigport \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_non_null(rigportname);
    assert_string_equal(rigportname, "/dev/rigport \r\n");  // FIXME...
}

void test_tncspeed(void **state) {
    int rc = call_parse_logcfg("TNCSPEED = 1200\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(tnc_serial_rate, 1200);
}

void test_rigspeed(void **state) {
    int rc = call_parse_logcfg("RIGSPEED = 38400\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(serial_rate, 38400);
}

void test_fifo_interface(void **state) {
    int rc = call_parse_logcfg("FIFO_INTERFACE\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(packetinterface, FIFO_INTERFACE);
}

void test_telnethost(void **state) {
    int rc = call_parse_logcfg("TELNETHOST = host.net \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(pr_hostaddress, "host.net");
}

void test_telnetport(void **state) {
    int rc = call_parse_logcfg("TELNETPORT = 12345 \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(portnum, 12345);
    assert_int_equal(packetinterface, TELNET_INTERFACE);
}

void test_long_serial(void **state) {
    shortqsonr = 1;
    int rc = call_parse_logcfg("LONG_SERIAL  \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(shortqsonr, 0);
}

void test_cluster(void **state) {
    int rc = call_parse_logcfg("CLUSTER\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(cluster, CLUSTER);
}

void test_qtc_cap_calls(void **state) {
    int rc = call_parse_logcfg(" QTC_CAP_CALLS = abc \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(qtc_cap_calls, "abc");
}

void test_cqdelay(void **state) {
    int rc = call_parse_logcfg("CQDELAY=12\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(cqdelay, 12);
}

void test_ssbpoints(void **state) {
    int rc = call_parse_logcfg("SSBPOINTS=2\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(ssbpoints, 2);
}

void test_cwpoints(void **state) {
    int rc = call_parse_logcfg("CWPOINTS=3\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(cwpoints, 3);
}

void test_ssbmode(void **state) {
    int rc = call_parse_logcfg("SSBMODE\n");
    assert_int_equal(rc, 0);
    assert_int_equal(trxmode, SSBMODE);
}

// TLFCOLOR1..6
void test_tlfcolorn(void **state) {
    char line[80];
    for (int i = 1 ; i <= 6; ++i) {
	int j = (i == 1 ? 1 : i + 1);   // skip COLOR_RED
	tlfcolors[j][0] = 0;
	tlfcolors[j][1] = 0;
	sprintf(line, "TLFCOLOR%d=%d%d\n", i, i, 7 - i);
	int rc = call_parse_logcfg(line);
	assert_int_equal(rc, 0);
	assert_int_equal(tlfcolors[j][0], i);
	assert_int_equal(tlfcolors[j][1], 7 - i);
    }
}

void test_contest(void **state) {
    int rc = call_parse_logcfg("CONTEST= adx \n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(whichcontest, "adx");
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);	/* setcontest() called */
}

void test_rules(void **state) {
    int rc = call_parse_logcfg("RULES=bdx\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(whichcontest, "bdx");
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_bandoutput(void **state) {
    int rc = call_parse_logcfg("BANDOUTPUT=9876543210\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(use_bandoutput, 1);
    for (int i = 0; i <= 9; ++i) {
	assert_int_equal(bandindexarray[i], 9 - i);
    }
}

void test_one_points(void **state) {
    int rc = call_parse_logcfg("ONE_POINT\n");
    assert_int_equal(rc, 0);
    assert_int_equal(contest->points.type, FIXED);
    assert_int_equal(contest->points.point, 1);
}
void test_two_points(void **state) {
    int rc = call_parse_logcfg("TWO_POINTS\n");
    assert_int_equal(rc, 0);
    assert_int_equal(contest->points.type, FIXED);
    assert_int_equal(contest->points.point, 2);
}

void test_three_points(void **state) {
    int rc = call_parse_logcfg("THREE_POINTS\n");
    assert_int_equal(rc, 0);
    assert_int_equal(contest->points.type, FIXED);
    assert_int_equal(contest->points.point, 3);
}

void test_bandmap(void **state) {
    int rc = call_parse_logcfg("BANDMAP\n");
    assert_int_equal(rc, 0);
    assert_int_equal(cluster, MAP);
    assert_int_equal(bm_config.showdupes, 1);
    assert_int_equal(bm_config.livetime, 900);
}

void test_bandmap_d100(void **state) {
    int rc = call_parse_logcfg("BANDMAP=D,100\n");
    assert_int_equal(rc, 0);
    assert_int_equal(cluster, MAP);
    assert_int_equal(bm_config.showdupes, 0);
    assert_int_equal(bm_config.livetime, 100);
}

void test_cwspeed(void **state) {
    int rc = call_parse_logcfg("CWSPEED= 18 \n");
    assert_int_equal(rc, 0);
    assert_int_equal(wpm_spy, 18);
}

void test_cwtone(void **state) {
    int rc = call_parse_logcfg("CWTONE=765\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(tonestr, "765");
}

void test_txdelay(void **state) {
    int rc = call_parse_logcfg("TXDELAY=28\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(txdelay, 28);
}

void test_sunspots(void **state) {
    int rc = call_parse_logcfg("SUNSPOTS=123\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal((int)(ssn_r * 1000), 123 * 1000);
}

void test_sfi(void **state) {
    int rc = call_parse_logcfg("SFI=160\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal((int)(ssn_r * 1000), 100 * 1000);
}

void test_tncport(void **state) {
    int rc = call_parse_logcfg("TNCPORT=/dev/ttyUSB1\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(tncportname, "/dev/ttyUSB1\n"); // FIXME NL...
}

void test_rigmodel(void **state) {
    int rc = call_parse_logcfg("RIGMODEL=123\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(myrig_model, 123);
}

void test_addnode(void **state) {
    int rc = call_parse_logcfg("ADDNODE=hostx:1234\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(lan_active, true);
    assert_int_equal(nodes, 1);
    assert_string_equal(bc_hostaddress[0], "hostx");
    assert_string_equal(bc_hostservice[0], "1234");
}

void test_thisnode(void **state) {
    int rc = call_parse_logcfg("THISNODE= C\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(thisnode, 'C');
}

void test_mult_list(void **state) {
    int rc = call_parse_logcfg("MULT_LIST=mfile.txt\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(multlist, 1);
    assert_string_equal(multsfile, "mfile.txt");
}

void test_markers(void **state) {
    int rc = call_parse_logcfg("MARKERS=m.txt\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(xplanet, MARKER_ALL);
    assert_string_equal(markerfile, "m.txt");
}

void test_markerdots(void **state) {
    int rc = call_parse_logcfg("MARKERDOTS=md.txt\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(xplanet, MARKER_DOTS);
    assert_string_equal(markerfile, "md.txt");
}

void test_markercalls(void **state) {
    int rc = call_parse_logcfg("MARKERCALLS=mc.txt\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(xplanet, MARKER_CALLS);
    assert_string_equal(markerfile, "mc.txt");
}

void test_dx_n_sections(void **state) {
    strcpy(whichcontest, "abc");
    int rc = call_parse_logcfg(" DX_&_SECTIONS \n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(dx_arrlsections, 1);
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_countrylist(void **state) {
    strcpy(whichcontest, "abc");
    strcpy(my.call, "GM1ABC");
    int rc = call_parse_logcfg("COUNTRYLIST=G, GM , F\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(countrylist[0], "G");
    assert_string_equal(countrylist[1], "GM");
    assert_string_equal(countrylist[2], "F");
    assert_string_equal(countrylist[3], "");
    assert_true(mult_side);
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_countrylist_from_file(void **state) {
    strcpy(my.call, "EB1ABC");
    strcpy(whichcontest, "bdx");
    int rc = call_parse_logcfg("COUNTRYLIST= data/countries.txt \n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(countrylist[0], "EA");
    assert_string_equal(countrylist[1], "CT");
    assert_string_equal(countrylist[2], "");
    assert_true(mult_side);
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_countrylist_points(void **state) {
    int rc = call_parse_logcfg("COUNTRY_LIST_POINTS=4\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(countrylist_points, 4);
}

void test_countrylist_only(void **state) {
    int rc = call_parse_logcfg("USE_COUNTRYLIST_ONLY\n");
    assert_int_equal(rc, PARSE_OK);
    assert_true(countrylist_only);
}

void test_countrylist_only_mult_side(void **state) {
    mult_side = true;
    int rc = call_parse_logcfg("USE_COUNTRYLIST_ONLY\n");
    assert_int_equal(rc, PARSE_OK);
    assert_false(countrylist_only);
}

void test_my_country_points(void **state) {
    int rc = call_parse_logcfg("MY_COUNTRY_POINTS=4\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(my_country_points, 4);
}

void test_my_continent_points(void **state) {
    int rc = call_parse_logcfg("MY_CONTINENT_POINTS=3\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(my_cont_points, 3);
}

void test_dx_points(void **state) {
    int rc = call_parse_logcfg("DX_POINTS=5\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(dx_cont_points, 5);
}

void test_syncfile(void **state) {
    int rc = call_parse_logcfg("SYNCFILE = a.log\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(synclogfile, "a.log");
}

void test_sidetone_volume(void **state) {
    int rc = call_parse_logcfg("SIDETONE_VOLUME = 63\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(sc_volume, "63");
}

void test_sc_device(void **state) {
    int rc = call_parse_logcfg("SC_DEVICE = abc\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(sc_device, "abc");
}

void test_mfj1278_keyer(void **state) {
    int rc = call_parse_logcfg("MFJ1278_KEYER = qwe\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(cwkeyer, MFJ1278_KEYER);
    assert_int_equal(digikeyer, MFJ1278_KEYER);
    assert_string_equal(controllerport, "qwe");
}

void test_clusterlogin(void **state) {
    int rc = call_parse_logcfg("CLUSTERLOGIN = ab1cde\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(clusterlogin, "ab1cde\r\n");
}

void test_initial_exchange(void **state) {
    int rc = call_parse_logcfg("INITIAL_EXCHANGE = abcde\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(exchange_list, "abcde");
}

void test_cwbandwidth(void **state) {
    int rc = call_parse_logcfg("CWBANDWIDTH = 350\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(cw_bandwidth, 350);
}

void test_change_rst(void **state) {
    int rc = call_parse_logcfg("CHANGE_RST= 55, 33 \r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_true(change_rst);
    assert_string_equal(rst_init_spy, "55, 33");
}

void test_change_rst_no_arg(void **state) {
    int rc = call_parse_logcfg("CHANGE_RST\r\n");
    assert_int_equal(rc, PARSE_OK);
    assert_true(change_rst);
    assert_string_equal(rst_init_spy, "(NULL)");
}

void test_gmfsk(void **state) {
    int rc = call_parse_logcfg("GMFSK=jkl\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(digikeyer, GMFSK);
    assert_string_equal(controllerport, "jkl");
}

void test_rttymode(void **state) {
    int rc = call_parse_logcfg("RTTYMODE\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(trxmode, DIGIMODE);
    assert_string_equal(modem_mode, "RTTY");
}

void test_digimodem(void **state) {
    int rc = call_parse_logcfg("DIGIMODEM=qwerty\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(rttyoutput, "qwerty");
}

void test_myqra(void **state) {
    int rc = call_parse_logcfg("MYQRA=JN97\n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(my.qra, "JN97\n");  // FIXME NL...
}

void test_powermult(void **state) {
    int rc = call_parse_logcfg("POWERMULT=8.3\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal((int)(fixedmult * 1000), 8300);
}

void test_qtc_recv(void **state) {
    int rc = call_parse_logcfg("QTC= RECV \n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(qtcdirection, RECV);
}

void test_qtc_send(void **state) {
    int rc = call_parse_logcfg("QTC=SEND \n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(qtcdirection, SEND);
}

void test_qtc_both(void **state) {
    int rc = call_parse_logcfg("QTC=BOTH\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(qtcdirection, RECV | SEND);
}

void test_continent_list_points(void **state) {
    int rc = call_parse_logcfg("CONTINENT_LIST_POINTS=6\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(continentlist_points, 6);
}

void test_continentlist(void **state) {
    strcpy(whichcontest, "abc");
    int rc = call_parse_logcfg("CONTINENTLIST=NA, SA \n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(continent_multiplier_list[0], "NA");
    assert_string_equal(continent_multiplier_list[1], "SA");
    assert_string_equal(continent_multiplier_list[2], "");
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_continentlist_from_file(void **state) {
    strcpy(whichcontest, "aadx");
    int rc = call_parse_logcfg("CONTINENTLIST= data/continents.txt \n");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(continent_multiplier_list[0], "AS");
    assert_string_equal(continent_multiplier_list[1], "");
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_bandweight_points(void **state) {
    int rc = call_parse_logcfg("BANDWEIGHT_POINTS=160:3,80:2,40:1,20:1,15:1,10:2\n");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(bandweight_points[BANDINDEX_160], 3);
    assert_int_equal(bandweight_points[BANDINDEX_80], 2);
    assert_int_equal(bandweight_points[BANDINDEX_60], 0);
    assert_int_equal(bandweight_points[BANDINDEX_40], 1);
    assert_int_equal(bandweight_points[BANDINDEX_30], 0);
    assert_int_equal(bandweight_points[BANDINDEX_20], 1);
    assert_int_equal(bandweight_points[BANDINDEX_17], 0);
    assert_int_equal(bandweight_points[BANDINDEX_15], 1);
    assert_int_equal(bandweight_points[BANDINDEX_12], 0);
    assert_int_equal(bandweight_points[BANDINDEX_10], 2);
    assert_int_equal(bandweight_points[BANDINDEX_OOB], 0);
}

void test_bandweight_multis(void **state) {
    int rc = call_parse_logcfg("BANDWEIGHT_MULTIS=80:4,40:3,20:2,15:2,10:2");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(bandweight_multis[BANDINDEX_160], 0);
    assert_int_equal(bandweight_multis[BANDINDEX_80], 4);
    assert_int_equal(bandweight_multis[BANDINDEX_60], 0);
    assert_int_equal(bandweight_multis[BANDINDEX_40], 3);
    assert_int_equal(bandweight_multis[BANDINDEX_30], 0);
    assert_int_equal(bandweight_multis[BANDINDEX_20], 2);
    assert_int_equal(bandweight_multis[BANDINDEX_17], 0);
    assert_int_equal(bandweight_multis[BANDINDEX_15], 2);
    assert_int_equal(bandweight_multis[BANDINDEX_12], 0);
    assert_int_equal(bandweight_multis[BANDINDEX_10], 2);
}

void test_pfx_num_multis(void **state) {
    strcpy(whichcontest, "abc");
    int rc = call_parse_logcfg("PFX_NUM_MULTIS=W,VE,VK,ZL,ZS,JA,PY,UA9");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(pfxnummultinr, 8);
    assert_int_equal(pfxnummulti[0].countrynr, 18);
    assert_int_equal(pfxnummulti[1].countrynr, 17);
    assert_int_equal(pfxnummulti[2].countrynr, 16);
    assert_int_equal(pfxnummulti[3].countrynr, 15);
    assert_int_equal(pfxnummulti[4].countrynr, 14);
    assert_int_equal(pfxnummulti[5].countrynr, 13);
    assert_int_equal(pfxnummulti[6].countrynr, 12);
    assert_int_equal(pfxnummulti[7].countrynr, 11);
    assert_int_equal(pfxnummulti[8].countrynr, 0);
    assert_int_equal(CONTEST_IS(UNKNOWN), 1);
}

void test_qtcrec_record_command(void **state) {
    int rc = call_parse_logcfg("QTCREC_RECORD_COMMAND=rec -r 8000 $ -q");
    assert_int_equal(rc, PARSE_OK);
    assert_string_equal(qtcrec_record_command[0], "rec -r 8000 ");
    assert_string_equal(qtcrec_record_command[1], " -q &");
    assert_string_equal(qtcrec_record_command_shutdown, "rec");
}

void test_exclude_multilist_continentlist(void **state) {
    strcpy(continent_multiplier_list[0], "EU");
    int rc = call_parse_logcfg("EXCLUDE_MULTILIST=CONTINENTLIST");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(exclude_multilist_type, EXCLUDE_CONTINENT);
}

void test_exclude_multilist_countrylist(void **state) {
    strcpy(countrylist[0], "SM");
    int rc = call_parse_logcfg("EXCLUDE_MULTILIST=COUNTRYLIST");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(exclude_multilist_type, EXCLUDE_COUNTRY);
}

void test_fldigi(void **state) {
#ifdef HAVE_LIBXMLRPC
    int rc = call_parse_logcfg("FLDIGI=http://host:1234/RPC2");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(digikeyer, FLDIGI);
    assert_true(fldigi_isenabled());
#endif
}

void test_rigptt(void **state) {
    int rc = call_parse_logcfg("RIGPTT");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(rigptt, 1);
}

void test_minitest_no_arg(void **state) {
    int rc = call_parse_logcfg("MINITEST");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(minitest, MINITEST_DEFAULT_PERIOD);
}

void test_minitest(void **state) {
    int rc = call_parse_logcfg("MINITEST=1200");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(minitest, 1200);
}

void test_tune_seconds(void **state) {
    int rc = call_parse_logcfg("TUNE_SECONDS=73");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(tune_seconds, 73);
}

void test_unique_call_multi_all(void **state) {
    int rc = call_parse_logcfg("UNIQUE_CALL_MULTI=ALL");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(unique_call_multi, UNIQUECALL_ALL);
}

void test_unique_call_multi_band(void **state) {
    int rc = call_parse_logcfg("UNIQUE_CALL_MULTI=BAND");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(unique_call_multi, UNIQUECALL_BAND);
}

void test_digi_rig_mode_usb(void **state) {
    int rc = call_parse_logcfg("DIGI_RIG_MODE=USB");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(digi_mode, RIG_MODE_USB);
}

void test_digi_rig_mode_lsb(void **state) {
    int rc = call_parse_logcfg("DIGI_RIG_MODE=LSB");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(digi_mode, RIG_MODE_LSB);
}

void test_digi_rig_mode_rtty(void **state) {
    int rc = call_parse_logcfg("DIGI_RIG_MODE=RTTY");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(digi_mode, RIG_MODE_RTTY);
}

void test_digi_rig_mode_rttyr(void **state) {
    int rc = call_parse_logcfg("DIGI_RIG_MODE=RTTYR");
    assert_int_equal(rc, PARSE_OK);
    assert_int_equal(digi_mode, RIG_MODE_RTTYR);
}
