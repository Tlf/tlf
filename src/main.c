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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <argp.h>
#include <ctype.h>
#include <hamlib/rig.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "addmult.h"
#include "background_process.h"
#include "bandmap.h"
#include "checkparameters.h"
#include "clear_display.h"
#include "checklogfile.h"
#include "checkqtclogfile.h"
#include "cw_utils.h"
#include "fldigixmlrpc.h"
#include "getmessages.h"
#include "getwwv.h"
#include "globalvars.h"		// Includes glib.h and tlf.h
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
#include "set_tone.h"
#include "splitscreen.h"
#include "startmsg.h"
#include "tlf_panel.h"
#include "ui_utils.h"
#include "readcabrillo.h"

#include <config.h>


SCREEN *mainscreen;

extern int lan_active;

char pr_hostaddress[48] = "131.155.192.179";
char *config_file = NULL;
int portnum = 0;

int use_rxvt = 0;
int use_xterm = 0;

int tlfcolors[8][2] = { {COLOR_BLACK, COLOR_WHITE},
    {COLOR_GREEN, COLOR_YELLOW},
    {COLOR_WHITE, COLOR_RED},
    {COLOR_CYAN, COLOR_WHITE},
    {COLOR_WHITE, COLOR_BLACK},
    {COLOR_WHITE, COLOR_MAGENTA},
    {COLOR_BLUE, COLOR_YELLOW},
    {COLOR_WHITE, COLOR_BLACK}
};
int debugflag = 0;
char *editor_cmd = NULL;
int tune_val = 0;
int use_bandoutput = 0;
int no_arrows = 0;
int bandindexarray[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int cqwwm2 = 0;

/* predefined contests */
int cqww = 0;
int wpx = 0;
int dxped = 0;
int sprint = 0;
int arrldx_usa = 0;
int arrl_fd = 0;
int arrlss = 0;
int pacc_pa_flg = 0;
int stewperry_flg = 0;
int focm = 0;
int sprint_mode = 0;
int minitest = 0;	/**< if set, length of minitest period in seconds */
int unique_call_multi = 0;          /* do we count calls as multiplier */

int universal = 0;

int addcallarea;
int pfxmult = 0;
int pfxmultab = 0;
int exc_cont = 0;
int manise80;
int other_flg;
int one_point = 0;
int two_point = 0;
int three_point = 0;
int ssbpoints;
int cwpoints;
int lowband_point_mult = 0;
int sc_sidetone;
char sc_volume[4] = "";
/* LZ3NY mods */
int my_country_points = -1;
int my_cont_points = -1;
int dx_cont_points = -1;
char countrylist[255][6];
int countrylist_only = 0;
int countrylist_points = -1;
char continent_multiplier_list[7][3]; // SA, NA, EU, AF, AS and OC
int continentlist_points = -1;
int continentlist_only = 0;
int exclude_multilist_type = 0;
int mult_side = 0;
/* end LZ3NY mods */

int portable_x2 = 0;
int recall_mult = 0;
int exchange_serial = 0;
int wysiwyg_once = 0;
int wysiwyg_multi = 0;
int country_mult = 0;
float fixedmult = 0.0;
int sectn_mult = 0;
int dx_arrlsections = 0;
int serial_section_mult = 0;
int serial_or_section = 0;	/* exchange is serial OR section, like HA-DX */
int serial_grid4_mult = 0;
int qso_once = 0;
int addcallarea_once = 0;
int noleadingzeros;
int ctcomp = 0;
int isdupe = 0;			// 0 if nodupe -- for auto qso b4 (LZ3NY)
int nob4 = 0;			// allow auto b4
int ignoredupe = 0;
int noautocq = 0;
int verbose = 0;
int no_rst = 0;			/* 1 - do not use RS/RST */
char myqra[7] = "";

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

t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];
int pfxnummultinr = 0;

char multsfile[80] = "";	/* name of file with a list of allowed
				   multipliers */
char exchange_list[40] = "";
int timeoffset = 0;
int trxmode = CWMODE;
rmode_t  rigmode = RIG_MODE_NONE;

int mixedmode = 0;
char his_rst[4] = "599";
char my_rst[4] = "599";
char last_rst[4] = "599";       /* Report for last QSO */

/* TODO Maybe we can use the following */
int mults_per_band = 1;		/* mults count per band */

int shortqsonr = LONGCW;	/* 1  =  short  cw char in exchange */
int cluster = NOCLUSTER;	/* 0 = OFF, 1 = FOLLOW, 2  = spots  3 = all */
int clusterlog = 0;		/* clusterlog on/off */
int searchflg = 0;		/* 1  = display search  window */
int show_time = 0;
cqmode_t cqmode = CQ;
int demode = 0;			/* 1 =  send DE  before s&p call  */
int contest = 0;		/* 0 =  General,  1  = contest */
int announcefilter = FILTER_ANN; /*  filter cluster  announcements */
int showscore_flag = 0;		/* show  score window */
int change_rst = 0;
char exchange[40];
char whichcontest[40] = "qso";
int defer_store = 0;
char call[20];
char logfile[120] = "general.log";
char *cabrillo = NULL;		/**< Name of the cabrillo format definition */
char synclogfile[120];
char markerfile[120] = "";
int xplanet = 0;
unsigned char rigptt = 0;
/**< Bitmask for Hamlib CAT PTT
 * bit 0 set: CAT PTT wanted--RIGPTT in logcfg.dat (set in parse_logcfg)
 * bit 1 set: CAT PTT available--from rig caps (set in sendqrg)
 * bit 2 set: PTT active (set/unset in gettxinfo)
 * bit 3 set: PTT On (set/unset in callinput)
 * bit 4 set: PTT Off (set/unset in callinput)
 */

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
    "TEST %\n", "@ DE %\n", "@ [\n", "TU 73\n", " @\n", "%\n",
    "@ SRI QSO B4 GL\n", "AGN\n",
    " ?\n", " QRZ?\n", " PSE K\n", "TEST % %\n", "@ [\n", "TU %\n",
    "", "", "", "", "", "", "", "", "", "", ""
};

char *digi_message[sizeof(message) / sizeof(message[0])];

char ph_message[14][80] = /**< Array of file names for voice keyer messages
			   * See description of message[]
			   */
{ "", "", "", "", "", "", "", "", "", "", "", "", "", "" };

char qtc_recv_msgs[12][80] = {"QTC?\n", "QRV\n", "R\n", "", "TIME?\n", "CALL?\n", "NR?\n", "AGN\n", "", "QSL ALL\n", "", ""}; // QTC receive windowS Fx messages
char qtc_send_msgs[12][80] = {"QRV?\n", "QTC sr/nr\n", "", "", "TIME\n", "CALL\n", "NR\n", "", "", "", "", ""}; // QTC send window Fx messages
char qtc_phrecv_message[14][80] = { "", "", "", "", "", "", "", "", "", "", "", "" };	// voice keyer file names when receives QTC's
char qtc_phsend_message[14][80] = { "", "", "", "", "", "", "", "", "", "", "", "" };	// voice keyer file names when send QTC's
int qtcrec_record = 0;
char qtcrec_record_command[2][50] = {"rec -q 8000", "-q &"};
char qtcrec_record_command_shutdown[50] = "pkill -SIGINT -n rec";
char qtc_cap_calls[40] = "";
int qtc_auto_filltime = 0;
int qtc_recv_lazy = 0;

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
char qsonrstr[5] = "0001";
char band[NBANDS][4] =
{ "160", " 80", " 60", " 40", " 30", " 20", " 17", " 15", " 12", " 10", "???" };
char comment[80];
char cqzone[3] = "";
char mycqzone[3] = "";
char ituzone[3] = "";
char continent[3] = "";
char mycontinent[3] = "";
char pxstr[11] = "";
int totalmults = 0;
int totalcountries = 0;
int totalzones = 0;
int secs = 0;
int countrynr;
int mycountrynr = 215;
int total = 0; 		/**< total number of qso points */
int band_score[NBANDS];
int dupe = 0;
int callfound = 0;
int partials = 0;	/**< show partial calls */
int use_part = 0;	/**< if 1 use automatically found partial call */
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
int keyer_backspace = 0;        // disabled

char controllerport[80] = "/dev/ttyS0"; // for GMFSK or MFJ-1278
char rttyoutput[120];		// where to GMFSK digimode output
rmode_t digi_mode = RIG_MODE_NONE;

int txdelay = 0;
int weight = 0;
char weightbuf[4];
char tonestr[5] = "600";
int cqdelay = 8;
int k_tune;
int k_pin14;
int k_ptt;

int miniterm = 0;		/* is miniterm for digimode active? */
char modem_mode[8];
int commentfield = 0;		/* 1 if we are in comment/excahnge input */

/*-------------------------------------packet-------------------------------*/
char spot_ptr[MAX_SPOTS][82];		/* Array of cluster spot lines */
int nr_of_spots;			/* Anzahl Lines in spot_ptr array */
int packetinterface = 0;
int fdSertnc = 0;
int tncport = 1;
char tncportname[40];
char rigconf[80];
int tnc_serial_rate = 2400;
char clusterlogin[80] = "";
int bmautoadd = 0;
int bmautograb = 0;

/*-------------------------------------rigctl-------------------------------*/
rig_model_t myrig_model = 351;  /* Ten-Tec Omni VI Plus */
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

/*---------------------------------simulator-------------------------------*/
int simulator = 0;
int simulator_mode = 0;
int simulator_seed = 8327;
int system_secs;
char tonecpy[5];
char simulator_tone[5];

/*-------------------------------the log lines-----------------------------*/
char qsos[MAX_QSOS][LOGLINELEN + 1];
int nr_qsos = 0;

/*------------------------------dupe array---------------------------------*/
int nr_worked = 0;		/**< number of calls in worked[] */
struct worked_t worked[MAX_CALLS]; /**< worked stations */

/*----------------------statistic of worked countries,zones ... -----------*/
int countries[MAX_DATALINES];	/* per country bit fieldwith worked bands set */
int zones[MAX_ZONES];		/* same for cq zones or itu zones;
				   using 1 - 40 or 1 - 90 */

struct mults_t multis[MAX_MULTS]; /**< worked multis */
int nr_multis = 0;		/**< number of multis in multis[] */

int multlist = 0;

int callareas[20];
int multscore[NBANDS];

struct ie_list *main_ie_list = NULL;	/* head of initial exchange list */

int zonescore[NBANDS];
int countryscore[NBANDS];
int zonedisplay = 0;
int addzone = 0;		/* flag for new zone */
int addcty = 0;			/* flag for new country */
int shownewmult = -1;
int minute_timer = 0;

int bandinx = BANDINDEX_40;	/* start with 40m */
int qsonum = 1;			/* nr of next QSO */
int ymax, xmax;			/* screen size */

pid_t pid;
struct tm *time_ptr, time_ptr_cabrillo;

freq_t freq;
int logfrequency = 0;
int rit;
int trx_control = 0;
freq_t bandfrequency[NBANDS] = {
    1830000, 3525000, 5352000, 7010000, 10105000, 14025000, 18070000, 21025000, 24900000,
    28025000, 0.
};

const char headerline[] =
    "   1=CQ  2=DE  3=RST 4=73  5=HIS  6=MY  7=B4   8=AGN  9=?  ";
const char *backgrnd_str;

char logline_edit[5][LOGLINELEN + 1];

char terminal1[88] = "";
char terminal2[88] = "";
char terminal3[88] = "";
char terminal4[88] = "";
char termbuf[88] = "";
int termbufcount = 0;

double QTH_Lat = 51.;
double QTH_Long = -7.;
double DEST_Lat = 51.;
double DEST_Long = 1.;

char hiscountry[40];

int this_second;

int wazmult = 0;		/* to add the ability of WAZ zones to be multiplier */
int itumult = 0;		/* to add the ability of ITU zones to be multiplier */
char itustr[3];

int nopacket = 0;		/* set if tlf is called with '-n' */
int no_trx_control = 0;		/* set if tlf is called with '-r' */
int convert_cabrillo = 0;       /* set if the arg input is a cabrillo */
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
    {"import",      'i', 0, 0,  "Import cabrillo file to Tlf format"},
    {"no-cluster",  'n', 0, 0,  "Start without cluster hookup" },
    {"no-rig",      'r', 0, 0,  "Start without radio control" },
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
	    nopacket = 1;
	    break;
	case 'r':
	    no_trx_control = 1; // disable radio control
	    break;
	case 'i':
	    convert_cabrillo = 1;
	    break;
	case 's':
	    if (strlen(arg) >= 120) {
		printf("Argument too long for sync\n");
		exit(EXIT_FAILURE);
	    }
	    strcpy(synclogfile, arg);
	    break;
	case 'd':		// debug rigctl
	    debugflag = 1;
	    break;
	case 'v':		// verbose startup
	    verbose = 1;
	    break;

	default:
	    return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


const static struct argp argp = { options, parse_opt, NULL, program_description };


/** initialize user interface */
void ui_init() {

    /* modify stdin terminals attributes to allow Ctrl-Q/S key recognition */
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_iflag &= ~(IXON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    /* getting users terminal string and (if RXVT) setting rxvt colours on it */
    /* LZ3NY hack :) */
    char *term = getenv("TERM");
    if (strcasecmp(term, "rxvt") == 0) {
	use_rxvt = 1;
    } else if ((strcasecmp(term, "xterm") == 0) ||
	       (strcasecmp(term, "xterm-256color") == 0)) {
	use_xterm = 1;
	use_rxvt = 1;
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
    if ((ymax < 25) || (xmax < 80)) {
	char c;

	showmsg("!! TLF needs at least 25 lines and 80 columns !!");
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
void ui_color_init() {

    if (use_rxvt == 1) {	// use rxvt colours
	init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_RED);
	if (use_xterm == 1) {
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


/** load all databases
 *
 * \return EXIT_FAILURE if not successful */
int databases_load() {
    int status;

    showmsg("Reading country data");
    readctydata();		/* read ctydb.dat */

    showmsg("Reading configuration data");
    status = read_logcfg(); 	/* read the configuration file */
    status |= read_rules();	/* read the additional contest rules
				   in "rules/contestname" */
    if (status != PARSE_OK) {
	showmsg("Problems in logcfg.dat or rule file detected! Continue Y/(N)?");
	if (toupper(key_get()) != 'Y') {
	    showmsg("73...");
	    return EXIT_FAILURE;
	}
    }

    if (*call == '\0') {
	showmsg
	("WARNING: No callsign defined in logcfg.dat! exiting...\n");
	return EXIT_FAILURE;
    }


    if (multlist == 1) {
	showmsg("Reading multiplier data      ");
	if (strlen(multsfile) == 0) {
	    showmsg("No multiplier file specified, exiting.. !!");
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
	    showmsg("Problems in initial exchange file detected! Continue Y/(N)?");
	    if (toupper(key_get()) != 'Y') {
		showmsg("73...");
		return EXIT_FAILURE;
	    }
	}
    }

    /* make sure logfile is there and has the right format */
    if (checklogfile_new(logfile) != 0) {
	showmsg("Can not access logfile. Giving up");
	return EXIT_FAILURE;
    }

    if (qtcdirection > 0) {
	if (checkqtclogfile() != 0) {
	    showmsg("QTC's giving up");
	    return EXIT_FAILURE;
	}
	readqtccalls();
    }
    // unset QTC_RECV_LAZY if mode is DIGIMODE
    if (trxmode == DIGIMODE) {
	qtc_recv_lazy = 0;
    }
    return 0;
}

void hamlib_init() {

    int status;

    if (no_trx_control == 1) {
	trx_control = 0;
    }

    if (trx_control != 0) {

	shownr("Rignumber is", (int) myrig_model);
	shownr("Rig speed is", serial_rate);

	showmsg("Trying to start rig ctrl");

	status = init_tlf_rig();

	if (status  != 0) {
	    showmsg("Continue without rig control Y/(N)?");
	    if (toupper(key_get()) != 'Y') {
		endwin();
		exit(1);
	    }
	    trx_control = 0;
	    showmsg("Disabling rig control!");
	    sleep(1);
	}
    }
}

void fldigi_init() {
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

void lan_init() {
    if (lan_active == 1) {
	if (lanrecv_init() < 0)	/* set up the network */
	    showmsg("LAN receive  init failed");
	else
	    showmsg("LAN receive  initialized");

	if (lan_send_init() < 0)
	    showmsg("LAN send init failed");
	else
	    showmsg("LAN send initialized");
    }
}


void packet_init() {
    if (nopacket == 1)
	packetinterface = 0;

    set_term(mainscreen);

    // really needed?
    refreshp();

    if ((nopacket == 0) && (packetinterface != 0)) {
	if (init_packet() == 0)
	    packet();
	else
	    cleanup_telnet();
    }
}


void keyer_init() {
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
	    netkeyer(K_TOD, keyerbuff);		// set TOD

	    if (sc_sidetone != 0)			// set soundcard output
		netkeyer(K_SIDETONE, "");

	    if (*sc_volume != '\0')			// set soundcard volume
		netkeyer(K_STVOLUME, sc_volume);
	}

    }

    if (cwkeyer == MFJ1278_KEYER || digikeyer == MFJ1278_KEYER ||
	    digikeyer == GMFSK) {
	init_controller();
    }

}


void show_GPL() {
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


int isFirstStart() {
    FILE *fp;

    if ((fp = fopen(".paras", "r")) == NULL) {
	return 1;
    }
    fclose(fp);

    return 0;
}

/** cleanup function
 *
 * Cleanup initialisations made by tlf. Will be called after exit() from
 * logit() or background_process()
 */
void tlf_cleanup() {
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


    strcat(logline0, backgrnd_str);
    strcat(logline1, backgrnd_str);
    strcat(logline2, backgrnd_str);
    strcat(logline3, backgrnd_str);
    strcat(logline4, backgrnd_str);

    strcat(terminal1, backgrnd_str);
    strcat(terminal2, backgrnd_str);
    strcat(terminal3, backgrnd_str);
    strcat(terminal4, backgrnd_str);

    termbuf[0] = '\0';
    hiscall[0] = '\0';


    if (isFirstStart()) {
	/* first time called in this directory */
	verbose = 1;
	printw(welcome);
	show_GPL();
	sleep(5);
	clear();
    }

    showmsg(welcome);
    showmsg("");

    total = 0;
    if (databases_load() == EXIT_FAILURE) {
	sleep(2);
	endwin();
	exit(EXIT_FAILURE);
    }

    if (convert_cabrillo == 1) {
	char tstring[100] = "";
	sprintf(tstring, "Converting cabrillo for contest %s from file %s.cbr",
		whichcontest, g_strstrip(call));
	showmsg(tstring);
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

    scroll_log();		/* read the last 5  log lines and set the next serial number */
    nr_qsos = readcalls();	/* read the logfile for score and dupe */

    checkparameters();		/* check .paras file */
    getmessages();		/* read .paras file */

    clearmsg_wait();

    packet_init();

    clear_display();		/* tidy up the display */
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    for (j = 13; j <= LINES - 1; j++) {	/* wipe lower window */
	mvprintw(j, 0, backgrnd_str);
    }
    refreshp();

    bm_init();			/* initialize bandmap */

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
