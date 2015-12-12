/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0r@eudxf.org>
 *                    2010-2014 Thomas Beierlein <tb@forth-ev.de>
 *                    2013-2014 Ervin Hegedus - HA2OS <airween@gmail.com>
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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <panel.h>

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
#include "sendqrg.h"		// Sets HAVE_LIBHAMLIB if enabled
#include "set_tone.h"
#include "splitscreen.h"
#include "startmsg.h"
#include "ui_utils.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LIBHAMLIB
# include <hamlib/rig.h>
#endif


SCREEN *mainscreen;
SCREEN *packetscreen;
WINDOW *sclwin, *entwin;

extern int lan_active;

int prsock = 0;
char pr_hostaddress[48] = "131.155.192.179";
char *config_file = NULL;
int portnum = 0;
struct tln_logline *loghead = NULL;
struct tln_logline *logtail = NULL;
struct tln_logline *viewing = NULL;

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
int editor = EDITOR_JOE;
char rttyoutput[120];
int use_vk = 0;
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
int emptydir = 0;
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
int multi = 0;			/* 0 = SO , 1 = MOST, 2 = MM */
int trxmode = CWMODE;
int rigmode = 0;		/* RIG_MODE_NONE in hamlib/rig.h, but if hamlib not compiled, then no dependecy */
int mixedmode = 0;
char his_rst[4] = "599";
char my_rst[4] = "599";
char last_rst[4] = "599";       /* Report for last QSO */
int mults_per_band = 1;		/* mults count per band */
int shortqsonr = LONGCW;	/* 1  =  short  cw char in exchange */
int cluster = NOCLUSTER;	/* 0 = OFF, 1 = FOLLOW, 2  = spots  3 = all */
int clusterlog = 0;		/* clusterlog on/off */
int searchflg = 0;		/* 1  = display search  window */
int show_time = 0;
int cqmode = CQ;		/* 1  = CQ  0 = S&P  */
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
char *cabrillo = NULL;		/*< Name of the cabrillo format definition */
char synclogfile[120];
char markerfile[120] = "";
int xplanet = 0;

char sp_return[80] = " \n";
char cq_return[80] = " \n";
char message[25][80] = /**< Array of CW/DigiMode messages
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
    { "TEST %\n", "@ DE %\n", "@ [\n", "TU 73\n", " @\n", "%\n",
	"@ SRI QSO B4 GL\n", "AGN\n",
	" ?\n", " QRZ?\n", " PSE K\n", "TEST % %\n", "@ [\n", "TU %\n",
	"", "", "", "", "", "", "", "", "", "", "" };

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
char band[9][4] =
    { "160", " 80", " 40", " 30", " 20", " 17", " 15", " 12", " 10" };
char comment[80];
char mode[20] = "Log     ";
char cqzone[3] = "";
char mycqzone[3] = "";
char ituzone[3] = "";
char continent[3] = "";
char mycontinent[3] = "";
char pxstr[11] = "";
int bandindex = 0;
int totalmults = 0;
int totalcountries = 0;
int totalzones = 0;
int secs = 0;
int countrynr;
int mycountrynr = 215;
int total = 0; 		/**< total number of qso points */
int band_score[9];
int dupe = 0;
int callfound = 0;
int partials = 0;	/**< show partial calls */
int use_part = 0;	/**< if 1 use automatically found partial call */
int block_part = 0; 	/**< if 1 block the call autocompletion
			  for these QSO */
char para_word[80] = "LODNCFS:3C\n";	/* longcw, cluster, search,  DE, contest, filter,  speed,  delay */
char lastmsg[1000] = "";
int scale_values[20] =
    { 40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6,
4, 2 };
char sc_device[40] = "/dev/dsp";

/*-------------------------------------keyer------------------------------*/
int keyerport = NO_KEYER;
int txdelay = 0;
int weight = 0;
char weightbuf[4];
char tonestr[5] = "600";
int cqdelay = 8;
char wkeyerbuffer[400];
int data_ready = 0;
char keyer_device[10] = "";	// ttyS0, ttyS1, lp0-2
int k_tune;
int k_pin14;
int k_ptt;
char controllerport[80] = "/dev/ttyS0";
int miniterm = 0;		/* is miniterm for digimode active? */
char modem_mode[8];
int commentfield = 0;		/* 1 if we are in comment/excahnge input */

/*-------------------------------------packet-------------------------------*/
char spot_ptr[MAX_SPOTS][82];		/* Array of cluster spot lines */
int nr_of_spots;			/* Anzahl Lines in spot_ptr array */
char lastwwv[120] = "";
int packetinterface = 0;
int fdSertnc = 0;
int fdFIFO = 0;
int tncport = 1;
char tncportname[40];
char rigconf[80];
int in_packetclient;
int tnc_serial_rate = 2400;
char clusterlogin[80] = "";

/*-------------------------------------rigctl-------------------------------*/
#ifdef HAVE_LIBHAMLIB
rig_model_t myrig_model = 351;
RIG *my_rig;			/* handle to rig (instance) */
freq_t outfreq;			/* output  to rig */
rmode_t rmode;			/* radio mode of operation */
pbwidth_t width;
vfo_t vfo;			/* vfo selection */
port_t myport;
#else
int outfreq;			/* output  to rig */
#endif
int ssb_bandwidth = 3000;
int cw_bandwidth = 0;
int serial_rate = 2400;
char rigportname[40];
int rignumber = 0;
int rig_comm_error = 0;
int rig_comm_success = 0;

/*---------------------------------simulator-------------------------------*/
int simulator = 0;
int simulator_mode = 0;
int simulator_seed = 8327;
int system_secs;
char tonecpy[5];
char simulator_tone[5];

/*-------------------------------the log lines-----------------------------*/
char qsos[MAX_QSOS][LOGLINELEN+1];
int nr_qsos = 0;

/*------------------------------dupe array---------------------------------*/
int nr_worked = 0;		/*< number of calls in worked[] */
struct worked_t worked[MAX_CALLS]; /*< worked stations */

/*----------------------statistic of worked countries,zones ... -----------*/
int countries[MAX_DATALINES];	/* per country bit fieldwith worked bands set */
int zones[MAX_ZONES];		/* same for cq zones or itu zones;
				   using 1 - 40 or 1 - 90 */
char mults[MAX_MULTS][12];
int mult_bands[MAX_MULTS];
int multarray_nr = 0;

GPtrArray *mults_possible;

int multlist = 0;

int callareas[20];
int multscore[NBANDS];

struct ie_list *main_ie_list;	/* head of initial exchange list */

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
struct tm *time_ptr;

float freq;
float mem;
int logfrequency = 0;
int rit;
int trx_control = 0;
int showfreq = 0;
float bandfrequency[9] =
    { 1830.0, 3525.0, 7010.0, 10105.0, 14025.0, 18070.0, 21025.0, 24900.0,
28025.0 };

char headerline[81] =
    "   1=CQ  2=DE  3=RST 4=73  5=HIS  6=MY  7=B4   8=AGN  9=?  \n";
char backgrnd_str[81] =
    "                                                                                ";

char logline_edit[5][LOGLINELEN+1];

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

double r = 50;
int m = 1;
char hiscountry[40];

int this_second;
int stop_backgrnd_process = 1;	/* dont start until we know what we are doing */

int wazmult = 0;		/* to add the ability of WAZ zones to be multiplier */
int itumult = 0;		/* to add the ability of ITU zones to be multiplier */
char itustr[3];

int nopacket = 0;		/* set if tlf is called with '-n' */
int no_trx_control = 0;		/* set if tlf is called with '-r' */

int bandweight_points[NBANDS] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
int bandweight_multis[NBANDS] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

pthread_t background_thread;
static struct termios oldt, newt;

/** parse program options
 */
void parse_options(int argc, char *argv[])
{
    while ((argc > 1) && (argv[1][0] == '-')) {
	switch (argv[1][1]) {
	    /* verbose option */
	case 'f':
	    if (strlen(argv[1] + 2) > 0) {
		if ((*(argv[1] + 2) == '~') && (*(argv[1] + 3) == '/')) {
		    /* tilde expansion */
		    config_file = g_strconcat( g_get_home_dir(),
			    argv[1] + 3, NULL);
		}
	    	else {
		    config_file = g_strdup(argv[1] + 2);
		}
	    }
	    break;
	case 's':
	    if (strlen(argv[1] + 2) > 0)
		strcpy(synclogfile, argv[1] + 2);
	    break;
	case 'd':		// debug rigctl
	    debugflag = 1;
	    break;
	case 'v':		// verbose startup
	    verbose = 1;
	    break;
	case 'V':		// output version
	    printf("Version: tlf-%s\n", VERSION);
	    exit(0);
	    break;
	case 'n':		// disable packet
	    nopacket = 1;
	    break;
	case 'r':
	    no_trx_control = 1; // disable radio control
	    break;
	default:
	    printf("Use: tlf [-v] Verbose\n");
	    printf("         [-V] Version\n");
	    printf("         [-f] Configuration file\n");
	    printf("         [-d] Debug mode\n");
	    printf("         [-h] This message\n");
	    printf("         [-n] Start without cluster hookup\n");
	    printf("         [-r] Start without radio control\n");
	    exit(0);
	    break;
	}
	--argc;
	++argv;
    }
}


/** initialize user interface */
void ui_init()
{
    /* modify stdin terminals attributes to allow Ctrl-Q/S key recognition */
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_iflag &= ~(IXON);
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

/* getting users terminal string and (if RXVT) setting rxvt colours on it */
/* LZ3NY hack :) */
    char *term = getenv("TERM");
    if (strcasecmp(term, "rxvt") == 0) {
	use_rxvt = 1;
    } else if (strcasecmp(term, "xterm") == 0) {
	use_xterm = 1;
	use_rxvt = 1;
    } else
	putenv("TERM=rxvt");	/*or going to native console linux driver */

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

	showmsg( "!! TLF needs at least 25 lines and 80 columns !!");
	showmsg( "   Continue anyway? Y/(N)" );
	c = toupper( key_get() );
	if (c != 'Y') {
	    showmsg( "73 es cuagn" );
	    sleep(1);
	    endwin();
	    exit(EXIT_FAILURE);
	}
	showmsg("");
    }

    if (!has_colors() || (start_color() == ERR)) {
	showmsg("Sorry, terminal does not support color");
	showmsg("Try TERM=linux  or use a text console !!");
	sleep(2);
	endwin();
	exit(EXIT_FAILURE);
    }


    refreshp();

    noecho();
    crmode();

//keypad(stdscr,TRUE);

}


/* setup colors */
void ui_color_init()
{
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
int databases_load()
{
    int status;

    showmsg("reading country data");
    readctydata();		/* read ctydb.dat */

    showmsg("reading configuration data");
    status = read_logcfg(); 	/* read the configuration file */
    status |= read_rules();	/* read the additional contest rules
				   in "rules/contestname" */
    if (status != PARSE_OK) {
	showmsg( "Problems in logcfg.dat or rule file detected! Continue Y/(N)?");
	if (toupper( key_get() ) != 'Y') {
	    showmsg("73...");
	    return EXIT_FAILURE;
	}
    }

    if (*call == '\0') {
	showmsg
	    ("WARNING: No callsign defined in logcfg.dat! exiting...\n\n\n");
	return EXIT_FAILURE;
    }

    mults_possible = g_ptr_array_new();

    if (multlist == 1) {
	showmsg("reading multiplier data      ");
	load_multipliers();
    }

    showmsg("reading callmaster data");
    load_callmaster();

    if (*exchange_list != '\0') {
	showmsg("reading initial exchange file");
	main_ie_list = make_ie_list(exchange_list);

	if (main_ie_list == NULL) {
	    showmsg( "Problems in initial exchange file detected! Continue Y/(N)?");
	    if (toupper( key_get() ) != 'Y') {
		showmsg("73...");
		return EXIT_FAILURE;
	    }
	}
    }

    /* make sure logfile is there and has the right format */
    if (checklogfile_new(logfile) != 0) {
	showmsg( "Can not access logfile. Giving up" );
	return EXIT_FAILURE;
    }

    if (qtcdirection > 0) {
	if (checkqtclogfile() != 0) {
	    showmsg( "QTC's giving up" );
	    return EXIT_FAILURE;
	}
	readqtccalls();
    }
    return 0;
}

void hamlib_init()
{
#ifdef HAVE_LIBHAMLIB		// Code for hamlib interface
    int status;

    showmsg("HAMLIB compiled in");

    if (no_trx_control == 1) {
	trx_control = 0;
    }

    if (trx_control != 0) {

	shownr("Rignumber is", (int) myrig_model);
	shownr("Rig speed is", serial_rate);

	showmsg("Trying to start rig ctrl");

	status = init_tlf_rig();

	if (status  != 0) {
	    showmsg( "Continue without rig control Y/(N)?");
	    if (toupper( key_get() ) != 'Y') {
		endwin();
		exit(1);
	    }
	    trx_control = 0;
	    showmsg( "Disabling rig control!");
	    sleep(1);
	}
    }
#else
    showmsg("No Hamlib compiled in!");

    trx_control = 0;
    showmsg( "Disabling rig control!");
    sleep(1);
#endif				/* HAVE_LIBHAMLIB */
}


void lan_init()
{
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


void packet_init()
{
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


void keyer_init()
{
    char keyerbuff[3];

    if (keyerport == NET_KEYER) {
	showmsg("Keyer is cwdaemon");

	if (netkeyer_init() < 0) {
	    showmsg("Cannot open NET keyer daemon ");
	    refreshp();
	    sleep(1);

	} else {
	    netkeyer(K_RESET, "0");

	    sprintf(weightbuf, "%d", weight);

	    write_tone();

	    snprintf(keyerbuff, 3, "%2d", GetCWSpeed());
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

    if (keyerport == MFJ1278_KEYER || keyerport == GMFSK) {
	init_controller();
    }

}



/** cleanup function
 *
 * Cleanup initialisations made by tlf. Will be called after exit() from
 * logit() or background_process()
 */
void tlf_cleanup()
{
    if (pthread_self() != background_thread) {
	pthread_cancel(background_thread);
	pthread_join(background_thread, NULL);
    }

//    commented out for the moment as it will segfault if called twice
//    cleanup_telnet();

    if (trxmode == CWMODE && keyerport == NET_KEYER)
	netkeyer_close();
    else
	deinit_controller();

    endwin();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}


/* ------------------------------------------------------------------------*/
/*     Main loop of the program			                           */
/* ------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int j;
    int ret;
    char tlfversion[80] = "";

    parse_options(argc, argv);

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

    strcpy(sp_return, message[SP_TU_MSG]);
    strcpy(cq_return, message[CQ_TU_MSG]);


    sprintf(tlfversion,
	    "        Welcome to tlf-%s by PA0R!!" , VERSION);
    showmsg(tlfversion);
    showmsg("");

    total = 0;
    if (databases_load() == EXIT_FAILURE) {
	sleep(2);
	endwin();
	exit(EXIT_FAILURE);
    }

    /* now setup colors */
    ui_color_init();

//              if (strlen(synclogfile) > 0)
//                      synclog(synclogfile);

    xmlrpc_showinfo();
    hamlib_init();
    lan_init();
    keyer_init();

    clear();
    mvprintw(0, 0, "        Welcome to tlf-%s by PA0R!!\n\n" , VERSION);
    refreshp();

    checkparameters();		/* check .paras file */
    getmessages();		/* read .paras file */

    packet_init();
    getwwv();			/* get the latest wwv info from packet */

    scroll_log();		/* read the last 5  log lines and set the qso number */

    nr_qsos = readcalls();	/* read the logfile for score and dupe */

    clear_display();		/* tidy up the display */
    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    for (j = 13; j <= 23; j++) {	/* wipe lower window */
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
    logit(NULL);

    return 0;
}
