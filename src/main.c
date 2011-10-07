/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0r@eudxf.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#define NDEBUG
#define NEWCODE = 1

#include "tlf.h"
#include "globalvars.h"
#include "main.h"
#include <glib.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

SCREEN *mainscreen;
SCREEN *packetscreen;
WINDOW *sclwin, *entwin;

extern int lan_active;

int prsock = 0;
char pr_hostaddress[48] = "131.155.192.179";
char config_file[80] = "";
int portnum = 0;
struct tln_logline *loghead = NULL;
struct tln_logline *logtail = NULL;
struct tln_logline *viewing = NULL;
struct tln_logline *temp = NULL;

int use_rxvt = 0;
int use_xterm = 0;

int tlfcolors[8][2] = { {COLOR_BLACK, COLOR_WHITE},
{COLOR_GREEN, COLOR_YELLOW},
{COLOR_WHITE, COLOR_RED},
{COLOR_CYAN, COLOR_WHITE},
{COLOR_WHITE, COLOR_BLACK},
{COLOR_MAGENTA, COLOR_WHITE},
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
int cqww = 0;
int cqwwm2 = 0;
int wpx = 0;
int dxped = 0;
int sprint = 0;
int arrldx_usa = 0;
int arrl_fd = 0;
int arrlss = 0;
int pacc_pa_flg = 0;
int universal = 0;
int addcallarea;
int pfxmult = 0;
int exc_cont = 0;
int manise80;
int other_flg;
int one_point = 0;
int two_point = 0;
int three_point = 0;
int two_eu_three_dx_points = 0;
int ssbpoints;
int cwpoints;
int lowband_point_mult = 0;
int sc_sidetone;
char sc_volume[3] = "";
  /* LZ3NY mods */
char contest_name[50];
int multiplier_points = -1;
int my_country_points = -1;
int my_cont_points = -1;
int dx_cont_points = -1;
char mit_multiplier_list[255][6];
int multiplier_only = 0;
int mult_side = 0;
char *mit_mult_array;
int in_country_list;
/* end LZ3NY mods */

int portable_x2 = 0;
int recall_mult = 0;
int exchange_serial = 0;
int wysiwyg_once = 0;
int wysiwyg_multi = 0;
int country_mult = 0;
int fixedmult = 0;
int sectn_mult = 0;
int dx_arrlsections = 0;
int serial_section_mult = 0;
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

char tlfversion[80] = "";
char testbuffer[120] = "";
char multsfile[80] = "";	/* name of file with a list of allowed 
				   multipliers */
char exchange_list[40] = "";
int timeoffset = 0;
int multi = 0;			/* 0 = SO , 1 = MOST, 2 = MM */
int trxmode = CWMODE;
int mixedmode = 0;
char his_rst[4] = "599";
char my_rst[4] = "599";
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
char sp_return[80] = " \n";
char cq_return[80] = " \n";
char whichcontest[40] = "qso";
int defer_store = 0;
char buffer[162];
char call[20];
char logfile[120] = "general.log";
char synclogfile[120];
char markerfile[120] = "";
int xplanet = 0;
char message[25][80] =
    { "TEST %\n", "@ DE %\n", "@ [\n", "TU 73\n", " @\n", "%\n",
"@ SRI QSO B4 GL\n", "AGN\n",
    " ?\n", " QRZ?\n", " PSE K\n", "TEST % %\n", "@ [\n", "TU %\n", "", "",
	"", "", "", "", "", "", "", "", ""
};
char ph_message[14][80] = { "", "", "", "", "", "", "", "", "", "", "", "" };	// voice keyer file names

char hiscall[20];			/**< call of other station */
char hiscall_sent[20] = "";		/**< part which was sent during early
					  start */
int cwstart = 0;			/**< number characters after which
					   sending call started automatically,
					   0 - off */
int sending_call = 0;
int early_started = 0;			/**< 1 if sending call started early,
					   strlen(hiscall)>cwstart or 'space' */
char lastcall[20];
char lastcomment[40];
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
int points = 0;
int total = 0;
int band_score[9];
int dupe = 0;
int callfound = 0;
int partials = 0;
int use_part = 0;
int block_part = 0;
char para_word[80] = "LODNCFS:3C\n";	/* longcw, cluster, search,  DE, contest, filter,  speed,  delay */
char lastmsg[1000] = "";
int scale_values[20] =
    { 40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6,
4, 2 };
char sc_device[40] = "/dev/dsp";

/*-------------------------------------keyer------------------------------*/
int keyerport = NO_KEYER;
int speed = 10;
int txdelay = 0;
int weight = 0;
char weightbuf[4];
char speedstr[50] = CW_SPEEDS;
char tonestr[5] = "600";
int cqdelay = 8;
char wkeyerbuffer[400];
int keyspeed = 5;
int cfd;			/* cwkeyer file descriptor */
int data_ready = 0;
char keyer_device[10] = "";	// ttyS0, ttyS1, lp0-2
int k_tune;
int k_pin14;
int k_ptt;
char controllerport[80] = "/dev/ttyS0";
int miniterm = 0;
char modem_mode[8];
int commentfield = 0;		/* 1 if we are in comment/excahnge input */

/*-------------------------------------packet-------------------------------*/
char spot_ptr[MAX_SPOTS][82];		/* Array of cluster spot lines */
int spotarray[MAX_SPOTS];		/* Array of indices into spot_ptr */
char spotinfo[MAX_SPOTS][82];
int ptr;				/* Anzahl Lines in ispot_ptr array */
long int *wwv_ptr;
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
freq_t rigfreq;			/* input frequency  */
freq_t outfreq;			/* output  to rig */
rmode_t rmode;			/* radio mode of operation */
pbwidth_t width;
vfo_t vfo;			/* vfo selection */
port_t myport;
#else
float rigfreq;			/* input frequency  */
int outfreq;			/* output  to rig */
#endif
int ssb_bandwidth = 3000;
int cw_bandwidth = 0;
int nobandchange = 0;
int serial_rate = 2400;
int rig_port = 0;
char rigportname[40];
int native_rig_fd = 0;
int rignumber = 0;
int rig_comm_error = 0;
int rig_comm_success = 0;

/*---------------------------------simulator-------------------------------*/
int simulator = 0;
int simulator_mode = 0;
int simulator_seed = 8327;
long system_secs;
char tonecpy[5];
char simulator_tone[5];

/*-------------------------------the log lines-----------------------------*/
char qsos[MAX_QSOS][82];
int nr_qsos = 0;

/*------------------------------dupe array---------------------------------*/
int callarray_nr = 0;		/* number of calls in callarray */
char callarray[MAX_CALLS][20];	/* internal log representation for dupes  */
char call_exchange[MAX_CALLS][12];
int call_band[MAX_CALLS];
int call_country[MAX_CALLS];

/*----------------------statisticof worked countries,zones ... -----------*/
int countries[MAX_DATALINES];	/* per country bit fieldwith worked bands set */
int zones[41];			/* same for cqzones; using 1 - 40 */
char mults[MAX_MULTS][12];
int mult_bands[MAX_MULTS];
int multarray_nr = 0;

GPtrArray *mults_possible;

int multlist = 0;

char callmasterarray[MAX_CALLMASTER][14];
long int nr_callmastercalls;

char callmaster_result[50][9];
int callareas[20];
int multscore[NBANDS];

struct ie_list *main_ie_list;	/* head of initial exchange list */

int zonescore[6];
int countryscore[6];
int zonedisplay = 0;
int addzone = 0;		/* flag for new zone */
int addcty = 0;			/* flag for new country */
int shownewmult = -1;
int minute_timer = 0;

int bandinx = BANDINDEX_40;	/* start with 40m */
int qsonum = 1;
int bufloc = 0;
int ymax, xmax;			/* screen size */
char lastwwv[120] = "";
int bandmap_pos = 0;
int nroflines;

pid_t pid;
struct tm *time_ptr;

char qrg_string[8];
float freq;
float mem;
int logfrequency = 0;
int rit;
int trx_control = 0;
int showfreq = 0;
float bandfrequency[9] =
    { 1830.0, 3525.0, 7010.0, 10105.0, 14025.0, 18070.0, 21025.0, 24900.0,
28025.0 };
char spot_target[8][40];

char headerline[81] =
    "   1=CQ  2=DE  3=RST 4=73  5=HIS  6=MY  7=B4   8=AGN  9=?  \n";
char infoline[81] = "";
char backgrnd_str[81] =
    "                                                                                ";

char terminal1[88] = "";
char terminal2[88] = "";
char terminal3[88] = "";
char terminal4[88] = "";
char termbuf[88] = "";
int termbufcount = 0;

char C_QTH_Lat[7] = "51";
char C_QTH_Long[8] = "-5";
char C_DEST_Lat[7] = "51";
char C_DEST_Long[8] = "1";

double yt = -4.9;		/* for muf calculation */
double xt = 52.4;
double yr = 5.0;
double xr = 50.0;
double r = 50;
int m = 1;
char hiscountry[40];

double range, bearing;
double sunrise;
double sundown;

int this_second;
int stop_backgrnd_process = 1;	/* dont start until we know what we are doing */

int wazmult = 0;		/* to add the ability of WAZ zones to be multiplier */
int itumult = 0;		/* to add the ability of ITU zones to be multiplier */
char itustr[3];

/* ------------------------------------------------------------------------*/
/*     Main loop of the program			                           */
/* ------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int j;
    pthread_t thrd1, thrd2;
    int ret;
    int retval;
    char keyerbuff[3];
    int nopacket = 0;

    while ((argc > 1) && (argv[1][0] == '-')) {
	switch (argv[1][1]) {
	    /* verbose option */
	case 'f':
	    if (strlen(argv[1] + 2) > 0)
		strcpy(config_file, argv[1] + 2);
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
	case 'n':		// output version
	    nopacket = 1;
	    break;
	default:
	    printf("Use: tlf [-v] Verbose\n");
	    printf("         [-V] Version\n");
	    printf("         [-f] Configuration file\n");
	    printf("         [-d] Debug mode\n");
	    printf("         [-h] This message\n");
	    printf("         [-n] Start without cluster hookup\n");
	    exit(0);
	    break;
	}
	--argc;
	++argv;
    }

    buffer[0] = '\0';
    buffer[79] = '\0';
    bufloc = 0;

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

/* getting users terminal string and (if RXVT) setting rxvt colours on it */
/* LZ3NY hack :) */
    if (strcasecmp(getenv("TERM"), "rxvt") == 0) {
	use_rxvt = 1;
	printf("terminal is:%s", getenv("TERM"));
    } else if (strcasecmp(getenv("TERM"), "xterm") == 0) {
	use_xterm = 1;
	use_rxvt = 1;
    } else
	putenv("TERM=rxvt");	/*or going to native console linux driver */

    if ((mainscreen = newterm(NULL, stdout, stdin)) == NULL) {	/* activate ncurses terminal control */
	perror("initscr");
	printf
	    ("\n Sorry, wrong terminal type !!!!! \nTry a  linux text terminal or set TERM=linux !!!");
	sleep(5);

	exit(EXIT_FAILURE);
    }
//keypad(stdscr,TRUE);

    getmaxyx(stdscr, ymax, xmax);
    noecho();
    crmode();

    strcpy(sp_return, message[12]);
    strcpy(cq_return, message[13]);

    refresh();

    if (has_colors()) {
	if (start_color() == ERR) {
	    perror("start_color");
	    endwin();
	    printf
		("\n Sorry, wrong terminal type !!!!! \n\nTry a linux text terminal or set TERM=linux !!!");
	    sleep(5);
	    exit(EXIT_FAILURE);
	}
	// use linux console colours
	init_pair(COLOR_BLACK, tlfcolors[0][0], tlfcolors[0][1]);
	init_pair(COLOR_GREEN, tlfcolors[1][0], tlfcolors[1][1]);
	init_pair(COLOR_RED, tlfcolors[2][0], tlfcolors[2][1]);
	init_pair(COLOR_CYAN, tlfcolors[3][0], tlfcolors[3][1]);
	init_pair(COLOR_WHITE, tlfcolors[4][0], tlfcolors[4][1]);
	init_pair(COLOR_MAGENTA, tlfcolors[5][0], tlfcolors[5][1]);
	init_pair(COLOR_BLUE, tlfcolors[6][0], tlfcolors[6][1]);
	init_pair(COLOR_YELLOW, tlfcolors[7][0], tlfcolors[7][1]);

	strcpy(tlfversion, "        Welcome to tlf-");
	strcat(tlfversion, VERSION);
	strcat(tlfversion, " by PA0R!!");
	showmsg(tlfversion);
	showmsg("");

	showmsg("reading country data");
	readctydata();		/* read ctydb.dat */

	showmsg("reading configuration data");

	read_logcfg();		/* read the configuration file */
	read_rules();		/* read the additional contest rules in "rules/contestname"  LZ3NY */

	checklogfile();		/* make sure logfile is there */

//              if (strlen(synclogfile) > 0)
//                      synclog(synclogfile);

	if (*call == '\0') {
	    showmsg
		("WARNING: No callsign defined in logcfg.dat! exiting...\n\n\n");
	    exit(1);
	}

	if (use_rxvt == 1) {	// use rxvt colours
	    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_RED);
	    if (use_xterm == 1) {
		init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLUE);
		init_pair(COLOR_RED, COLOR_WHITE, 8);
		init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_MAGENTA);
	    } else {
		init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_YELLOW);
		init_pair(COLOR_RED, COLOR_WHITE, COLOR_RED);
		init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_RED);
	    }
	    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	    if (use_xterm == 1) {
		init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_WHITE);
		init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_WHITE);
	    } else {
		init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_RED);
		init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_YELLOW);
	    }
	    init_pair(COLOR_YELLOW, COLOR_CYAN, COLOR_YELLOW);
	} else {
	    // use linux console colours redefined....
	    init_pair(COLOR_BLACK, tlfcolors[0][0], tlfcolors[0][1]);
	    init_pair(COLOR_GREEN, tlfcolors[1][0], tlfcolors[1][1]);
	    init_pair(COLOR_RED, tlfcolors[2][0], tlfcolors[2][1]);
	    init_pair(COLOR_CYAN, tlfcolors[3][0], tlfcolors[3][1]);
	    init_pair(COLOR_WHITE, tlfcolors[4][0], tlfcolors[4][1]);
	    init_pair(COLOR_MAGENTA, tlfcolors[5][0], tlfcolors[5][1]);
	    init_pair(COLOR_BLUE, tlfcolors[6][0], tlfcolors[6][1]);
	    init_pair(COLOR_YELLOW, tlfcolors[7][0], tlfcolors[7][1]);

	}

	mults_possible = g_ptr_array_new();

	if (multlist == 1) {
	    showmsg("reading multiplier data      ");
	    load_multipliers();

	}

	attron(COLOR_PAIR(COLOR_BLACK));
	showmsg("reading callmaster data");

	nr_callmastercalls = load_callmaster();

	main_ie_list = make_ie_list();	// get initial exchange file
	if (main_ie_list == NULL)
	    showmsg("No initial exchange available");
//else
//      test_ie_list(main_ie_list);

#ifdef HAVE_LIBHAMLIB		// Code for hamlib interface

	showmsg("HAMLIB defined");

	if (trx_control != 0) {

	    shownr("Rignumber is", (int) myrig_model);
	    shownr("Rig speed is", serial_rate);

	    showmsg("Trying to start rig ctrl");

	    /** \todo fix exclusion of newer hamlib models */
	    if ((int) myrig_model > 1999)
		init_native_rig();
	    else
		init_tlf_rig();
	}
#else
	if (trx_control != 0) {
//                      trx_control = 0;
	    showmsg("No Hamlib library, using native driver");
	    shownr("Rignumber is", rignumber);
	    shownr("Rig speed is", serial_rate);
	    init_native_rig();
	    sleep(1);
	}
#endif				// end code for hamlib interface

	if (keyerport == NET_KEYER) {
	    showmsg("Keyer is cwdaemon");
	    if (verbose == 1)
		sleep(1);
	}
	if (keyerport == MFJ1278_KEYER || keyerport == GMFSK) {
	    init_controller();
	}

	if (lan_active == 1) {
	    retval = lanrecv_init();

	    if (retval < 0)	/* set up the network */
		shownr("LAN receive  init failed", retval);
	    else
		showmsg("LAN receive  initialized");

	    if (lan_send_init() < 0)
		showmsg("LAN send init failed");
	    else
		showmsg("LAN send initialized");
	}
	if (verbose == 1)
	    sleep(1);

	checkparameters();	/* check .paras file */

	clear();
	strcpy(tlfversion, "        Welcome to tlf-");
	strcat(tlfversion, VERSION);
	strcat(tlfversion, " by PA0R!!\n\n");
	mvprintw(0, 0, tlfversion);
	refresh();
	getmessages();		/* read .paras file */

	if (nopacket == 1)
	    packetinterface = 0;

	set_term(mainscreen);

	refresh();

	if (packetinterface != 0) {

	    if (init_packet() == 0)
		packet();
	    else
		cleanup_telnet();

	}

	if (keyerport == NET_KEYER) {
	    if (netkeyer_init() < 0) {
		mvprintw(24, 0, "Cannot open NET keyer daemon ");
		refresh();
		sleep(1);

	    } else {
		netkeyer(K_RESET, "0");

		sprintf(weightbuf, "%d", weight);

		strncpy(keyerbuff, speedstr + (speed * 2), 2);
		keyerbuff[2] = '\0';

		write_tone();

		netkeyer(K_SPEED, keyerbuff);	// set speed

		netkeyer(K_WEIGHT, weightbuf);	// set weight

		if (*keyer_device != '\0')
		    netkeyer(K_DEVICE, keyer_device);	// set device

		sprintf(keyerbuff, "%d", txdelay);

		netkeyer(K_TOD, keyerbuff);	// set TOD

		if (sc_sidetone != 0)	// set soundcard output
		{
		    netkeyer(K_SIDETONE, "");

		    if (*sc_volume != '\0')	// set soundcard volume

			netkeyer(K_STVOLUME, sc_volume);
		}
	    }

	    if (keyerport != NET_KEYER)
		write_tone();
	}

	getwwv();		/* get the latest wwv info from packet */

	scroll_log();		/* read the last 5  log lines and set the qso number */

	nr_qsos = readcalls();	/* read the logfile for score and dupe */

	clear_display();	/* tidy up the display */

	qrb();

	attron(COLOR_PAIR(7) | A_STANDOUT);

	for (j = 13; j <= 23; j++) {	/* wipe lower window */
	    mvprintw(j, 0, backgrnd_str);
	}

	bm_init();			/* initialize bandmap */

	/* Create the first thread */
	ret = pthread_create(&thrd1, NULL, (void *) logit, NULL);
	if (ret) {
	    perror("pthread_create: logit");
	    endwin();
	    exit(EXIT_FAILURE);
	}

	/* Create the second thread */
	ret =
	    pthread_create(&thrd2, NULL, (void *) background_process,
			   NULL);
	if (ret) {
	    perror("pthread_create: backgound_process");
	    endwin();
	    exit(EXIT_FAILURE);
	}

	pthread_join(thrd2, NULL);
	pthread_join(thrd1, NULL);
	endwin();
	exit(EXIT_SUCCESS);

    } else {
	printf("Terminal does not support color\n");
	printf("\nTry TERM=linux  or use a text console !!\n");
	refresh();
	sleep(2);
    }
    cleanup_telnet();

    if (trxmode == CWMODE && keyerport == NET_KEYER)
	netkeyer_close();
    else
	close(cfd);		/* close keyer */

    endwin();

    return (0);
}
