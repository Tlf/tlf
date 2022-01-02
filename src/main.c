/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0r@eudxf.org>
 *                    2010-2017 Thomas Beierlein <tb@forth-ev.de>
 *                    2013-2016 Ervin Hegedus - HA2OS <airween@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include <argp.h>
#include <ctype.h>
#include <hamlib/rig.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "addmult.h"
#include "background_process.h"
#include "bandmap.h"
#include "change_rst.h"
#include "clear_display.h"
#include "checklogfile.h"
#include "checkqtclogfile.h"
#include "cw_utils.h"
#include "fldigixmlrpc.h"
#include "getmessages.h"
#include "getwwv.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
#include "hamlib_keyer.h"
#include "initial_exchange.h"
#include "lancode.h"
#include "logit.h"
#include "netkeyer.h"
#include "parse_logcfg.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "readctydata.h"
#include "readcalls.h"
#include "readqtccalls.h"
#include "rtty.h"
#include "rules.h"
#include "scroll_log.h"
#include "searchlog.h"		// Includes glib.h
#include "sendqrg.h"
#include "setcontest.h"
#include "set_tone.h"
#include "splitscreen.h"
#include "startmsg.h"
#include "tlf_panel.h"
#include "readcabrillo.h"
#include "ui_utils.h"

#include <config.h>


SCREEN *mainscreen;

char pr_hostaddress[48] = "131.155.192.179";
char *config_file = NULL;
int portnum = 0;

bool use_rxvt = false;
bool use_xterm = false;

int tlfcolors[8][2] = { {COLOR_BLACK, COLOR_WHITE},
    {COLOR_GREEN, COLOR_YELLOW},
    {COLOR_WHITE, COLOR_RED},
    {COLOR_CYAN, COLOR_WHITE},
    {COLOR_WHITE, COLOR_BLACK},
    {COLOR_WHITE, COLOR_MAGENTA},
    {COLOR_BLUE, COLOR_YELLOW},
    {COLOR_WHITE, COLOR_BLACK}
};
bool debugflag = false;
char *editor_cmd = NULL;
int tune_val = 0;
int use_bandoutput = 0;
bool no_arrows = false;
int bandindexarray[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
bool cqwwm2 = false;

char whichcontest[40] = "qso";
bool iscontest = false;		/* false =  General,  true  = contest */
contest_config_t *contest = &config_qso;	/* contest configuration */

/* predefined contests */
bool sprint_mode = false;
int minitest = 0;	/**< if set, length of minitest period in seconds */
int unique_call_multi = 0;          /* do we count calls as multiplier */


int addcallarea;
bool pfxmult = false;
bool pfxmultab = false;
bool exc_cont = false;
int ssbpoints;
int cwpoints;
bool lowband_point_mult = false;
bool sc_sidetone;
char sc_volume[4] = "";
/* LZ3NY mods */
int my_country_points = -1;
int my_cont_points = -1;
int dx_cont_points = -1;
char countrylist[255][6];
bool countrylist_only = false;
int countrylist_points = -1;
char continent_multiplier_list[7][3]; // SA, NA, EU, AF, AS and OC
int continentlist_points = -1;
bool continentlist_only = false;
int exclude_multilist_type = EXCLUDE_NONE;
bool mult_side = false;
/* end LZ3NY mods */

bool portable_x2 = false;
bool wysiwyg_once = false;
bool wysiwyg_multi = false;
bool country_mult = false;
float fixedmult = 0.0;
bool sectn_mult = false;
bool sectn_mult_once = false;
bool dx_arrlsections = false;
bool serial_section_mult = false;
bool serial_or_section = false;	/* exchange is serial OR section, like HA-DX */
bool serial_grid4_mult = false;
bool qso_once = false;
int noleadingzeros;
bool ctcomp = false;
bool nob4 = false;		// allow auto b4
bool ignoredupe = false;
int dupe = 0;
bool noautocq = false;
bool verbose = false;
bool no_rst = false;		/* do not use RS/RST */

int pacc_qsos[10][10];
int ve_cty;
int w_cty;
int zl_cty;
int ja_cty;
int py_cty;
int ce_cty;
int lu_cty;
int vk_cty;
int zs_cty;
int ua9_cty;

pfxnummulti_t pfxnummulti[MAXPFXNUMMULT];
int pfxnummultinr = 0;

char multsfile[80] = "";	/* name of file with a list of allowed
				   multipliers */
char exchange_list[40] = "";
int timeoffset = 0;
int trxmode = CWMODE;
rmode_t  rigmode = RIG_MODE_NONE;

bool mixedmode = false;
char sent_rst[4] = "599";
char recvd_rst[4] = "599";
char last_rst[4] = "599";       /* Report for last QSO */

/* TODO Maybe we can use the following */
int mults_per_band = 1;		/* mults count per band */

int shortqsonr = LONGCW;	/* 1  =  short  cw char in exchange */
int cluster = NOCLUSTER;	/* 0 = OFF, 1 = FOLLOW, 2  = spots  3 = all */
bool clusterlog = false;	/* clusterlog on/off */
bool searchflg = false;		/* display search  window */
bool show_time = false;
cqmode_t cqmode = CQ;
bool demode = false;		/* send DE  before s&p call  */

int announcefilter = FILTER_ANN; /*  filter cluster  announcements */
bool showscore_flag = false;	/* show  score window */
char exchange[40];
int defer_store = 0;
mystation_t my;			/* all info about me */

char logfile[120] = "general.log";
char *cabrillo = NULL;		/**< Name of the Cabrillo format definition */
char synclogfile[120];
char markerfile[120] = "";
int xplanet = MARKER_NONE;
int rigptt = 0;
int tune_seconds;               /* tune up time in seconds for Alt-T */

char message[25][80] = /**< Array of CW messages
 			*
 			* message[0]..[11] activated by F1..F12 key
 			* message[12] - TU message S&P mode
 			* message[13] - TU message CQ mode
 			* message[14]..[23] activated by Alt-0..9
			* message[24] - S&P call message
 			*
 			* special use:
			*
 			* message[0]  (F1)  - 'cq message' in CQ mode,
			*                     'de <call>' in S&P
			* message[2]  (F3)  - send rapport
			* message[4]  (F5)  - hiscall (used if '?' entered
			* 		      in call field
			* message[6]  (F7)  - 'worked before' message
 			* message[11] (F12) - used for auto-cq
			*
			* additional use if in CTCOMP mode
			* message[1]  (F2)  - insert pressed
 			*/
{
    "TEST %", "@ DE %", "@ [", "TU 73", "@", "%",
    "@ SRI QSO B4 GL", "AGN",
    "?", "QRZ?", "PSE K", "TEST % %", "@ [", "TU %",
    "", "", "", "", "", "", "", "", "", "", ""
};

char *digi_message[sizeof(message) / sizeof(message[0])];

char ph_message[14][80] = /**< Array of file names for voice keyer messages
			   * See description of message[]
			   */
    { "", "", "", "", "", "", "", "", "", "", "", "", "", "" };

char qtc_recv_msgs[12][80] = {
    "QTC?", "QRV", "R", "", "TIME?", "CALL?",
    "NR?", "AGN", "", "QSL ALL", "", ""};	// QTC receive windows Fx messages

char qtc_send_msgs[12][80] = {
    "QRV?", "QTC sr/nr", "", "", "TIME", "CALL",
    "NR", "", "", "", "", ""};		    	// QTC send window Fx messages

char qtc_phrecv_message[14][80] = {
    "", "", "", "", "", "",
    "", "", "", "", "", "" };			// voice keyer file names when receives QTCs

char qtc_phsend_message[14][80] = {
    "", "", "", "", "", "",
    "", "", "", "", "", "" };			// voice keyer file names when send QTCs

bool qtcrec_record = false;
char qtcrec_record_command[2][50] = {"rec -q 8000", "-q &"};
char qtcrec_record_command_shutdown[50] = "pkill -SIGINT -n rec";
char qtc_cap_calls[40] = "";
bool qtc_auto_filltime = false;
bool qtc_recv_lazy = false;

struct qso_t *current_qso;

char hiscall[20];			/**< call of other station */
char hiscall_sent[20] = "";		/**< part which was sent during early
					  start */
int cwstart = 0;			/**< number characters after which
					   sending call started automatically,
					   0 - off, -1 - manual start */
int sending_call = 0;
int early_started = 0;			/**< 1 if sending call started early,
					   strlen(hiscall)>cwstart or 'space' */
char lastcall[20];
char lastqsonr[5];
char qsonrstr[5] = "0001";
char band[NBANDS][4] =
{ "160", " 80", " 60", " 40", " 30", " 20", " 17", " 15", " 12", " 10", "???" };
char comment[80];
char normalized_comment[80];
char proposed_exchange[80];
char cqzone[3] = "";
char ituzone[3] = "";
char continent[3] = "";
char wpx_prefix[11] = "";
int totalmults = 0;
int totalcountries = 0;
int totalzones = 0;
int secs = 0;
int countrynr;
int total = 0; 		/**< total number of qso points */
int qso_points;		/**< number of points for last qso */
int qsos_per_band[NBANDS];
bool partials = false;	/**< show partial calls */
bool use_part = false;	/**< use automatically found partial call */
int block_part = 0; 	/**< if 1 block the call autocompletion
			  for these QSO */
char para_word[80] = "LODNCFS:3C\n";	/* longcw, cluster, search, DE,
					   contest, filter,  speed,  delay */
char lastmsg[1000] = "";
char sc_device[40] = "/dev/dsp";

/*-------------------------------------keyer------------------------------*/
int cwkeyer = NO_KEYER;
int digikeyer = NO_KEYER;

char keyer_device[10] = "";	// ttyS0, ttyS1, lp0-2 for net_keyer
bool keyer_backspace = false;   // disabled

char controllerport[80] = "/dev/ttyS0"; // for GMFSK or MFJ-1278
char rttyoutput[120];		// where to GMFSK digimode output
rmode_t digi_mode = RIG_MODE_NONE;

int txdelay = 0;
int weight = 0;
char weightbuf[4];
int cqdelay = 8;
int k_pin14 = 0;
int k_ptt = 0;

int miniterm = 0;		/* is miniterm for digimode active? */
char modem_mode[8];
int commentfield = 0;		/* 1 if we are in comment/excahnge input */

/*-------------------------------------packet-------------------------------*/
char spot_ptr[MAX_SPOTS][82];		/* Array of cluster spot lines */
int nr_of_spots;			/* Anzahl Lines in spot_ptr array */
int packetinterface = 0;
int fdSertnc = 0;
char tncportname[40];
char rigconf[80];
int tnc_serial_rate = 2400;
char clusterlogin[80] = "";
bool bmautoadd = false;
bool bmautograb = false;

/*-------------------------------------rigctl-------------------------------*/
int myrig_model = 0;            /* unset */
RIG *my_rig;			/* handle to rig (instance) */
rmode_t rmode;			/* radio mode of operation */
pbwidth_t width;
vfo_t vfo;			/* vfo selection */
port_t myport;
int ssb_bandwidth = 3000;
int cw_bandwidth = 0;
int serial_rate = 2400;
char *rigportname;
int rignumber = 0;
int rig_comm_error = 0;
int rig_comm_success = 0;

/*----------------------------------fldigi---------------------------------*/
char fldigi_url[50] = "http://localhost:7362/RPC2";

/*-------------------------------the log lines-----------------------------*/
char qsos[MAX_QSOS][LOGLINELEN + 1];
int nr_qsos = 0;

/*------------------------------dupe array---------------------------------*/
int nr_worked = 0;		/**< number of calls in worked[] */
worked_t worked[MAX_CALLS]; 	/**< worked stations */

/*----------------------statistic of worked countries,zones ... -----------*/
int countries[MAX_DATALINES];	/* per country field with worked bands set */
int zones[MAX_ZONES];		/* same for cq zones or itu zones;
				   using 1 - 40 or 1 - 90 */

mults_t multis[MAX_MULTS]; 	/**< worked multis */
int nr_multis = 0;		/**< number of multis in multis[] */

int multlist = 0;

int callareas[20];
int multscore[NBANDS];

struct ie_list *main_ie_list = NULL;	/* head of initial exchange list */

int zonescore[NBANDS];
int countryscore[NBANDS];
int zonedisplay = 0;
int new_zone = 0;		/* index of new zone */
int new_cty = 0;		/* index of new country */
int new_mult = -1;
int minute_timer = 0;

int bandinx = BANDINDEX_40;	/* start with 40m */
int qsonum = 1;			/* nr of next QSO */
int ymax, xmax;			/* screen size */

struct tm time_ptr_cabrillo;

freq_t freq;
bool logfrequency = false;
bool rit;
bool trx_control = false;
freq_t bandfrequency[NBANDS] = {
    1830000, 3525000, 5352000, 7010000, 10105000, 14025000, 18070000, 21025000, 24900000,
    28025000, 0.
};

char fkey_header[60] =
    "1=CQ  2=DE  3=RST 4=73  5=HIS  6=MY  7=B4   8=AGN  9=?";
const char *backgrnd_str;

char logline_edit[5][LOGLINELEN + 1];

char termbuf[88] = "";
int termbufcount = 0;

double DEST_Lat = 51.;
double DEST_Long = 1.;

char hiscountry[40];

bool wazmult = false;		/* to add the ability of WAZ zones to be multiplier */
bool itumult = false;		/* to add the ability of ITU zones to be multiplier */
char itustr[3];

bool nopacket = false;		/* set if tlf is called with '-n' */
bool no_trx_control = false;	/* set if tlf is called with '-r' */
bool convert_cabrillo = false;  /* set if the arg input is a cabrillo */
int do_cabrillo = 0;		/* actually converting cabrillo file to Tlf log */

int bandweight_points[NBANDS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
int bandweight_multis[NBANDS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};

pthread_t background_thread;
static struct termios oldt, newt;

/*-------------------------parse program options---------------------------*/
const char *argp_program_version = "tlf-" VERSION;
const char *argp_program_bug_address = "<tlf-devel@nongnu.org>";
static const char program_description[] =
    "tlf - contest logging program for amateur radio operators";
static const struct argp_option options[] = {
    {
	"config",   'f', "FILE", 0,
	"Use FILE as configuration file instead of logcfg.dat in the current directory"
    },
    {"import",      'i', 0, 0,  "Import Cabrillo file to Tlf format"},
    {"no-cluster",  'n', 0, 0,  "Start without cluster hookup" },
    {"no-rig",      'r', 0, 0,  "Start without radio control" },
    {"list",	    'l', 0, 0,  "List built-in contests" },
    {"sync",        's', "URL", 0,  "Synchronize log with other node" },
    {"debug",       'd', 0, 0,  "Debug mode" },
    {"verbose",     'v', 0, 0,  "Produce verbose output" },
    { 0 }
};

/* parse a single option */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
	case 'f':		// config file
	    config_file = g_strdup(arg);
	    break;
	case 'n':		// disable packet
	    nopacket = true;
	    break;
	case 'r':
	    no_trx_control = true; // disable radio control
	    break;
	case 'i':
	    convert_cabrillo = true;
	    break;
	case 'l':
	    list_contests();
	    exit(EXIT_SUCCESS);
	    break;
	case 's':
	    if (strlen(arg) >= 120) {
		printf("Argument too long for sync\n");
		exit(EXIT_FAILURE);
	    }
	    strcpy(synclogfile, arg);
	    break;
	case 'd':		// debug rigctl
	    debugflag = true;
	    break;
	case 'v':		// verbose startup
	    verbose = true;
	    break;

	default:
	    return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


const static struct argp argp = { options, parse_opt, NULL, program_description };


/** initialize user interface */
static void ui_init() {

    /* modify stdin terminals attributes to allow Ctrl-Q/S key recognition */
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_iflag &= ~(IXON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    /* getting users terminal string and (if RXVT) setting rxvt colours on it */
    /* LZ3NY hack :) */
    char *term = getenv("TERM");
    if (strcasecmp(term, "rxvt") == 0 ||
	    strcasecmp(term, "rxvt-unicode") == 0) {
	use_rxvt = true;
    } else if (strcasecmp(term, "xterm") == 0 ||
	       strcasecmp(term, "xterm-256color") == 0) {
	use_xterm = true;
	use_rxvt = true;
    }

    /* Check the environment variable ESCDELAY.
     *
     * If unset set it to 25 mS which should allow enough time to capture
     * escaped key codes and yet be fast enough to call stoptx() when needed
     * by the user.  When unset Ncurses assumes a default value of 1000 mS so
     * use set_escdelay(0) to set it as the Ncurses documentation declares this
     * method to be thread safe.
     *
     * Else let Ncurses honor the user defined value of the env variable.
     */
    if (getenv("ESCDELAY") == NULL) {
	set_escdelay(25);
    }

    /* activate ncurses terminal control */
    if ((mainscreen = newterm(NULL, stdout, stdin)) == NULL) {
	perror("initscr");
	printf
	("\nSorry, wrong terminal type !!!!! \nTry a  linux text terminal or set TERM=linux !!!\n");
	sleep(2);

	exit(EXIT_FAILURE);
    }

    InitSearchPanel();	/* at least one panel has to be defined
				   for refreshp() to work */

    getmaxyx(stdscr, ymax, xmax);
    if ((ymax < 22) || (xmax < 80)) {
	char c;

	showmsg("!! TLF works best with at least 22 lines and 80 columns !!");
	showmsg("   Continue anyway? Y/(N) ");
	c = toupper(key_get());
	if (c != 'Y') {
	    showmsg("73 es cuagn");
	    sleep(1);
	    endwin();
	    exit(EXIT_FAILURE);
	}
	clearmsg();
    }

    if (!has_colors() || (start_color() == ERR)) {
	showmsg("Sorry, terminal does not support color");
	showmsg("Try TERM=linux  or use a text console !!");
	sleep(1);
	endwin();
	exit(EXIT_FAILURE);
    }

    refreshp();

    noecho();
    cbreak();

    keypad(stdscr, TRUE);

    lookup_keys();
}


/* setup colors */
static void ui_color_init() {

    if (use_rxvt) {		// use rxvt colours
	init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_RED);
	if (use_xterm) {
	    init_pair(C_HEADER, COLOR_GREEN, COLOR_BLUE);
	    init_pair(COLOR_RED, COLOR_WHITE, 8);
	    init_pair(C_WINDOW, COLOR_CYAN, COLOR_MAGENTA);
	    init_pair(C_DUPE, COLOR_WHITE, COLOR_MAGENTA);
	    init_pair(C_INPUT, COLOR_BLUE, COLOR_WHITE);
	} else {
	    init_pair(C_HEADER, COLOR_GREEN, COLOR_YELLOW);
	    init_pair(COLOR_RED, COLOR_WHITE, COLOR_RED);
	    init_pair(C_WINDOW, COLOR_CYAN, COLOR_RED);
	    init_pair(C_DUPE, COLOR_RED, COLOR_MAGENTA);
	    init_pair(C_INPUT, COLOR_BLUE, COLOR_YELLOW);
	}
	init_pair(C_LOG, COLOR_WHITE, COLOR_BLACK);
	init_pair(C_BORDER, COLOR_CYAN, COLOR_YELLOW);
    } else {
	// use linux console colours
	init_pair(COLOR_BLACK, tlfcolors[0][0], tlfcolors[0][1]); // b/w
	init_pair(C_HEADER, tlfcolors[1][0], tlfcolors[1][1]);    // Gn/Ye
	init_pair(COLOR_RED, tlfcolors[2][0], tlfcolors[2][1]);   // W/R
	init_pair(C_WINDOW, tlfcolors[3][0], tlfcolors[3][1]);    // Cy/W
	init_pair(C_LOG, tlfcolors[4][0], tlfcolors[4][1]);       // W/B
	init_pair(C_DUPE, tlfcolors[5][0], tlfcolors[5][1]);      // W/Mag
	init_pair(C_INPUT, tlfcolors[6][0], tlfcolors[6][1]);     // Bl/Y
	init_pair(C_BORDER, tlfcolors[7][0], tlfcolors[7][1]);    // W/B
    }
}

void center_fkey_header() {
    int width = sizeof(fkey_header) - 1;
    if (strlen(fkey_header) == width) {
	return;     // already OK
    }
    int right_padding = (width - strlen(fkey_header)) / 2;
    int left_padding = width - strlen(fkey_header) - right_padding;
    char tmp[sizeof(fkey_header)];
    strcpy(tmp, fkey_header);
    sprintf(fkey_header, "%s%s%s",
	    spaces(left_padding), tmp, spaces(right_padding));
}

static void init_variables() {

    iscontest = false;
    partials = false;
    use_part = false;
    cwkeyer = NO_KEYER;
    digikeyer = NO_KEYER;
    portnum = 0;
    packetinterface = 0;
    nodes = 0;
    shortqsonr = 0;
    tune_seconds = 6;   /* tune up for 6 s */

    ctcomp = false;

    for (int i = 0; i < 25; i++) {
	FREE_DYNAMIC_STRING(digi_message[i]);
    }

    FREE_DYNAMIC_STRING(cabrillo);

}

/** load all databases
 *
 * \return EXIT_FAILURE if not successful */
static int databases_load() {
    int status;

    showmsg("Reading country data");
    readctydata();		/* read ctydb.dat */

    init_variables();

    showmsg("Reading configuration data");

    status = read_logcfg(); 	/* read the configuration file */
    if (status != PARSE_OK) {
	showmsg("Problems parsing logcfg.dat!");
	return EXIT_FAILURE;
    }

    status = read_rules();	/* read the additional contest rules
				   in "rules/contestname" */
    if (status != PARSE_OK) {
	showmsg("Problems parsing rule file!");
	return EXIT_FAILURE;
    }

    if (*my.call == '\0') {
	showmsg("ERROR: No callsign defined in logcfg.dat!\n");
	return EXIT_FAILURE;
    }


    if (multlist == 1) {
	showmsg("Reading multiplier data      ");
	if (strlen(multsfile) == 0) {
	    showmsg("No multiplier file specified!");
	    return EXIT_FAILURE;
	}
    }
    init_and_load_multipliers();

    showmsg("Reading callmaster data");
    load_callmaster();

    if (*exchange_list != '\0') {
	showmsg("Reading initial exchange file");
	main_ie_list = make_ie_list(exchange_list);

	if (main_ie_list == NULL) {
	    showmsg("Problems in initial exchange file detected!");
	    return EXIT_FAILURE;
	}
    }

    /* make sure logfile is there and has the right format */
    if (checklogfile_new(logfile) != 0) {
	showmsg("Can not access logfile. Giving up");
	return EXIT_FAILURE;
    }

    if (qtcdirection > 0) {
	if (checkqtclogfile() != 0) {
	    showmsg("QTCs giving up");
	    return EXIT_FAILURE;
	}
	readqtccalls();
    }
    // unset QTC_RECV_LAZY if mode is DIGIMODE
    if (trxmode == DIGIMODE) {
	qtc_recv_lazy = false;
    }
    return EXIT_SUCCESS;
}

static void hamlib_init() {

    if (no_trx_control) {
	trx_control = false;
    }

    if (!trx_control) {
	return;
    }

    shownr("Rig model number is", myrig_model);
    shownr("Rig speed is", serial_rate);

    showmsg("Trying to start rig control");

    int status = init_tlf_rig();

    if (status != 0) {
	showmsg("Continue without rig control Y/(N)?");
	if (toupper(key_get()) != 'Y') {
	    endwin();
	    exit(1);
	}
	trx_control = false;
	showmsg("Disabling rig control!");
	sleep(1);
    }
}

static void fldigi_init() {
#ifdef HAVE_LIBXMLRPC
    int status;

    if (digikeyer == FLDIGI) {
	xmlrpc_showinfo();
	status = fldigi_xmlrpc_init();
	if (status != 0) {
	    digikeyer = NO_KEYER;
	}
    }
#endif
}

static void lan_init() {
    if (lan_active) {
	if (lan_recv_init() < 0)	/* set up the network */
	    showmsg("LAN receive  init failed");
	else
	    showmsg("LAN receive  initialized");

	if (lan_send_init() < 0)
	    showmsg("LAN send init failed");
	else
	    showmsg("LAN send initialized");
    }
}


static void packet_init() {
    if (nopacket)
	packetinterface = 0;

    set_term(mainscreen);

    // really needed?
    refreshp();

    if (!nopacket && packetinterface > 0) {
	if (init_packet() == 0)
	    packet();
	else
	    cleanup_telnet();
    }
}


static void keyer_init() {
    char keyerbuff[3];

    if (cwkeyer == NET_KEYER) {
	showmsg("CW-Keyer is cwdaemon");

	if (netkeyer_init() < 0) {
	    showmsg("Cannot open NET keyer daemon ");
	    refreshp();
	    sleep(1);

	} else {
	    netkeyer(K_RESET, "0");

	    sprintf(weightbuf, "%d", weight);

	    write_tone();

	    snprintf(keyerbuff, 3, "%2u", GetCWSpeed());
	    netkeyer(K_SPEED, keyerbuff);		// set speed

	    netkeyer(K_WEIGHT, weightbuf);		// set weight

	    if (*keyer_device != '\0')
		netkeyer(K_DEVICE, keyer_device);	// set device

	    sprintf(keyerbuff, "%d", txdelay);
	    netkeyer(K_TOD, keyerbuff);			// set TOD

	    if (sc_sidetone)				// set soundcard output
		netkeyer(K_SIDETONE, "");

	    if (*sc_volume != '\0')			// set soundcard volume
		netkeyer(K_STVOLUME, sc_volume);
	}

    }

    if (cwkeyer == HAMLIB_KEYER) {
	showmsg("CW-Keyer is Hamlib");
	if (!trx_control) {
	    showmsg("Radio control is not activated!!");
	    sleep(1);
	    endwin();
	    exit(EXIT_FAILURE);
	}
	if (!rig_has_send_morse()) {
	    showmsg("Rig does not support CW via Hamlib");
	    sleep(1);
	    endwin();
	    exit(EXIT_FAILURE);
	}
	if (!rig_has_stop_morse()) {
#if HAMLIB_VERSION >= 400
	    showmsg("Rig does not support stopping CW!!");
#else
	    showmsg("Hamlib version does not supprt stopping CW!!");
#endif
	    showmsg("Continue anyway Y/(N)?");
	    if (toupper(key_get()) != 'Y') {
		endwin();
		exit(1);
	    }
	}
    }

    if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER ||
	    digikeyer == GMFSK) {
	init_controller();
    }

}


static void show_GPL() {
    printw("\n\n\n");
    printw("           TTTTT  L      FFFFF\n");
    printw("             T    L      F    \n");
    printw("             T    L      FFFFF\n");
    printw("             T    L      F    \n");
    printw("             T    LLLLL  F    \n");
    printw("\n\nThis program is copyright 2002, 2003, 2004 by Rein Couperus, PA0R\n\n");
    printw("It is free software; you can redistribute it and/or modify it under the terms\n");
    printw("of the GNU General Public License as published by the Free Software Foundation;\n");
    printw("either version 2 of the License, or (at your option) any later version.\n\n");
    printw("This program is distributed in the hope that it will be useful, but\n");
    printw("WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n");
    printw("or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License\n");
    printw("for more details.\n");

    refreshp();
}


static int isFirstStart() {
    FILE *fp;

    if ((fp = fopen(".paras", "r")) == NULL) {
	return 1;
    }
    fclose(fp);

    return 0;
}

/* write empty .paras file to remember that tlf got already started once
 * in these directory and GPL has been shown
 */
static void mark_GPL_seen() {

    FILE *fp = fopen(".paras", "w");
    if (fp) {
	fclose(fp);
    }
}


/** cleanup function
 *
 * Cleanup initialisations made by tlf. Will be called after exit() from
 * logit() or background_process()
 */
static void tlf_cleanup() {
    if (pthread_self() != background_thread) {
	pthread_cancel(background_thread);
	pthread_join(background_thread, NULL);
    }

    cleanup_telnet();

    if (trxmode == CWMODE && cwkeyer == NET_KEYER)
	netkeyer_close();
    else
	deinit_controller();

    if (my_rig) {
	close_tlf_rig(my_rig);
    }

#ifdef HAVE_LIBXMLRPC
    if (digikeyer == FLDIGI) {
	fldigi_xmlrpc_cleanup();
    }
#endif

    endwin();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    puts("\n\nThanks for using Tlf.. 73\n");
}

static void ignore(int sig) {
    // no action except ignoring Ctrl-C
}

/* ------------------------------------------------------------------------*/
/*     Main loop of the program			                           */
/* ------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    int j;
    int ret;
    char welcome[80];

    backgrnd_str = spaces(80);

    sprintf(welcome, "        Welcome to %s by PA0R!!", argp_program_version);

    argp_parse(&argp, argc, argv, 0, 0, NULL);  // parse options

    ui_init();

    rst_init(NULL);

    strcat(logline0, backgrnd_str);
    strcat(logline1, backgrnd_str);
    strcat(logline2, backgrnd_str);
    strcat(logline3, backgrnd_str);
    strcat(logline4, backgrnd_str);

    init_terminal_strings();

    termbuf[0] = '\0';
    hiscall[0] = '\0';


    if (isFirstStart()) {
	/* first time called in this directory */
	verbose = true;
	printw("%s", welcome);
	show_GPL();
	mark_GPL_seen();
	sleep(5);
	clear();
    }

    showmsg(welcome);
    showmsg("");

    memset(&my, 0, sizeof(my));

    total = 0;
    if (databases_load() == EXIT_FAILURE) {
	showmsg("**** Press any key to exit...");
	key_get();
	showmsg("73...");
	sleep(2);
	endwin();
	exit(EXIT_FAILURE);
    }

    if (convert_cabrillo) {
	char *tstring =
	    g_strdup_printf("Converting Cabrillo for contest %s for %s",
			    whichcontest, my.call);
	showmsg(tstring);
	g_free(tstring);
	showmsg("");
	getstationinfo();
	if (0 != readcabrillo(READCAB_MODE_CLI))
	    showmsg("Sorry. Conversion failed....");
	else
	    showmsg("Done...");
	sleep(2);
	endwin();
	exit(EXIT_SUCCESS);
    }

    /* now setup colors */
    ui_color_init();

//              if (strlen(synclogfile) > 0)
//                      synclog(synclogfile);

    hamlib_init();
    fldigi_init();
    lan_init();
    keyer_init();

    nr_qsos = readcalls(logfile);   /* read the logfile and rebuild
				       point and multiplier scoring */

    scroll_log();		/* show the last 5  log lines and
				   set the next serial number */
    show_station_info();
    clearmsg_wait();

    packet_init();

    center_fkey_header();
    clear_display();		/* tidy up the display */
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    for (j = 13; j <= LINES - 1; j++) {	/* wipe lower window */
	clear_line(j);
    }
    refreshp();

    bm_init();			/* initialize bandmap */

    if (signal(SIGINT, SIG_IGN) != SIG_IGN) {   /* ignore Ctrl-C */
	signal(SIGINT, ignore);
    }
    atexit(tlf_cleanup); 	/* register cleanup function */

    /* Create the background thread */
    ret = pthread_create(&background_thread, NULL, background_process, NULL);
    if (ret) {
	perror("pthread_create: backgound_process");
	endwin();
	exit(EXIT_FAILURE);
    }

    /* now start logging  !! Does never return */
    logit();

    return 0;
}
