/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0r@eudxf.org>
 *                    2010-2014 Thomas Beierlein <tb@forth-ev.de>
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../src/globalvars.h"
#include "../src/setcontest.h"
#include "../src/tlf.h"
#include "../src/tlf_curses.h"

#include "test.h"


char lastqsonr[5];

int prsock = 0;
char pr_hostaddress[48] = "111.222.111.222";
char *config_file = NULL;
int portnum = 0;

bool use_rxvt = false;
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


bool debugflag = false;
char *editor_cmd = NULL;
char rttyoutput[120];
int tune_val = 0;
int use_bandoutput = 0;
bool no_arrows = false;
int bandindexarray[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
bool cqwwm2 = false;

int cwkeyer = NO_KEYER;
int digikeyer = NO_KEYER;

char whichcontest[40] = "qso";
bool iscontest = false;		/* false =  General,  true  = contest */
contest_config_t *contest;	/* contest configuration */


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
int recall_mult = 0;
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
int isdupe = 0;			// 0 if nodupe -- for auto qso b4 (LZ3NY)
bool nob4 = false;			// allow auto b4
bool ignoredupe = false;
bool noautocq = false;
bool verbose = false;
bool no_rst = false;		/* do not use RS/RST */
bool sprint_mode = false;

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
int multi = 0;			/* 0 = SO , 1 = MOST, 2 = MM */
int trxmode = CWMODE;
/* RIG_MODE_NONE in hamlib/rig.h, but if hamlib not compiled, then no dependecy */
rmode_t rigmode = 0;
rmode_t digi_mode = 0;
bool mixedmode = false;
char sent_rst[4] = "599";
char recvd_rst[4] = "599";
char last_rst[4] = "599";       /* Report for last QSO */
int mults_per_band = 1;		/* mults count per band */
int shortqsonr = LONGCW;	/* 1  =  short  cw char in exchange */
int cluster = NOCLUSTER;	/* 0 = OFF, 1 = FOLLOW, 2  = spots  3 = all */
bool clusterlog = false;		/* clusterlog on/off */
bool searchflg = false;		/* display search  window */
bool show_time = false;
cqmode_t cqmode = CQ;
bool demode = false;		/* send DE  before s&p call  */
int announcefilter = FILTER_ANN; /*  filter cluster  announcements */
bool showscore_flag = false;	/* show  score window */
int change_rst = 0;
char exchange[40];
int defer_store = 0;
mystation_t my;
char logfile[120] = "general.log";
char *cabrillo = NULL;		/*< Name of the cabrillo format definition */
char synclogfile[120];
char markerfile[120] = "";
int xplanet = MARKER_NONE;
char fldigi_url[50] = "http://localhost:7362/RPC2";

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
{
    "TEST %", "@ DE %", "@ [", "TU 73", "@", "%",
    "@ SRI QSO B4 GL", "AGN",
    "?", "QRZ?", "PSE K", "TEST % %", "@ [", "TU %",
    "", "", "", "", "", "", "", "", "", "", ""
};

char fkey_header[60] =
    "   1=CQ  2=DE  3=RST 4=73  5=HIS  6=MY  7=B4   8=AGN  9=?  ";

char *digi_message[sizeof(message) / sizeof(message[0])];

char ph_message[14][80] = /**< Array of file names for voice keyer messages
			   * See description of message[]
			   */
    { "", "", "", "", "", "", "", "", "", "", "", "", "", "" };

char qtc_recv_msgs[12][80] = {
    "QTC?", "QRV", "R", "", "TIME?", "CALL?",
    "NR?", "AGN", "", "QSL ALL", "", ""}; // QTC receive windows Fx messages

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

int qtcdirection = 0;

int minitest = 0;

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
char qsonrstr[5] = "0001";
char band[NBANDS][4] =
{ "160", " 80", " 60", " 40", " 30", " 20", " 17", " 15", " 12", " 10", "???" };
char comment[80];
char normalized_comment[80];
char proposed_exchange[80];
char mode[20] = "Log     ";
char cqzone[3] = "";
char ituzone[3] = "";
char continent[3] = "";
char wpx_prefix[11] = "";
int bandindex = 0;
int totalmults = 0;
int totalcountries = 0;
int totalzones = 0;
int secs = 0;
int countrynr;
int total = 0; 		/**< total number of qso points */
int qso_points;
int qsos_per_band[NBANDS];
int dupe = 0;
bool partials = false;	/**< show partial calls */
bool use_part = false;	/**< if 1 use automatically found partial call */
int block_part = 0; 	/**< if 1 block the call autocompletion
			  for these QSO */
char para_word[80] =
    "LODNCFS:3C\n";	/* longcw, cluster, search,  DE, contest, filter,  speed,  delay */
char lastmsg[1000] = "";
int scale_values[20] = {
    40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6,
    4, 2
};
char sc_device[40] = "/dev/dsp";

/*-------------------------------------keyer------------------------------*/
int keyerport = NO_KEYER;
int txdelay = 0;
int tune_seconds = 16;
int weight = 0;
char weightbuf[4];
char tonestr[5] = "600";
int cqdelay = 8;
char wkeyerbuffer[400];
int data_ready = 0;
char keyer_device[10] = "";	// ttyS0, ttyS1, lp0-2
bool keyer_backspace = false;
int k_pin14;
int k_ptt;
char controllerport[80] = "/dev/ttyS0";
int miniterm = 0;		/* is miniterm for digimode active? */
char modem_mode[8];
int commentfield = 0;		/* 1 if we are in comment/excahnge input */

/*-------------------------------------packet-------------------------------*/
char spot_ptr[MAX_SPOTS][82];		/* Array of cluster spot lines */
int nr_of_spots;			/* Anzahl Lines in spot_ptr array */
int packetinterface = 0;
int fdSertnc = 0;
int fdFIFO = 0;
int tncport = 1;
char tncportname[40];
char rigconf[80];
int in_packetclient;
int tnc_serial_rate = 2400;
char clusterlogin[80] = "";
bool bmautoadd = false;
bool bmautograb = false;

/*-------------------------------------rigctl-------------------------------*/
#ifdef HAVE_LIBHAMLIB
int myrig_model = 351;
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
char *rigportname;
int rignumber = 0;
int rig_comm_error = 0;
int rig_comm_success = 0;
int rigptt = 0;

/*-------------------------------the log lines-----------------------------*/
char qsos[MAX_QSOS][LOGLINELEN + 1];
int nr_qsos = 0;

/*------------------------------dupe array---------------------------------*/
int nr_worked = 0;		/*< number of calls in worked[] */
worked_t worked[MAX_CALLS]; 	/*< worked stations */

/*----------------------statistic of worked countries,zones ... -----------*/
int countries[MAX_DATALINES];	/* per country bit fieldwith worked bands set */
int zones[MAX_ZONES];		/* same for cq zones or itu zones;
				   using 1 - 40 or 1 - 90 */
char mults[MAX_MULTS][12];
int mult_bands[MAX_MULTS];
int multarray_nr = 0;

int multlist = 0;

int callareas[20];
int multscore[NBANDS];

struct ie_list *main_ie_list;	/* head of initial exchange list */

int zonescore[NBANDS];
int countryscore[NBANDS];
int zonedisplay = 0;
int new_zone = 0;		/* index of for new zone */
int new_cty = 0;		/* index of new country */
int new_mult = -1;
int minute_timer = 0;

int bandinx = BANDINDEX_40;	/* start with 40m */
int qsonum = 1;			/* nr of next QSO */
int ymax, xmax;			/* screen size */

pid_t pid;
struct tm *time_ptr;

freq_t freq;
freq_t mem;
bool logfrequency = false;
bool rit;
bool trx_control = false;
int showfreq = 0;
freq_t bandfrequency[NBANDS] = {
    1830000.0, 3525000.0, 5352000.0, 7010000.0, 10105000.0, 14025000.0, 18070000.0, 21025000.0, 24900000.0,
    28025000.0, 0.0
};

char headerline[81] =
    "   1=CQ  2=DE  3=RST 4=73  5=HIS  6=MY  7=B4   8=AGN  9=?  \n";
const char *backgrnd_str =
    "                                                                                ";

char logline_edit[5][LOGLINELEN + 1];

char termbuf[88] = "";
int termbufcount = 0;

double DEST_Lat = 51.;
double DEST_Long = 1.;

char hiscountry[40];

int this_second;
int stop_backgrnd_process = 1;	/* dont start until we know what we are doing */

bool wazmult = false;		/* to add the ability of WAZ zones to be multiplier */
bool itumult = false;		/* to add the ability of ITU zones to be multiplier */
char itustr[3];

bool nopacket = false;		/* set if tlf is called with '-n' */

int bandweight_points[NBANDS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int bandweight_multis[NBANDS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

mults_t multis[MAX_MULTS]; 	/**< worked multis */
int nr_multis = 0;      	/**< number of multis in multis[] */

int unique_call_multi = 0;          /* do we count calls as multiplier */

//////////////////
char lan_logline[256];	    // defined in log_to_disk.c

//////////////////

#include <curses.h>
NCURSES_EXPORT_VAR(WINDOW *) stdscr = NULL;
int wattr_on(WINDOW *win, attr_t attrs, void *opts) {
    return 0;
}

void displayit() {
}

int netkeyer(int cw_op, char *cwmessage) {
    return 0;
}

char mvprintw_history[NLAST][LINESZ];

void clear_mvprintw_history() {
    for (int i = 0; i < NLAST; ++i) {
	mvprintw_history[i][0] = 0;
    }
}

void add_mvprintw_history(int y, int x, const char *fmt, va_list args) {
    // shift history
    for (int i = NLAST - 1; i >= 1; --i) {
	strcpy(mvprintw_history[i], mvprintw_history[i - 1]);
    }
    // add new record
    sprintf(mvprintw_history[0], "%02d|%02d|", y, x);
    vsnprintf(mvprintw_history[0] + 6, 100 - 6, fmt, args);
}

int mvprintw(int y, int x, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    add_mvprintw_history(y, x, fmt, args);
    va_end(args);
    return OK;
}

int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    add_mvprintw_history(y, x, fmt, args);
    va_end(args);
    return OK;
}

// mvaddstr is defined as a macro composed of wmove+waddnstr

static int last_y, last_x;

int wmove(WINDOW *win, int y, int x) {
    last_y = y;
    last_x = x;
    return OK;
}

int waddnstr(WINDOW *win, const char *str, int n) {
    return mvprintw(last_y, last_x, "%s", str);
}

