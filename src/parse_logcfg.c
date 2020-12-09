/*
* Tlf - contest logging program for amateur radio operators
* Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0rct@amsat.org>
* 		2011-2020           Thomas Beierlein <tb@forth-ev.de>
* 		2013 		    Fred DH5FS
*               2013-2016           Ervin Hegedus - HA2OS <airween@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <ctype.h>
#include <hamlib/rig.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "bandmap.h"
#include "change_rst.h"
#include "cw_utils.h"
#include "fldigixmlrpc.h"
#include "globalvars.h"
#include "getctydata.h"
#include "getpx.h"
#include "getwwv.h"
#include "lancode.h"
#include "locator2longlat.h"
#include "parse_logcfg.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "setcontest.h"
#include "startmsg.h"
#include "tlf_curses.h"
#include "searchlog.h"

#include <config.h>


extern bool mixedmode;
extern bool ignoredupe;
extern bool continentlist_only;
extern int lan_port;
extern char rigconf[];
extern char ph_message[14][80];
extern int cwkeyer;
extern int digikeyer;
extern int keyer_backspace;
extern int partials;
extern int use_part;
extern int portnum;
extern int packetinterface;
extern int tncport;
extern int shortqsonr;
extern char *cabrillo;
extern rmode_t digi_mode;
extern int ctcomp;
extern int recall_mult;
extern int trx_control;
extern int rit;
extern int showscore_flag;
extern int searchflg;
extern int demode;
extern int portable_x2;
extern int landebug;
extern int call_update;
extern int nob4;
extern int time_master;
extern int show_time;
extern int use_rxvt;
extern int exc_cont;
extern int noautocq;
extern int bmautoadd;
extern int bmautograb;
extern int logfrequency;
extern int no_rst;
extern int serial_or_section;
extern int sprint_mode;
extern int sc_sidetone;
extern int no_arrows;
extern int lowband_point_mult;
extern int cluster;
extern int clusterlog;
extern char keyer_device[10];
extern int timeoffset;
extern int netkeyer_port;
extern char netkeyer_hostaddress[];
extern char *rigportname;
extern int tnc_serial_rate;
extern int serial_rate;
extern char pr_hostaddress[];
extern char *editor_cmd;
extern int cqdelay;
extern int ssbpoints;
extern int cwpoints;

bool exist_in_country_list();

void KeywordRepeated(char *keyword);
void KeywordNotSupported(char *keyword);
void ParameterNeeded(const char *keyword);
void WrongFormat(const char *keyword);
void WrongFormat_details(const char *keyword, const char *details);

#define  MAX_COMMANDS (sizeof(commands) / sizeof(*commands))	/* commands in list */


int read_logcfg(void) {

    extern int nodes;
    extern int node;
    extern char *config_file;

    char defltconf[80];

    int status;
    int i;
    FILE *fp;

    iscontest = false;
    partials = 0;
    use_part = 0;
    cwkeyer = NO_KEYER;
    digikeyer = NO_KEYER;
    portnum = 0;
    packetinterface = 0;
    tncport = 0;
    nodes = 0;
    node = 0;
    shortqsonr = 0;

    /* Disable CT Mode until CTCOMPATIBLE is defined. */
    ctcomp = 0;

    for (i = 0; i < 25; i++) {
	if (digi_message[i] != NULL) {
	    free(digi_message[i]);
	    digi_message[i] = NULL;
	}
    }
    if (cabrillo != NULL) {
	free(cabrillo);
	cabrillo = NULL;
    }

    strcpy(defltconf, PACKAGE_DATA_DIR);
    strcat(defltconf, "/logcfg.dat");

    if (config_file == NULL)
	config_file = g_strdup("logcfg.dat");

    if ((fp = fopen(config_file, "r")) == NULL) {
	if ((fp = fopen(defltconf, "r")) == NULL) {
	    showmsg("Error opening logcfg.dat file.");
	    showmsg("Exiting...");
	    sleep(5);
	    endwin();
	    exit(1);
	} else {
	    showstring("Using default (Read Only) config file:", defltconf);
	}

    } else
	showstring("Reading config file:", config_file);

    status = parse_configfile(fp);
    fclose(fp);

    return status;
}

static bool isCommentLine(char *buffer) {
    if ((buffer[0] != '#') && (buffer[0] != ';') && (strlen(buffer) > 1)) {
	return false;
    } else {
	return true;
    }
}

int parse_configfile(FILE *fp) {
    int status = PARSE_OK;
    char buffer[160];

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
	/* skip comments and empty lines */
	if (!isCommentLine(buffer)) {
	    status |= parse_logcfg(buffer);
	}
    }

    return status;
}

/** convert band string into index number (0..NBANDS-1) */
int getidxbybandstr(char *confband) {
    static char bands_strings[NBANDS][4] = {"160", "80", "60", "40", "30", "20", "17", "15", "12", "10"};
    int i;

    g_strchomp(confband);

    for (i = 0; i < NBANDS; i++) {
	if (strcmp(confband, g_strchomp(bands_strings[i])) == 0) {
	    return i;
	}
    }
    return -1;
}

////////////////////
// global variables for matcher functions:
GMatchInfo *match_info;
const char *parameter;

static int parse_int(const char *string, gint64 min, gint64 max, int *result) {

    gchar *str = g_strdup(string);
    g_strstrip(str);

    gint64 value;
    GError *error = NULL;
    gboolean ok = g_ascii_string_to_signed(str, 10, min, max, &value, &error);

    g_free(str);

    if (ok) {
	*result = (int)value;
	return PARSE_OK;
    }

    int rc = PARSE_INVALID_INTEGER;
    if (error != NULL) {
	if (error->code == G_NUMBER_PARSER_ERROR_OUT_OF_BOUNDS) {
	    rc = PARSE_INTEGER_OUT_OF_RANGE;
	}
	g_free(error);
    }

    return rc;
}

int cfg_bool_const(const cfg_arg_t arg) {
    *arg.bool_p = arg.bool_value;
    return PARSE_OK;
}

int cfg_int_const(const cfg_arg_t arg) {
    *arg.int_p  = arg.int_value;
    return PARSE_OK;
}

int cfg_integer(const cfg_arg_t arg) {
    return parse_int(parameter, (gint64)arg.min, (gint64)arg.max, arg.int_p);
}

//
// static: char_p, size > 0, base not used
// dynamic: char_pp, size optional, base optional
// message: msg, size > 0, base used
//
int cfg_string(const cfg_arg_t arg) {
    gchar *index = g_match_info_fetch(match_info, 1);
    int n = 0;
    if (NULL != index) {
	n = atoi(index);    // use provided index
	g_free(index);
    }

    char *str = g_strdup(parameter);
    // chomp/strip
    if (arg.chomp) {
	g_strchomp(str);
    }
    if (arg.strip) {
	g_strstrip(str);
    }
    // check length
    if (arg.size > 0 && strlen(str) >= arg.size) {
	g_free(str);
	return PARSE_STRING_TOO_LONG;
    }
    // replace trailing newline with a space
    if (arg.nl_to_space) {
	char *nl = strrchr(str, '\n');
	if (nl) {
	    *nl = ' ';
	}
    }

    // store value
    switch (arg.string_type) {
	case STATIC:
	    g_strlcpy(arg.char_p, str, arg.size);
	    g_free(str);
	    break;
	case MESSAGE:
	    g_strlcpy(arg.msg[arg.base + n], str, arg.size);
	    g_free(str);
	    break;
	case DYNAMIC:
	    if (arg.char_pp[arg.base + n] != NULL) {
		g_free(arg.char_pp[arg.base + n]);
	    }
	    arg.char_pp[arg.base + n] = str;
    }
    return PARSE_OK;
}

static int cfg_telnetport(const cfg_arg_t arg) {
    int rc = cfg_integer((cfg_arg_t) {.int_p = &portnum, .min = 1, .max = INT32_MAX});
    if (rc != PARSE_OK) {
	return rc;
    }
    packetinterface = TELNET_INTERFACE;
    return PARSE_OK;
}

static config_t logcfg_configs[] = {
    {"CONTEST_MODE",        CFG_BOOL_TRUE(iscontest)},
    {"MIXED",               CFG_BOOL_TRUE(mixedmode)},
    {"IGNOREDUPE",          CFG_BOOL_TRUE(ignoredupe)},
    {"USE_CONTINENTLIST_ONLY",  CFG_BOOL_TRUE(continentlist_only)},

    {"USEPARTIALS",     CFG_INT_ONE(use_part)},
    {"PARTIALS",        CFG_INT_ONE(partials)},
    {"RECALL_MULTS",    CFG_INT_ONE(recall_mult)},
    {"WYSIWYG_MULTIBAND",   CFG_INT_ONE(wysiwyg_multi)},
    {"WYSIWYG_ONCE",    CFG_INT_ONE(wysiwyg_once)},
    {"RADIO_CONTROL",   CFG_INT_ONE(trx_control)},
    {"RIT_CLEAR",       CFG_INT_ONE(rit)},
    {"SHORT_SERIAL",    CFG_INT_ONE(shortqsonr)},
    {"SCOREWINDOW",     CFG_INT_ONE(showscore_flag)},
    {"CHECKWINDOW",     CFG_INT_ONE(searchflg)},
    {"SEND_DE",         CFG_INT_ONE(demode)},
    {"SERIAL_EXCHANGE", CFG_INT_ONE(exchange_serial)},
    {"COUNTRY_MULT",    CFG_INT_ONE(country_mult)},
    {"PORTABLE_MULT_2", CFG_INT_ONE(portable_x2)},
    {"CQWW_M2",         CFG_INT_ONE(cqwwm2)},
    {"LAN_DEBUG",       CFG_INT_ONE(landebug)},
    {"CALLUPDATE",      CFG_INT_ONE(call_update)},
    {"TIME_MASTER",     CFG_INT_ONE(time_master)},
    {"CTCOMPATIBLE",    CFG_INT_ONE(ctcomp)},
    {"SERIAL\\+SECTION",    CFG_INT_ONE(serial_section_mult)},
    {"SECTION_MULT",    CFG_INT_ONE(sectn_mult)},
    {"NOB4",            CFG_INT_ONE(nob4)},
    {"SHOW_TIME",       CFG_INT_ONE(show_time)},
    {"RXVT",            CFG_INT_ONE(use_rxvt)},
    {"WAZMULT",         CFG_INT_ONE(wazmult)},
    {"ITUMULT",         CFG_INT_ONE(itumult)},
    {"CONTINENT_EXCHANGE",  CFG_INT_ONE(exc_cont)},
    {"NOAUTOCQ",        CFG_INT_ONE(noautocq)},
    {"NO_BANDSWITCH_ARROWKEYS", CFG_INT_ONE(no_arrows)},
    {"SOUNDCARD",       CFG_INT_ONE(sc_sidetone)},
    {"LOWBAND_DOUBLE",  CFG_INT_ONE(lowband_point_mult)},
    {"CLUSTER_LOG",     CFG_INT_ONE(clusterlog)},
    {"SERIAL\\+GRID4",  CFG_INT_ONE(serial_grid4_mult)},
    {"LOGFREQUENCY",    CFG_INT_ONE(logfrequency)},
    {"NO_RST",          CFG_INT_ONE(no_rst)},
    {"SERIAL_OR_SECTION",   CFG_INT_ONE(serial_or_section)},
    {"PFX_MULT_MULTIBAND",  CFG_INT_ONE(pfxmultab)},
    {"QTCREC_RECORD",   CFG_INT_ONE(qtcrec_record)},
    {"QTC_AUTO_FILLTIME",   CFG_INT_ONE(qtc_auto_filltime)},
    {"BMAUTOGRAB",      CFG_INT_ONE(bmautograb)},
    {"BMAUTOADD",       CFG_INT_ONE(bmautoadd)},
    {"QTC_RECV_LAZY",   CFG_INT_ONE(qtc_recv_lazy)},
    {"SPRINTMODE",      CFG_INT_ONE(sprint_mode)},
    {"KEYER_BACKSPACE", CFG_INT_ONE(keyer_backspace)},
    {"SECTION_MULT_ONCE",   CFG_INT_ONE(sectn_mult_once)},

    {"F([1-9]|1[0-2])", CFG_MESSAGE(message, -1)},  // index is 1-based
    {"S&P_TU_MSG",      CFG_MESSAGE(message, SP_TU_MSG)},
    {"CQ_TU_MSG",       CFG_MESSAGE(message, CQ_TU_MSG)},
    {"ALT_([0-9])",     CFG_MESSAGE(message, CQ_TU_MSG + 1)},
    {"S&P_CALL_MSG",    CFG_MESSAGE(message, SP_CALL_MSG)},

    {"VKM([1-9]|1[0-2])",   CFG_MESSAGE_CHOMP(ph_message, -1)},
    {"VKCQM",               CFG_MESSAGE_CHOMP(ph_message, CQ_TU_MSG)},
    {"VKSPM",               CFG_MESSAGE_CHOMP(ph_message, SP_TU_MSG)},

    {"DKF([1-9]|1[0-2])",   CFG_MESSAGE_DYNAMIC(digi_message, -1)},
    {"DKCQM",               CFG_MESSAGE_DYNAMIC(digi_message, CQ_TU_MSG)},
    {"DKSPM",               CFG_MESSAGE_DYNAMIC(digi_message, SP_TU_MSG)},
    {"DKSPC",               CFG_MESSAGE_DYNAMIC(digi_message, SP_CALL_MSG)},
    {"ALT_DK([1-9]|10)",    CFG_MESSAGE_DYNAMIC(digi_message, CQ_TU_MSG)},

    {"QR_F([1-9]|1[0-2])",      CFG_MESSAGE(qtc_recv_msgs, -1) },
    {"QR_VKM([1-9]|1[0-2])",    CFG_MESSAGE_CHOMP(qtc_phrecv_message, -1) },
    {"QR_VKCQM",                CFG_MESSAGE_CHOMP(qtc_phrecv_message, CQ_TU_MSG) },
    {"QR_VKSPM",                CFG_MESSAGE_CHOMP(qtc_phrecv_message, SP_TU_MSG) },

    {"QS_F([1-9]|1[0-2])",      CFG_MESSAGE(qtc_send_msgs, -1) },
    {"QS_VKM([1-9]|1[0-2])",    CFG_MESSAGE_CHOMP(qtc_phsend_message, -1) },
    {"QS_VKCQM",                CFG_MESSAGE_CHOMP(qtc_phsend_message, CQ_TU_MSG) },
    {"QS_VKSPM",                CFG_MESSAGE_CHOMP(qtc_phsend_message, SP_TU_MSG) },

    {"LAN_PORT",        CFG_INT(lan_port, 1000, INT32_MAX)},
    {"TIME_OFFSET",     CFG_INT(timeoffset, -23, 23)},
    {"NETKEYERPORT",    CFG_INT(netkeyer_port, 1, INT32_MAX)},
    {"TNCSPEED",        CFG_INT(tnc_serial_rate, 0, INT32_MAX)},
    {"RIGSPEED",        CFG_INT(serial_rate, 0, INT32_MAX)},
    {"CQDELAY",         CFG_INT(cqdelay, 3, 60)},
    {"SSBPOINTS",       CFG_INT(ssbpoints, 0, INT32_MAX)},
    {"CWPOINTS",        CFG_INT(cwpoints, 0, INT32_MAX)},

    {"NETKEYER",        CFG_INT_CONST(cwkeyer, NET_KEYER)},
    {"FIFO_INTERFACE",  CFG_INT_CONST(packetinterface, FIFO_INTERFACE)},
    {"LONG_SERIAL",     CFG_INT_CONST(shortqsonr, 0)},
    {"CLUSTER",         CFG_INT_CONST(cluster, CLUSTER)},

//    {"CALL",        CFG_STRING_STATIC(my.call, 20) },
    {"RIGCONF",         CFG_STRING_STATIC(rigconf, 80)},
    {"LOGFILE",         CFG_STRING_STATIC(logfile, 120)},
    {"KEYER_DEVICE",    CFG_STRING_STATIC(keyer_device, 10)},
    {"NETKEYERHOST",    CFG_STRING_STATIC(netkeyer_hostaddress, 16)},
    {"TELNETHOST",      CFG_STRING_STATIC(pr_hostaddress, 48)},
    {"QTC_CAP_CALLS",   CFG_STRING_STATIC(qtc_cap_calls, 40)},

    {"CABRILLO",    CFG_STRING(cabrillo)},
    {"CALLMASTER",  CFG_STRING(callmaster_filename)},
    {"EDITOR",      CFG_STRING(editor_cmd)},

    {"RIGPORT",  CFG_STRING_NOCHOMP(rigportname)},

    {"TELNETPORT",  NEED_PARAM, cfg_telnetport},

    {NULL}  // end marker
};


static int check_match(const config_t *cfg, const char *keyword) {
    gchar *pattern = g_strdup_printf("^%s$", cfg->regex);
    GRegex *regex = g_regex_new(pattern, 0, 0, NULL);
    g_free(pattern);

    int result = PARSE_NO_MATCH;    // default: not found

    g_regex_match(regex, keyword, 0, &match_info);
    if (g_match_info_matches(match_info)) {

	if (cfg->param_kind == NEED_PARAM && parameter == NULL) {
	    result = PARSE_MISSING_PARAMETER;
// -- no error at the moment
//        } else if (cfg->param_kind == NO_PARAM && parameter != NULL) {
//            result = PARSE_EXTRA_PARAMETER;
	} else {
	    result = cfg->func(cfg->arg);
	}
    }
    g_match_info_free(match_info);
    g_regex_unref(regex);

    return result;
}

static int apply_config(const char *keyword, const char *param,
			const config_t *configs) {

    parameter = param;  // save for matcher functions

    int result = PARSE_NO_MATCH;

    for (const config_t *cfg = configs; cfg->regex ; ++cfg) {
	result = check_match(cfg, keyword);
	if (result != PARSE_NO_MATCH) {
	    break;
	}
    }

    switch (result) {
	case PARSE_OK:
	    return PARSE_OK;

	case PARSE_NO_MATCH:
	    return PARSE_NO_MATCH;
	//KeywordNotSupported(keyword);
	//break;

	case PARSE_MISSING_PARAMETER:
	    ParameterNeeded(keyword);
	    break;

	case PARSE_INVALID_INTEGER:
	    WrongFormat_details(keyword, "invalid number");
	    break;

	case PARSE_INTEGER_OUT_OF_RANGE:
	    WrongFormat_details(keyword, "value out of range");
	    break;

	default:
	    WrongFormat(keyword);
    }

    return PARSE_CONFIRM;
}
////////////////////

static int confirmation_needed;


#define PARAMETER_NEEDED(x) 			\
    do {					\
	if (fields[1] == NULL) { 		\
	    ParameterNeeded(x); 		\
    	    g_strfreev( fields );		\
	    return( confirmation_needed ); 				\
	}					\
    } while(0)

int parse_logcfg(char *inputbuffer) {

    extern int use_rxvt;
    extern char message[][80];
    extern char ph_message[14][80];
    extern char whichcontest[];
    extern char logfile[];
    extern int recall_mult;
    extern int one_point;
    extern int two_point;
    extern int three_point;
    extern int exchange_serial;
    extern int country_mult;
    extern int wysiwyg_multi;
    extern int wysiwyg_once;
    extern float fixedmult;
    extern int portable_x2;
    extern int trx_control;
    extern int rit;
    extern int shortqsonr;
    extern int cluster;
    extern int clusterlog;
    extern int showscore_flag;
    extern int searchflg;
    extern int demode;
    extern bool iscontest;
    extern int weight;
    extern int txdelay;
    extern char tonestr[];
    extern char *editor_cmd;
    extern int partials;
    extern int use_part;
    extern bool mixedmode;
    extern char pr_hostaddress[];
    extern int portnum;
    extern int packetinterface;
    extern int tncport;
    extern int tnc_serial_rate;
    extern int serial_rate;
    extern rig_model_t myrig_model;
    extern char *rigportname;
    extern int rignumber;
    extern char rigconf[];
    extern char exchange_list[40];
    extern char tncportname[];
    extern int netkeyer_port;
    extern char netkeyer_hostaddress[];
    extern char bc_hostaddress[MAXNODES][16];
    extern char bc_hostservice[MAXNODES][16];
    extern int lan_active;
    extern char thisnode;
    extern int nodes;
    extern int node;
    extern int cqwwm2;
    extern int landebug;
    extern int call_update;
    extern int timeoffset;
    extern int time_master;
    extern char multsfile[];
    extern int multlist;
    extern int universal;
    extern int serial_section_mult;
    extern int serial_grid4_mult;
    extern int sectn_mult;
    extern int sectn_mult_once;
    extern int dx_arrlsections;
    extern int pfxmult;
    extern int exc_cont;
    extern char markerfile[];
    extern int xplanet;
    extern int nob4;
    extern int noautocq;
    extern int show_time;
    extern char keyer_device[10];
    extern int wazmult;
    extern int itumult;
    extern int cqdelay;
    extern int trxmode;
    extern int use_bandoutput;
    extern int no_arrows;
    extern int bandindexarray[];
    extern int ssbpoints;
    extern int cwpoints;
    extern int lowband_point_mult;
    extern int sc_sidetone;
    extern char sc_volume[];
    extern char modem_mode[];
    extern int no_rst;
    extern int serial_or_section;

    /* LZ3NY mods */
    extern int my_country_points;
    extern int my_cont_points;
    extern int dx_cont_points;
    extern int countrylist_points;
    extern bool countrylist_only;
    extern int continentlist_points;
    extern bool continentlist_only;
    char c_temp[11];
    extern bool mult_side;
    extern char countrylist[][6];
    extern char continent_multiplier_list[7][3];
    extern int exclude_multilist_type;

    /* end LZ3NY mods */
    extern int tlfcolors[8][2];
    extern char synclogfile[];
    extern char sc_device[40];
    extern char controllerport[80];	// port for multi-mode controller
    extern char clusterlogin[];
    extern int cw_bandwidth;
    extern char rttyoutput[];
    extern int logfrequency;
    extern bool ignoredupe;
    extern int bandweight_points[NBANDS];
    extern int bandweight_multis[NBANDS];
    extern pfxnummulti_t pfxnummulti[MAXPFXNUMMULT];
    extern int pfxnummultinr;
    extern int pfxmultab;
    extern int bmautoadd;
    extern int bmautograb;
    extern int sprint_mode;
#ifdef HAVE_LIBXMLRPC
    extern char fldigi_url[50];
#endif
    extern unsigned char rigptt;
    extern int minitest;
    extern int unique_call_multi;
    extern int lan_port;
    extern int verbose;

    char *commands[] = {
	NULL,   //"enable",		/* 0 */		/* deprecated */
	NULL,   //"disable",				/* deprecated */
	NULL,   //"F1",
	NULL,   //"F2",
	NULL,   //"F3",
	NULL,   //"F4",			/* 5 */
	NULL,   //"F5",
	NULL,   //"F6",
	NULL,   //"F7",
	NULL,   //"F8",
	NULL,   //"F9",			/* 10 */
	NULL,   //"F10",
	NULL,   //"F11",
	NULL,   //"F12",
	NULL,   //"S&P_TU_MSG",
	NULL,   //"CQ_TU_MSG",		/* 15 */
	"CALL",
	"CONTEST",
	NULL,   //"LOGFILE",
	NULL,   //"KEYER_DEVICE",
	"BANDOUTPUT",		/* 20 */
	NULL,   //"RECALL_MULTS",
	"ONE_POINT",
	"THREE_POINTS",
	NULL,   //"WYSIWYG_MULTIBAND",
	NULL,   //"WYSIWYG_ONCE",		/* 25 */
	NULL,   //"RADIO_CONTROL",
	NULL,   //"RIT_CLEAR",
	NULL,   //"SHORT_SERIAL",
	NULL,   //"LONG_SERIAL",
	NULL,   //"CONTEST_MODE",		/* 30 */
	NULL,   //"CLUSTER",
	"BANDMAP",
	NULL,   //"SPOTLIST",				/* deprecated */
	NULL,   //"SCOREWINDOW",
	NULL,   //"CHECKWINDOW",		/* 35 */
	NULL,   //"FILTER",				/* deprecated */
	NULL,   //"SEND_DE",
	"CWSPEED",
	"CWTONE",
	"WEIGHT",		/* 40 */
	"TXDELAY",
	"SUNSPOTS",
	"SFI",
	NULL,   //"SHOW_FREQUENCY",                       /* deprecated */
	NULL,   //"EDITOR",		/* 45 */
	NULL,   //"PARTIALS",
	NULL,   //"USEPARTIALS",
	NULL,   //"POWERMULT_5",				/* deprecated */
	NULL,   //"POWERMULT_2",				/* deprecated */
	NULL,   //"POWERMULT_1",		/* 50 */	/* deprecated */
	NULL,   //"MANY_CALLS",				/* deprecated */
	NULL,   //"SERIAL_EXCHANGE",
	NULL,   //"COUNTRY_MULT",
	"2EU3DX_POINTS",
	NULL,   //"PORTABLE_MULT_2",	/* 55 */
	NULL,   //"MIXED",
	NULL,   //"TELNETHOST",
	NULL,   //"TELNETPORT",
	"TNCPORT",
	NULL,   //"FIFO_INTERFACE",	/* 60 */
	"RIGMODEL",
	NULL,   //"RIGSPEED",
	NULL,   //"TNCSPEED",
	NULL,   //"RIGPORT",
	NULL,   //"NETKEYER",		/* 65 */
	NULL,   //"NETKEYERPORT",
	NULL,   //"NETKEYERHOST",
	"ADDNODE",
	"THISNODE",
	NULL,   //"CQWW_M2",		/* 70 */
	NULL,   //"LAN_DEBUG",
	NULL,   //"A/LT_0",
	NULL,   //"A/LT_1",
	NULL,   //"A/LT_2",
	NULL,   //"A/LT_3",		/* 75 */
	NULL,   //"A/LT_4",
	NULL,   //"A/LT_5",
	NULL,   //"A/LT_6",
	NULL,   //"A/LT_7",
	NULL,   //"A/LT_8",		/* 80 */
	NULL,   //"ALT_9",
	NULL,   //"CALLUPDATE",
	NULL,   //"TIME_OFFSET",
	NULL,   //"TIME_MASTER",
	NULL,   //"CTCOMPATIBLE",		/*  85  */
	"TWO_POINTS",
	"MULT_LIST",
	NULL,   //"SERIAL+SECTION",
	NULL,   //"SECTION_MULT",
	"MARKERS",		/* 90 */
	"DX_&_SECTIONS",
	"MARKERDOTS",
	"MARKERCALLS",
	NULL,   //"NOB4",
	/*LZ3NY */
	"COUNTRYLIST",		//by lz3ny      /* 95 */
	"COUNTRY_LIST_POINTS",	//by lz3ny
	"USE_COUNTRYLIST_ONLY",	//by lz3ny
	"MY_COUNTRY_POINTS",	//by lz3ny
	"MY_CONTINENT_POINTS",	//by lz3ny
	"DX_POINTS",		//by lz3ny                 /* 100 */
	NULL,   //"SHOW_TIME",
	NULL,   //"RXVT",
	NULL,   //"VKM1",
	NULL,   //"VKM2",
	NULL,   //"VKM3",		/* 105 */
	NULL,   //"VKM4",
	NULL,   //"VKM5",
	NULL,   //"VKM6",
	NULL,   //"VKM7",
	NULL,   //"VKM8",		/* 110 */
	NULL,   //"VKM9",
	NULL,   //"VKM10",
	NULL,   //"VKM11",
	NULL,   //"VKM12",
	NULL,   //"VKSPM",		/* 115 */
	NULL,   //"VKCQM",
	NULL,   //"WAZMULT",
	NULL,   //"ITUMULT",
	NULL,   //"CQDELAY",
	"PFX_MULT",		/* 120 */
	NULL,   //"CONTINENT_EXCHANGE",
	"RULES",
	NULL,   //"NOAUTOCQ",
	"SSBMODE",
	NULL,   //"NO_BANDSWITCH_ARROWKEYS",	/* 125 */
	"RIGCONF",
	"TLFCOLOR1",
	"TLFCOLOR2",
	"TLFCOLOR3",
	"TLFCOLOR4",		/* 130 */
	"TLFCOLOR5",
	"TLFCOLOR6",
	"SYNCFILE",
	NULL,   //"SSBPOINTS",
	NULL,   //"CWPOINTS",		/* 135 */
	NULL,   //"SOUNDCARD",
	"SIDETONE_VOLUME",
	NULL,   //"S_METER",				/* deprecated */
	"SC_DEVICE",
	"MFJ1278_KEYER",	/* 140 */
	"CLUSTERLOGIN",
	"ORION_KEYER",
	"INITIAL_EXCHANGE",
	"CWBANDWIDTH",
	NULL,   //"LOWBAND_DOUBLE",	/* 145 */
	NULL,   //"CLUSTER_LOG",
	NULL,   //"SERIAL+GRID4",
	"CHANGE_RST",
	"GMFSK",
	"RTTYMODE",		/* 150 */
	"DIGIMODEM",
	NULL,   //"LOGFREQUENCY",
	NULL,   //"IGNOREDUPE",
	NULL,   //"CABRILLO",
	NULL,   //"CW_TU_MSG",		/* 155 */	/* deprecated */
	NULL,   //"VKCWR",				/* deprecated */
	NULL,   //"VKSPR",				/* deprecated */
	NULL,   //"NO_RST",
	"MYQRA",
	"POWERMULT",		/* 160 */
	NULL,   //"SERIAL_OR_SECTION",
	"QTC",
	"CONTINENTLIST",
	"CONTINENT_LIST_POINTS",
	NULL,   //"USE_CONTINENTLIST_ONLY",  /* 165 */
	"BANDWEIGHT_POINTS",
	"BANDWEIGHT_MULTIS",
	"PFX_NUM_MULTIS",
	NULL,   //"PFX_MULT_MULTIBAND",
	NULL,   //"QR_F1",		/* 170 */
	NULL,   //"QR_F2",
	NULL,   //"QR_F3",
	NULL,   //"QR_F4",
	NULL,   //"QR_F5",
	NULL,   //"QR_F6",		/* 175 */
	NULL,   //"QR_F7",
	NULL,   //"QR_F8",
	NULL,   //"QR_F9",
	NULL,   //"QR_F10",
	NULL,   //"QR_F11",		/* 180 */
	NULL,   //"QR_F12",
	NULL,   //"QS_F1",
	NULL,   //"QS_F2",
	NULL,   //"QS_F3",
	NULL,   //"QS_F4",
	NULL,   //"QS_F5",
	NULL,   //"QS_F6",
	NULL,   //"QS_F7",
	NULL,   //"QS_F8",
	NULL,   //"QS_F9",		/* 190 */
	NULL,   //"QS_F10",
	NULL,   //"QS_F11",
	NULL,   //"QS_F12",
	NULL,   //"QR_VKM1",
	NULL,   //"QR_VKM2",
	NULL,   //"QR_VKM3",
	NULL,   //"QR_VKM4",
	NULL,   //"QR_VKM5",
	NULL,   //"QR_VKM6",
	NULL,   //"QR_VKM7",			/* 200 */
	NULL,   //"QR_VKM8",
	NULL,   //"QR_VKM9",
	NULL,   //"QR_VKM10",
	NULL,   //"QR_VKM11",
	NULL,   //"QR_VKM12",
	NULL,   //"QR_VKSPM",
	NULL,   //"QR_VKCQM",
	NULL,   //"QS_VKM1",
	NULL,   //"QS_VKM2",
	NULL,   //"QS_VKM3",			/* 210 */
	NULL,   //"QS_VKM4",
	NULL,   //"QS_VKM5",
	NULL,   //"QS_VKM6",
	NULL,   //"QS_VKM7",
	NULL,   //"QS_VKM8",
	NULL,   //"QS_VKM9",
	NULL,   //"QS_VKM10",
	NULL,   //"QS_VKM11",
	NULL,   //"QS_VKM12",
	NULL,   //"QS_VKSPM",		/* 220 */
	NULL,   //"QS_VKCQM",
	NULL,   //"QTCREC_RECORD",
	"QTCREC_RECORD_COMMAND",
	"EXCLUDE_MULTILIST",
	NULL,   //"S&P_CALL_MSG",		/* 225 */
	NULL,   //"QTC_CAP_CALLS",
	NULL,   //"QTC_AUTO_FILLTIME",
	NULL,   //"BMAUTOGRAB",
	NULL,   //"BMAUTOADD",
	NULL,   //"QTC_RECV_LAZY",		/* 230 */
	NULL,   //"SPRINTMODE",
	"FLDIGI",
	"RIGPTT",
	"MINITEST",
	"UNIQUE_CALL_MULTI",		/* 235 */
	NULL,   //"KEYER_BACKSPACE",
	"DIGI_RIG_MODE",
	NULL,   //"DKF1",				/* 238 */
	NULL,   //"DKF2",
	NULL,   //"DKF3",
	NULL,   //"DKF4",
	NULL,   //"DKF5",
	NULL,   //"DKF6",
	NULL,   //"DKF7",
	NULL,   //"DKF8",
	NULL,   //"DKF9",
	NULL,   //"DKF10",
	NULL,   //"DKF11",
	NULL,   //"DKF12",
	NULL,   //"DKCQM",			/* 250 */
	NULL,   //"DKSPM",
	NULL,   //"DKSPC",
	NULL,   //"ALT_DK1",			/* 253 */
	NULL,   //"ALT_DK2",
	NULL,   //"ALT_DK3",
	NULL,   //"ALT_DK4",
	NULL,   //"ALT_DK5",
	NULL,   //"ALT_DK6",
	NULL,   //"ALT_DK7",
	NULL,   //"ALT_DK8",			/* 260 */
	NULL,   //"ALT_DK9",
	NULL,   //"ALT_DK10",
	NULL,   //"CALLMASTER",
	NULL,   //"LAN_PORT",                     /* 264 */
	NULL,   //"SECTION_MULT_ONCE"
    };

    char **fields;
    char teststring[80];
    char buff[40];
    int ii;
    int jj, hh;
    char *tk_ptr;


    /* split the inputline at '=' to max 2 elements
     *
     * leave the components in fields[0] (keyword) and
     * fields[1] for the parameters
     *
     * if only 1 component (no '='), it is only a keyword
     *    g_strstrip it and test for keywordlist
     *
     * if 2 components (there is a '=' in the line)
     *    g_strstrip first component and test for keywordlist
     *    g_strchug second component -> strip leading space from parameters
     *
     * That allows plain keywords and also keywords with parameters (which
     * follows a '=' sign
     */
    confirmation_needed = PARSE_OK;

    fields = g_strsplit(inputbuffer, "=", 2);
    g_strstrip(fields[0]);

    if (*fields[0] == '\0') { 	/* only whitespace found? */
	g_strfreev(fields);
	return (PARSE_OK);
    }

    if (g_strv_length(fields) == 2) {   /* strip leading whitespace */
	g_strchug(fields[1]);		/* from parameters */
    }

    int result = apply_config(fields[0], fields[1], logcfg_configs);
    if (result != PARSE_NO_MATCH) { // we got a match
	g_strfreev(fields);
	return result;
    }

    g_strlcpy(teststring, fields[0], sizeof(teststring));

    for (ii = 0; ii < MAX_COMMANDS; ii++) {
	if (g_strcmp0(teststring, commands[ii]) == 0) {
	    break;
	}
    }

    switch (ii) {

	case 0: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 1: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 2 ... 10: {	/* messages */
	    PARAMETER_NEEDED(teststring);
	    strcpy(message[ii - 2], fields[1]);
	    break;
	}
	case 11 ... 13: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(message[ii - 2], fields[1]);
	    break;
	}
	case 14: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(message[SP_TU_MSG], fields[1]);
	    break;
	}
	case 15: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(message[CQ_TU_MSG], fields[1]);
	    break;	/* end messages */
	}
	case 16: {
	    char *tmpcall;
	    PARAMETER_NEEDED(teststring);
	    if (strlen(fields[1]) > 20) {
		mvprintw(6, 0,
			 "WARNING: Defined call sign too long! exiting...\n");
		refreshp();
		exit(1);
	    }
	    if (strlen(fields[1]) == 0) {
		mvprintw(6, 0,
			 "WARNING: No callsign defined in logcfg.dat! exiting...\n");
		refreshp();
		exit(1);
	    }

	    /* strip NL and trailing whitespace, convert to upper case */
	    tmpcall = g_ascii_strup(g_strchomp(fields[1]), -1);
	    g_strlcpy(my.call, tmpcall, 20);
	    g_free(tmpcall);
	    /* as other code parts rely on a trailing NL on the call
	     * we add back such a NL for now */
	    strcat(my.call, "\n");
	    // check that call sign can be found in cty database !!
	    break;
	}
	case 17:
	case 122: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(whichcontest, g_strchomp(fields[1]));
	    if (strlen(whichcontest) > 40) {
		showmsg
		("WARNING: contest name is too long! exiting...");
		exit(1);
	    }
	    setcontest();
	    break;
	}
	case 18: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(logfile, g_strchomp(fields[1]));
	    break;
	}
	case 19: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(keyer_device, g_strchomp(fields[1]),
		      sizeof(keyer_device));
	    break;
	}
	case 20: {		// Use the bandswitch output on parport0
	    /* \todo add message if parameter too short */
	    use_bandoutput = 1;
	    if ((fields[1] != NULL) && (strlen(fields[1]) >= 10)) {
		for (jj = 0; jj <= 9; jj++) {	// 10x
		    hh = ((int)(fields[1][jj])) - 48;

		    if (hh >= 0 && hh <= 9)
			bandindexarray[jj] = hh;
		    else
			bandindexarray[jj] = 0;
		}
	    }
	    break;
	}
	case 21: {
	    recall_mult = 1;
	    break;
	}
	case 22: {
	    one_point = 1;
	    universal = 1;
	    break;
	}
	case 23: {
	    three_point = 1;
	    universal = 1;
	    break;
	}
	case 24: {
	    wysiwyg_multi = 1;
	    break;
	}
	case 25: {
	    wysiwyg_once = 1;
	    break;
	}
	case 26: {
	    trx_control = 1;
	    break;
	}
	case 27: {
	    rit = 1;
	    break;
	}
	case 28: {
	    shortqsonr = 1;
	    break;
	}
	case 29: {
	    shortqsonr = 0;
	    break;
	}
	case 30: {
	    iscontest = true;
	    break;
	}
	case 31: {
	    cluster = CLUSTER;
	    break;
	}
	case 32: {
	    cluster = MAP;

	    /* init bandmap filtering */
	    bm_config.allband = 1;
	    bm_config.allmode = 1;
	    bm_config.showdupes = 1;
	    bm_config.skipdupes = 0;
	    bm_config.livetime = 900;
	    bm_config.onlymults = 0;

	    /* Allow configuration of bandmap display if keyword
	     * is followed by a '='
	     * Parameter format is BANDMAP=<xxx>,<number>
	     * <xxx> - string parsed for the letters B, M, D and S
	     * <number> - spot livetime in seconds (>=30)
	     */
	    if (fields[1] != NULL) {
		char **bm_fields;
		bm_fields = g_strsplit(fields[1], ",", 2);
		if (bm_fields[0] != NULL) {
		    char *ptr = bm_fields[0];
		    while (*ptr != '\0') {
			switch (*ptr++) {
			    case 'B': bm_config.allband = 0;
				break;
			    case 'M': bm_config.allmode = 0;
				break;
			    case 'D': bm_config.showdupes = 0;
				break;
			    case 'S': bm_config.skipdupes = 1;
				break;
			    case 'O': bm_config.onlymults = 1;
				break;
			    default:
				break;
			}
		    }
		}

		if (bm_fields[1] != NULL) {
		    int livetime;
		    g_strstrip(bm_fields[1]);
		    livetime = atoi(bm_fields[1]);
		    if (livetime >= 30)
			/* aging called each second */
			bm_config.livetime = livetime;
		}


		g_strfreev(bm_fields);
	    }
	    break;
	}
	case 33: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 34: {
	    showscore_flag = 1;
	    break;
	}
	case 35: {
	    searchflg = 1;
	    break;
	}
	case 36: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 37: {
	    demode = 1;
	    break;
	}
	case 38: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strncat(buff, fields[1], 2);
	    SetCWSpeed(atoi(buff));
	    break;
	}
	case 39: {
	    int tone;
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);
	    tone = atoi(buff);
	    if ((tone > -1) && (tone < 1000)) {
		sprintf(tonestr, "%d", tone);
	    }
	    break;
	}
	case 40: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);
	    weight = atoi(buff);
	    if (weight < -50)
		weight = -50;
	    if (weight > 50)
		weight = 50;
	    break;
	}
	case 41: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);
	    txdelay = atoi(buff);
	    if (txdelay > 50)
		txdelay = 50;
	    if (txdelay < 0)
		txdelay = 0;
	    break;
	}
	case 42: {
	    PARAMETER_NEEDED(teststring);
	    wwv_set_r(atoi(fields[1]));
	    break;
	}
	case 43: {
	    PARAMETER_NEEDED(teststring);
	    wwv_set_sfi(atoi(fields[1]));
	    break;
	}
	case 45: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    editor_cmd = g_strdup(g_strchomp(fields[1]));
	}
	case 46: {
	    partials = 1;
	    break;
	}

	case 47: {
	    use_part = 1;
	    break;
	}
	/*case 48:{
	    fixedmult = 5;
	    break;
	}
	case 49:{
	    fixedmult = 2;
	    break;
	}
	case 50:{
	    fixedmult = 1;
	    break;
	} */
	case 51: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 52: {
	    exchange_serial = 1;
	    break;
	}
	case 53: {
	    country_mult = 1;
	    break;
	}
	case 54: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 55: {
	    portable_x2 = 1;
	    break;
	}
	case 56: {
	    mixedmode = true;
	    break;
	}
	case 57: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(pr_hostaddress, g_strchomp(fields[1]), 48);
	    break;
	}
	case 58: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strncat(buff, fields[1], 5);
	    portnum = atoi(buff);
	    packetinterface = TELNET_INTERFACE;
	    break;
	}
	case 59: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);
	    if (strlen(buff) > 2) {
		strncpy(tncportname, buff, 39);
	    } else
		tncport = atoi(buff) + 1;

	    packetinterface = TNC_INTERFACE;
	    break;
	}
	case 60: {
	    packetinterface = FIFO_INTERFACE;
	    break;
	}
	case 61: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);

	    if (strncmp(buff, "ORION", 5) == 0)
		rignumber = 2000;
	    else
		rignumber = atoi(buff);

	    myrig_model = (rig_model_t) rignumber;

	    break;
	}
	case 62: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);
	    serial_rate = atoi(buff);
	    break;
	}
	case 63: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strcat(buff, fields[1]);
	    tnc_serial_rate = atoi(buff);
	    break;
	}
	case 64: {
	    PARAMETER_NEEDED(teststring);
	    rigportname = strdup(fields[1]);
	    break;
	}
	case 65: {
	    cwkeyer = NET_KEYER;
	    break;
	}
	case 66: {
	    PARAMETER_NEEDED(teststring);
	    netkeyer_port = atoi(fields[1]);
	    break;
	}
	case 67: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(netkeyer_hostaddress, g_strchomp(fields[1]), 16);
	    break;
	}
	case 68: {
	    PARAMETER_NEEDED(teststring);
	    if (node < MAXNODES) {
		/* split host name and port number, separated by colon */
		char **an_fields;
		an_fields = g_strsplit(fields[1], ":", 2);
		/* copy host name */
		g_strlcpy(bc_hostaddress[node], g_strchomp(an_fields[0]),
			  sizeof(bc_hostaddress[0]));
		if (an_fields[1] != NULL) {
		    /* copy host port, if found */
		    g_strlcpy(bc_hostservice[node], g_strchomp(an_fields[1]),
			      sizeof(bc_hostservice[0]));
		}
		g_strfreev(an_fields);

		if (node++ < MAXNODES)
		    nodes++;
	    }
	    lan_active = 1;
	    break;
	}
	case 69: {
	    char c;
	    PARAMETER_NEEDED(teststring);
	    c = toupper(fields[1][0]);
	    if (c >= 'A' && c <= 'H')
		thisnode = c;
	    else
		WrongFormat(teststring);
	    break;
	}
	case 70: {
	    cqwwm2 = 1;
	    break;
	}
	case 71: {
	    landebug = 1;
	    break;
	}
	case 72 ... 81: {	/* messages */
	    PARAMETER_NEEDED(teststring);
	    strcpy(message[ii - 58], fields[1]);
	    break;
	}
	case 82: {
	    call_update = 1;
	    break;
	}
	case 83: {
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strncat(buff, fields[1], 3);
	    timeoffset = atoi(buff);
	    if (timeoffset > 23)
		timeoffset = 23;
	    if (timeoffset < -23)
		timeoffset = -23;
	    break;
	}
	case 84: {
	    time_master = 1;
	    break;
	}
	case 85: {
	    ctcomp = 1;
	    break;
	}
	case 86: {
	    two_point = 1;
	    universal = 1;
	    break;
	}
	case 87: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(multsfile, g_strchomp(fields[1]), 80);
	    multlist = 1;
	    universal = 1;
	    break;
	}
	case 88: {
	    serial_section_mult = 1;
	    break;
	}
	case 89: {
	    sectn_mult = 1;
	    break;
	}
	case 90: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(markerfile, g_strchomp(fields[1]));
	    xplanet = 1;
	    break;
	}
	case 91: {
	    dx_arrlsections = 1;
	    setcontest();
	    break;
	}
	case 92: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(markerfile, g_strchomp(fields[1]));
	    xplanet = 2;
	    break;
	}
	case 93: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(markerfile, g_strchomp(fields[1]));
	    xplanet = 3;
	    break;
	}
	case 94: {
	    nob4 = 1;
	    break;
	}

	case 95: {
	    /* COUNTRYLIST   (in file or listed in logcfg.dat)     LZ3NY
	     */

	    int counter = 0;
	    static char country_list_raw[50] = ""; 	/* use only first
				   COUNTRY_LIST
				   definition */
	    char temp_buffer[255] = "";
	    char buffer[255] = "";
	    FILE *fp;

	    PARAMETER_NEEDED(teststring);
	    if (strlen(country_list_raw) == 0) {/* only if first definition */

		/* First of all we are checking if the parameter <xxx> in
		COUNTRY_LIST=<xxx> is a file name.  If it is we start
		parsing the file. If we  find a line starting with our
		case insensitive contest name, we copy the countries from
		that line into country_list_raw.
		If the input was not a file name we directly copy it into
		country_list_raw (must not have a preceeding contest name). */

		g_strlcpy(temp_buffer, fields[1], sizeof(temp_buffer));
		g_strchomp(temp_buffer);	/* drop trailing whitespace */

		if ((fp = fopen(temp_buffer, "r")) != NULL) {

		    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

			g_strchomp(buffer);   /* no trailing whitespace*/

			/* accept only a line starting with the contest name
			 * (CONTEST=) followed by ':' */
			if (strncasecmp(buffer, whichcontest,
					strlen(whichcontest) - 1) == 0) {

			    strncpy(country_list_raw,
				    buffer + strlen(whichcontest) + 1,
				    strlen(buffer) - 1);
			}
		    }

		    fclose(fp);
		} else {	/* not a file */

		    if (strlen(temp_buffer) > 0)
			strcpy(country_list_raw, temp_buffer);
		}
	    }

	    /* parse the country_list_raw string into an array
	     * (countrylist) for future use. */
	    tk_ptr = strtok(country_list_raw, ":,.- \t");
	    counter = 0;

	    if (tk_ptr != NULL) {
		while (tk_ptr) {
		    strcpy(countrylist[counter], tk_ptr);
		    tk_ptr = strtok(NULL, ":,.-_\t ");
		    counter++;
		}
	    }

	    /* on which multiplier side of the rules we are */
	    getpx(my.call);
	    mult_side = exist_in_country_list();
	    setcontest();
	    break;
	}

	case 96: {		// COUNTRY_LIST_POINTS
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(c_temp, fields[1], sizeof(c_temp));
	    if (countrylist_points == -1)
		countrylist_points = atoi(c_temp);

	    break;
	}
	case 97: {		// COUNTRY_LIST_ONLY
	    countrylist_only = true;
	    if (mult_side == 1)
		countrylist_only = false;

	    break;
	}
	case 98: {		//HOW Many points scores my country  lz3ny
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(c_temp, fields[1], sizeof(c_temp));
	    if (my_country_points == -1)
		my_country_points = atoi(c_temp);

	    break;
	}
	case 99: {		//MY_CONTINENT_POINTS       lz3ny
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(c_temp, fields[1], sizeof(c_temp));
	    if (my_cont_points == -1)
		my_cont_points = atoi(c_temp);

	    break;
	}
	case 100: {		//DX_CONTINENT_POINTS       lz3ny
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(c_temp, fields[1], sizeof(c_temp));
	    if (dx_cont_points == -1)
		dx_cont_points = atoi(c_temp);

	    break;
	}
	/* end LZ3NY mod */
	case 101: {		// show time in searchlog window
	    show_time = 1;
	    break;
	}
	case 102: {		// use rxvt colours
	    use_rxvt = 1;
	    break;
	}
	case 103 ... 116: {	// get phone messages
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(ph_message[ii - 103], g_strchomp(fields[1]), 71);
	    if (verbose) {
		gchar *tmp;
		tmp = g_strdup_printf("  Phone message #%d is %s", ii - 103,
				      ph_message[ii - 103]);	// (W9WI)
		showmsg(tmp);
		g_free(tmp);
	    }
	    break;
	}
	case 117: {		// WAZ Zone is a Multiplier
	    wazmult = 1;
	    break;
	}
	case 118: {		// ITU Zone is a Multiplier
	    itumult = 1;
	    break;
	}
	case 119: {		// CQ Delay (0.5 sec)
	    PARAMETER_NEEDED(teststring);
	    buff[0] = '\0';
	    strncpy(buff, fields[1], 3);
	    cqdelay = atoi(buff);
	    if ((cqdelay < 3) || (cqdelay > 60))
		cqdelay = 20;

	    break;
	}
	case 120: {		// wpx style prefixes mult
	    pfxmult = 1;	// enable set points
	    break;
	}
	case 121: {		// exchange continent abbrev
	    exc_cont = 1;
	    break;
	}
	case 123: {		// don't use auto_cq
	    noautocq = 1;
	    break;
	}
	case 124: {		// start in SSB mode
	    trxmode = SSBMODE;
	    break;
	}
	case 125: {		// arrow keys don't switch bands...
	    no_arrows = 1;
	    break;
	}
	case 126: {		// Hamlib rig conf parameters
	    PARAMETER_NEEDED(teststring);
	    if (strlen(fields[1]) >= 80) {
		showmsg
		("WARNING: rigconf parameters too long! exiting...");
		sleep(5);
		exit(1);
	    }
	    g_strlcpy(rigconf, g_strchomp(fields[1]), 80);	// RIGCONF=
	    break;
	}
	case 127: {		// define color GREEN (header)
	    PARAMETER_NEEDED(teststring);
	    if (strlen(fields[1]) >= 2 && isdigit(fields[1][0]) &&
		    isdigit(fields[1][1])) {
		tlfcolors[1][0] = fields[1][0] - 48;
		tlfcolors[1][1] = fields[1][1] - 48;
	    } else {
		WrongFormat(teststring);
	    }
	    break;
	}
	case 128 ... 132: {		// define color CYAN (windows), WHITE (log win)
	    // MAGENTA (Marker / dupes), BLUE (input field)
	    // and YELLOW (Window frames)
	    PARAMETER_NEEDED(teststring);
	    if (strlen(fields[1]) >= 2 && isdigit(fields[1][0]) &&
		    isdigit(fields[1][1])) {
		tlfcolors[ii - 128 + 3][0] = fields[1][0] - 48;
		tlfcolors[ii - 128 + 3][1] = fields[1][1] - 48;
	    } else {
		WrongFormat(teststring);
	    }
	    break;
	}
	case 133: {		// define name of synclogfile
	    PARAMETER_NEEDED(teststring);
	    strcpy(synclogfile, g_strchomp(fields[1]));
	    break;
	}
	case 134: {		//SSBPOINTS=
	    PARAMETER_NEEDED(teststring);
	    strcpy(buff, fields[1]);
	    ssbpoints = atoi(buff);
	    break;
	}
	case 135: {		//CWPOINTS=
	    PARAMETER_NEEDED(teststring);
	    strcpy(buff, fields[1]);
	    cwpoints = atoi(buff);
	    break;
	}
	case 136: {		// SOUNDCARD, use soundcard for cw sidetone
	    sc_sidetone = 1;
	    break;
	}
	case 137: {		// sound card volume (default = 70)
	    int volume;

	    PARAMETER_NEEDED(teststring);
	    volume = atoi(fields[1]);
	    if (volume > -1 && volume < 101)
		sprintf(sc_volume, "%d", volume);
	    else
		strcpy(sc_volume, "70");
	    break;
	}
	case 139: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(sc_device, g_strchomp(fields[1]), sizeof(sc_device));
	    break;
	}
	case 140: {
	    PARAMETER_NEEDED(teststring);
	    cwkeyer = MFJ1278_KEYER;
	    digikeyer = MFJ1278_KEYER;
	    g_strlcpy(controllerport, g_strchomp(fields[1]),
		      sizeof(controllerport));
	    break;
	}
	case 141: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(clusterlogin, fields[1]);
	    break;
	}
	case 142: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 143: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(exchange_list, g_strchomp(fields[1]),
		      sizeof(exchange_list));
	    break;
	}
	case 144: {
	    PARAMETER_NEEDED(teststring);
	    cw_bandwidth = atoi(fields[1]);
	    break;
	}
	case 145: {
	    lowband_point_mult = 1;
	    break;
	}
	case 146: {
	    clusterlog = 1;
	    break;
	}
	case 147: {
	    serial_grid4_mult = 1;
	    break;
	}
	case 148: {
	    change_rst = true;
	    if (g_strv_length(fields) == 2) {
		/* comma separated list of RS(T) values 33..39, 43..39, 53..59
		 * allowed.
		 */
		if (!g_regex_match_simple(
			    "^([3-5][3-9]\\d?\\s*,\\s*)*[3-5][3-9]\\d?$",
			    g_strstrip(fields[1]), G_REGEX_CASELESS,
			    (GRegexMatchFlags)0)) {
		    WrongFormat(teststring);
		}
		rst_init(fields[1]);
	    } else {
		rst_init(NULL);
	    }
	    break;
	}
	case 149: {
	    PARAMETER_NEEDED(teststring);
	    digikeyer = GMFSK;
	    g_strlcpy(controllerport, g_strchomp(fields[1]),
		      sizeof(controllerport));
	    break;
	}
	case 150: {		// start in digital mode
	    trxmode = DIGIMODE;
	    strcpy(modem_mode, "RTTY");
	    break;
	}
	case 151: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(rttyoutput, g_strchomp(fields[1]), 111);
	    break;
	}
	case 152: {
	    logfrequency = 1;
	    break;
	}
	case 153: {
	    ignoredupe = true;
	    break;
	}
	case 154: {		/* read name of cabrillo format to use */

	    if (cabrillo != NULL) {
		free(cabrillo);	/* free old string if already set */
		cabrillo = NULL;
	    }
	    cabrillo = strdup(g_strchomp(fields[1]));
	    break;
	}
	case 155:
	case 156:
	case 157: {
	    KeywordNotSupported(teststring);
	    break;
	}
	case 158: {
	    no_rst = 1;
	    break;
	}
	case 159: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(my.qra, fields[1]);

	    if (check_qra(my.qra) == 0) {
		showmsg
		("WARNING: Invalid MYQRA parameters! exiting...");
		sleep(5);
		exit(1);
	    }
	    break;
	}
	case 160: {
	    PARAMETER_NEEDED(teststring);
	    if (fixedmult == 0.0 && atof(fields[1]) > 0.0) {
		fixedmult = atof(fields[1]);
	    }
	    break;
	}
	case 161: {
	    serial_or_section = 1;
	    break;
	}

	case 162: {
	    PARAMETER_NEEDED(teststring);
	    g_strchomp(fields[1]);
	    if (strncmp(fields[1], "RECV", 4) == 0) {
		qtcdirection = RECV;
	    }
	    if (strncmp(fields[1], "SEND", 4) == 0) {
		qtcdirection = SEND;
	    } else if (strcmp(fields[1], "BOTH") == 0) {
		qtcdirection = RECV | SEND;
	    }
	    if (qtcdirection == 0) {
		KeywordNotSupported(teststring);
	    } else {
		int q;
		for (q = 0; q < QTC_RY_LINE_NR; q++) {
		    qtc_ry_lines[q].content[0] = '\0';
		    qtc_ry_lines[q].attr = 0;
		}
	    }
	    break;
	}

	case 163: {
	    /* based on LZ3NY code, by HA2OS
	       CONTINENT_LIST   (in file or listed in logcfg.dat),
	       First of all we are checking if inserted data in
	       CONTINENT_LIST= is a file name.  If it is we start
	       parsing the file. If we got our case insensitive contest name,
	       we copy the multipliers from it into multipliers_list.
	       If the input was not a file name we directly copy it into
	       cont_multiplier_list (must not have a preceeding contest name).
	       The last step is to parse the multipliers_list into an array
	       (continent_multiplier_list) for future use.
	     */

	    int counter = 0;
	    static char cont_multiplier_list[50] = ""; 	/* use only first
				   CONTINENT_LIST
				   definition */
	    char temp_buffer[255] = "";
	    char buffer[255] = "";
	    FILE *fp;

	    PARAMETER_NEEDED(teststring);
	    if (strlen(cont_multiplier_list) == 0) {	/* if first definition */
		g_strlcpy(temp_buffer, fields[1], sizeof(temp_buffer));
		g_strchomp(temp_buffer);	/* drop trailing whitespace */

		if ((fp = fopen(temp_buffer, "r")) != NULL) {

		    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

			g_strchomp(buffer);   /* no trailing whitespace*/

			/* accept only a line starting with the contest name
			 * (CONTEST=) followed by ':' */
			if (strncasecmp(buffer, whichcontest,
					strlen(whichcontest) - 1) == 0) {

			    strncpy(cont_multiplier_list,
				    buffer + strlen(whichcontest) + 1,
				    strlen(buffer) - 1);
			}
		    }

		    fclose(fp);
		} else {	/* not a file */

		    if (strlen(temp_buffer) > 0)
			strcpy(cont_multiplier_list, temp_buffer);
		}
	    }

	    /* creating the array */
	    tk_ptr = strtok(cont_multiplier_list, ":,.- \t");
	    counter = 0;

	    if (tk_ptr != NULL) {
		while (tk_ptr) {
		    strncpy(continent_multiplier_list[counter], tk_ptr, 2);
		    tk_ptr = strtok(NULL, ":,.-_\t ");
		    counter++;
		}
	    }

	    setcontest();
	    break;
	}


	case 164: {		// CONTINENT_LIST_POINTS
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(c_temp, fields[1], sizeof(c_temp));
	    if (continentlist_points == -1) {
		continentlist_points = atoi(c_temp);
	    }

	    break;
	}
	case 165: {		// CONTINENT_LIST_ONLY
	    continentlist_only = true;
	    break;
	}

	case 166: {		// BANDWEIGHT_POINTS
	    PARAMETER_NEEDED(teststring);
	    static char bwp_params_list[50] = "";
	    int bandindex = -1;

	    if (strlen(bwp_params_list) == 0) {
		g_strlcpy(bwp_params_list, fields[1], sizeof(bwp_params_list));
		g_strchomp(bwp_params_list);
	    }

	    tk_ptr = strtok(bwp_params_list, ";:,");
	    if (tk_ptr != NULL) {
		while (tk_ptr) {

		    bandindex = getidxbybandstr(g_strchomp(tk_ptr));
		    tk_ptr = strtok(NULL, ";:,");
		    if (tk_ptr != NULL && bandindex >= 0) {
			bandweight_points[bandindex] = atoi(tk_ptr);
		    }
		    tk_ptr = strtok(NULL, ";:,");
		}
	    }
	    break;
	}

	case 167: {		// BANDWEIGHT_MULTIS
	    PARAMETER_NEEDED(teststring);
	    static char bwm_params_list[50] = "";
	    int bandindex = -1;

	    if (strlen(bwm_params_list) == 0) {
		g_strlcpy(bwm_params_list, fields[1], sizeof(bwm_params_list));
		g_strchomp(bwm_params_list);
	    }

	    tk_ptr = strtok(bwm_params_list, ";:,");
	    if (tk_ptr != NULL) {
		while (tk_ptr) {

		    bandindex = getidxbybandstr(g_strchomp(tk_ptr));
		    tk_ptr = strtok(NULL, ";:,");
		    if (tk_ptr != NULL && bandindex >= 0) {
			bandweight_multis[bandindex] = atoi(tk_ptr);
		    }
		    tk_ptr = strtok(NULL, ";:,");
		}
	    }
	    break;
	}

	case 168: {
	    /* based on LZ3NY code, by HA2OS
	       PFX_NUM_MULTIS   (in file or listed in logcfg.dat),
	       We directly copy it into pfxnummulti_str, then parse the prefixlist
	       and fill the pfxnummulti array.
	     */

	    int counter = 0;
	    int pfxnum;
	    static char pfxnummulti_str[50] = "";
	    char parsepfx[15] = "";

	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(pfxnummulti_str, fields[1], sizeof(pfxnummulti_str));
	    g_strchomp(pfxnummulti_str);

	    /* creating the array */
	    tk_ptr = strtok(pfxnummulti_str, ",");
	    counter = 0;

	    if (tk_ptr != NULL) {
		while (tk_ptr) {
		    parsepfx[0] = '\0';
		    if (isdigit(tk_ptr[strlen(tk_ptr) - 1])) {
			sprintf(parsepfx, "%sAA", tk_ptr);
		    } else {
			sprintf(parsepfx, "%s0AA", tk_ptr);
		    }
		    pfxnummulti[counter].countrynr = getctydata(parsepfx);
		    for (pfxnum = 0; pfxnum < 10; pfxnum++) {
			pfxnummulti[counter].qsos[pfxnum] = 0;
		    }
		    tk_ptr = strtok(NULL, ",");
		    counter++;
		}
	    }
	    pfxnummultinr = counter;
	    setcontest();
	    break;
	}
	case 169: {             /* wpx style prefixes mult */
	    pfxmultab = 1;	/* enable pfx on all band */
	    break;
	}

	case 170 ... 181: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(qtc_recv_msgs[ii - 170], fields[1]);
	    break;
	}
	case 182 ... 193: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(qtc_send_msgs[ii - 182], fields[1]);
	    break;
	}
	case 194 ... 207: {	// get QTC recv phone messages
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(qtc_phrecv_message[ii - 194], g_strchomp(fields[1]), 71);
	    if (verbose) {
		gchar *tmp;
		tmp = g_strdup_printf("  QTC RECV phone message #%d is %s",
				      ii - 194, qtc_phrecv_message[ii - 194]);
		showmsg(tmp);
		g_free(tmp);
	    }
	    break;
	}
	case 208 ... 221: {	// get QTC send phone messages
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(qtc_phsend_message[ii - 208], g_strchomp(fields[1]), 71);
	    if (verbose) {
		gchar *tmp;
		tmp = g_strdup_printf("  QTC SEND phone message #%d is %s",
				      ii - 208, qtc_phrecv_message[ii - 208]);
		showmsg(tmp);
		g_free(tmp);
	    }
	    break;
	}
	case 222: {
	    qtcrec_record = 1;
	    break;
	}
	case 223: {
	    PARAMETER_NEEDED(teststring);
	    int p, q = 0, i = 0, s = 0;
	    for (p = 0; p < strlen(fields[1]); p++) {
		if (p > 0 && fields[1][p] == ' ') {
		    s = 1;
		    qtcrec_record_command_shutdown[p] = '\0';
		}
		if (s == 0) {
		    qtcrec_record_command_shutdown[p] = fields[1][p];
		}
		if (fields[1][p] == '$') {
		    qtcrec_record_command[i][q] = '\0';
		    i = 1;
		    p++;
		    q = 0;
		}
		if (fields[1][p] != '\n') {
		    qtcrec_record_command[i][q] = fields[1][p];
		}
		q++;
		qtcrec_record_command[i][q] = ' ';
	    }

	    if (qtcrec_record_command[i][q - 1] != '&') {
		qtcrec_record_command[i][q++] = ' ';
		qtcrec_record_command[i][q++] = '&';
	    }
	    qtcrec_record_command[i][q] = '\0';
	    break;
	}
	case 224: {
	    PARAMETER_NEEDED(teststring);
	    if (strcmp(g_strchomp(fields[1]), "CONTINENTLIST") == 0) {
		if (strlen(continent_multiplier_list[0]) == 0) {
		    showmsg
		    ("WARNING: you need to set the CONTINENTLIST parameter...");
		    sleep(5);
		    exit(1);
		}
		exclude_multilist_type = EXCLUDE_CONTINENT;
	    } else if (strcmp(g_strchomp(fields[1]), "COUNTRYLIST") == 0) {
		if (strlen(countrylist[0]) == 0) {
		    showmsg
		    ("WARNING: you need to set the COUNTRYLIST parameter...");
		    sleep(5);
		    exit(1);
		}
		exclude_multilist_type = EXCLUDE_COUNTRY;
	    } else {
		showmsg
		("WARNING: choose one of these for EXCLUDE_MULTILIST: CONTINENTLIST, COUNTRYLIST");
		sleep(5);
		exit(1);
	    }
	    break;
	}
	case 225: {
	    PARAMETER_NEEDED(teststring);
	    strcpy(message[SP_CALL_MSG], fields[1]);
	    break;	/* end messages */
	}
	case 226: {
	    PARAMETER_NEEDED(teststring);
	    g_strlcpy(qtc_cap_calls, g_strchomp(fields[1]),
		      sizeof(exchange_list));
	    break;
	}
	case 227: {
	    qtc_auto_filltime = 1;
	    break;
	}
	case 228: {
	    bmautograb = 1;
	    break;
	}
	case 229: {
	    bmautoadd = 1;
	    break;
	}
	case 230: {
	    qtc_recv_lazy = 1;
	    break;
	}
	case 231: {
	    sprint_mode = 1;
	    break;
	}
	case 232: {
#ifndef HAVE_LIBXMLRPC
	    showmsg("WARNING: XMLRPC not compiled - skipping setup.");
	    sleep(2);
	    digikeyer = NO_KEYER;
#else
	    if (fields[1] != NULL) {
		g_strlcpy(fldigi_url, g_strchomp(fields[1]),
			  sizeof(fldigi_url));
	    }
	    digikeyer = FLDIGI;
	    if (!fldigi_isenabled())
		fldigi_toggle();
#endif
	    break;
	}
	case 233: {
	    rigptt |= (1 << 0);		/* bit 0 set--CAT PTT wanted (RIGPTT) */
	    break;
	}
	case 234: {
	    if (fields[1] != NULL) {
		int minisec;
		minisec = atoi(g_strchomp(fields[1]));
		if ((3600 % minisec) != 0) {
		    showmsg
		    ("WARNING: invalid MINITEST value, must be an integral divider for 3600s!");
		    sleep(5);
		    exit(1);
		} else {
		    minitest = minisec;
		}
	    } else {
		minitest = MINITEST_DEFAULT_PERIOD;
	    }
	    break;
	}
	case 235: {
	    PARAMETER_NEEDED(teststring);
	    if (strcmp(g_strchomp(fields[1]), "ALL") == 0) {
		unique_call_multi = UNIQUECALL_ALL;
	    } else if (strcmp(g_strchomp(fields[1]), "BAND") == 0) {
		unique_call_multi = UNIQUECALL_BAND;
	    } else {
		showmsg
		("WARNING: choose one of these for UNIQUE_CALL_MULTI: ALL, BAND");
		sleep(5);
		exit(1);
	    }
	    break;
	}
	case 236: { // KEYER_BACKSPACE
	    keyer_backspace = 1;
	    break;
	}
	case 237: {
	    PARAMETER_NEEDED(teststring);
	    g_strchomp(fields[1]);
	    if (strcmp(fields[1], "USB") == 0)
		digi_mode = RIG_MODE_USB;
	    else if (strcmp(fields[1], "LSB") == 0)
		digi_mode = RIG_MODE_LSB;
	    else if (strcmp(fields[1], "RTTY") == 0)
		digi_mode = RIG_MODE_RTTY;
	    else if (strcmp(fields[1], "RTTYR") == 0)
		digi_mode = RIG_MODE_RTTYR;
	    else {
		showmsg
		("WARNING: invalid DIGI_RIG_MODE value, must be \"USB\", \"LSB\", \"RTTY\", or \"RTTYR\"");
		sleep(5);
		exit(1);
	    }
	    break;
	}
	case 238 ... 249: {
	    PARAMETER_NEEDED(teststring);
	    if (digi_message[ii - 238]) {
		KeywordRepeated(commands[ii]);
		free(digi_message[ii - 238]);
	    }
	    digi_message[ii - 238] = strdup(fields[1]);
	    if (digi_message[ii - 238]) {
		/* Replace trailing newline with a space */
		char *nl = strrchr(digi_message[ii - 238], '\n');
		if (nl)
		    *nl = ' ';
	    }
	    break;
	}
	case 250:
	    PARAMETER_NEEDED(teststring);
	    if (digi_message[CQ_TU_MSG]) {
		KeywordRepeated(commands[ii]);
		free(digi_message[CQ_TU_MSG]);
	    }
	    digi_message[CQ_TU_MSG] = strdup(fields[1]);
	    if (digi_message[CQ_TU_MSG]) {
		/* Replace trailing newline with a space */
		char *nl = strrchr(digi_message[CQ_TU_MSG], '\n');
		if (nl)
		    *nl = ' ';
	    }
	    break;
	case 251:
	    PARAMETER_NEEDED(teststring);
	    if (digi_message[SP_TU_MSG]) {
		KeywordRepeated(commands[ii]);
		free(digi_message[SP_TU_MSG]);
	    }
	    digi_message[SP_TU_MSG] = strdup(fields[1]);
	    if (digi_message[SP_TU_MSG]) {
		/* Replace trailing newline with a space */
		char *nl = strrchr(digi_message[SP_TU_MSG], '\n');
		if (nl)
		    *nl = ' ';
	    }
	    break;
	case 252:
	    PARAMETER_NEEDED(teststring);
	    if (digi_message[SP_CALL_MSG]) {
		KeywordRepeated(commands[ii]);
		free(digi_message[SP_CALL_MSG]);
	    }
	    digi_message[SP_CALL_MSG] = strdup(fields[1]);
	    if (digi_message[SP_CALL_MSG]) {
		/* Replace trailing newline with a space */
		char *nl = strrchr(digi_message[SP_CALL_MSG], '\n');
		if (nl)
		    *nl = ' ';
	    }
	    break;
	case 253 ... 262: {
	    PARAMETER_NEEDED(teststring);
	    if (digi_message[ii - 239]) {
		KeywordRepeated(commands[ii]);
		free(digi_message[ii - 239]);
	    }
	    digi_message[ii - 239] = strdup(fields[1]);
	    if (digi_message[ii - 239]) {
		/* Replace trailing newline with a space */
		char *nl = strrchr(digi_message[ii - 239], '\n');
		if (nl)
		    *nl = ' ';
	    }
	    break;
	}
	case 263: {
	    PARAMETER_NEEDED(teststring);
	    g_strchomp(fields[1]);
	    if (callmaster_filename != NULL) {
		g_free(callmaster_filename);
	    }
	    callmaster_filename = g_strdup(fields[1]);

	    break;
	}
	case 264: {
	    PARAMETER_NEEDED(teststring);
	    lan_port = atoi(fields[1]);
	    break;
	}
	case 265: {
	    sectn_mult_once = 1;
	    break;
	}
	default: {
	    KeywordNotSupported(teststring);
	    break;
	}
    }

    g_strfreev(fields);

    return (confirmation_needed);

}


/** Complain about problems in configuration
 *
 * Complains in standout mode about some problem. Beep and wait for
 * 2 seconds
 *
 * \param msg  The reason for the problem to be shown
 */
void Complain(char *msg) {
    attron(A_STANDOUT);
    showmsg(msg);
    attroff(A_STANDOUT);
    confirmation_needed = PARSE_CONFIRM;
    beep();
}

/** Complain about duplicate keyword */
void KeywordRepeated(char *keyword) {
    char msgbuffer[128];
    sprintf(msgbuffer,
	    "Keyword '%s' repeated more than once.\n",
	    keyword);
    Complain(msgbuffer);
}

/** Complain about not supported keyword */
void KeywordNotSupported(char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Keyword '%s' not supported. See man page.\n",
	    keyword);
    Complain(msgbuffer);
}

/** Complain about missing parameter */
void ParameterNeeded(const char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Keyword '%s' must be followed by an parameter ('=....'). See man page.\n",
	    keyword);
    Complain(msgbuffer);
}

/** Complain about wrong parameter format */
void WrongFormat(const char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Wrong parameter format for keyword '%s'. See man page.\n",
	    keyword);
    Complain(msgbuffer);
}

void WrongFormat_details(const char *keyword, const char *details) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Wrong parameter for keyword '%s': %s.\n",
	    keyword, details);
    Complain(msgbuffer);
}
