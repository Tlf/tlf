/*
* Tlf - contest logging program for amateur radio operators
* Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0rct@amsat.org>
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
#include "parse_logcfg.h"
#include "speed_conversion.h"
#include "tlf.h"
#include "write_tone.h"
#include "speedup.h"
#include "speeddown.h"
#include <curses.h>
#include <string.h>
#include "startmsg.h"
#include <glib.h>
#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif
#include <ctype.h>
#include "bandmap.h"

extern int keyerport;
extern char tonestr[];
extern int speed, partials;
extern int use_part;
extern int contest;
extern int portnum;
extern int packetinterface;
extern int tncport;
extern int shortqsonr;
extern char * cabrillo;

int exist_in_country_list();

char inputbuffer[160];
FILE *fp;

void KeywordNotSupported(char *keyword);

#define  MAX_COMMANDS 158	/* commands in list */


int read_logcfg(void)
{
    extern int nodes;
    extern int node;
    extern char config_file[];

    char defltconf[80];

    contest = 0;
    speed = 14;
    partials = 0;
    use_part = 0;
    keyerport = 0;
    portnum = 0;
    packetinterface = 0;
    tncport = 0;
    nodes = 0;
    node = 0;
    shortqsonr = 0;
    cabrillo = NULL;

    strcpy(defltconf, PACKAGE_DATA_DIR);
    strcat(defltconf, "/logcfg.dat");

    if (strlen(config_file) == 0)
	strcpy(config_file, "logcfg.dat");

    if ((fp = fopen(config_file, "r")) == NULL) {
	if ((fp = fopen(defltconf, "r")) == NULL) {
	    showmsg("Error opening logcfg.dat file.");
	    showmsg("Exiting...");
	    sleep(5);
	    endwin();
	    exit(1);
	} else {
	    showstring("Using (Read Only) file", defltconf);
	}

    } else
	showstring("Opening config file", config_file);

    while ( fgets(inputbuffer, 120, fp) != NULL ) {

	if ((inputbuffer[0] != '#') && (strlen(inputbuffer) > 1)) {	
	    					/* skip comments and 
						 * empty lines */
	    parse_logcfg(inputbuffer);
	}
    }

    fclose(fp);

    return (0);
}

void parse_logcfg(char *inputbuffer)
{
    extern int use_rxvt;
    extern char message[15][80];
    extern char ph_message[14][80];
    extern char sp_return[];
    extern char cq_return[];
    extern char call[];
    extern char whichcontest[];
    extern char logfile[];
    extern int keyerport;
    extern int recall_mult;
    extern int one_point;
    extern int two_point;
    extern int three_point;
    extern int two_eu_three_dx_points;
    extern int exchange_serial;
    extern int country_mult;
    extern int wysiwyg_multi;
    extern int wysiwyg_once;
    extern int fixedmult;
    extern int portable_x2;
    extern int trx_control;
    extern int rit;
    extern int shortqsonr;
    extern int cluster;
    extern int clusterlog;
    extern int showscore_flag;
    extern int searchflg;
    extern int demode;
    extern int contest;
    extern int speed;
    extern int weight;
    extern int txdelay;
    extern char tonestr[];
    extern int showfreq;
    extern int editor;
    extern int partials;
    extern int use_part;
    extern int mixedmode;
    extern char pr_hostaddress[];
    extern int portnum;
    extern int packetinterface;
    extern int tncport;
    extern int tnc_serial_rate;
    extern char lastwwv[];
    extern int serial_rate;
#ifdef HAVE_LIBHAMLIB
    extern rig_model_t myrig_model;
#endif
    extern int rig_port;
    extern char rigportname[];
    extern int rignumber;
    extern char rigconf[];
    extern char exchange_list[40];
    extern char tncportname[];
    extern int netkeyer_port;
    extern char netkeyer_hostaddress[];
    extern char bc_hostaddress[MAXNODES][16];
    extern int lan_active;
    extern char thisnode;
    extern int nodes;
    extern int node;
    extern int cqwwm2;
    extern int landebug;
    extern int call_update;
    extern int timeoffset;
    extern int time_master;
    extern int ctcomp;
    extern char multsfile[];
    extern int multlist;
    extern int universal;
    extern int serial_section_mult;
    extern int serial_grid4_mult;
    extern int sectn_mult;
    extern int dx_arrlsections;
    extern int pfxmult;
    extern int exc_cont;
    extern int wpx;
    extern char markerfile[];
    extern int xplanet;
    extern int nob4;
    extern int noautocq;
    extern int show_time;
    extern char keyer_device[];
    extern int use_vk;
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

/* LZ3NY mods */
    extern int mult_side;
    extern int my_country_points;
    extern int my_cont_points;
    extern int dx_cont_points;
    extern int countrylist_points;
    extern int countrylist_only;
    char c_temp[11];
    extern int my_cont_points;
    extern int dx_cont_points;
    extern int mult_side;
    extern char mit_multiplier_list[][6];
    char *mit_mult_array;
/* end LZ3NY mods */
    extern int tlfcolors[8][2];
    extern char synclogfile[];
    extern int scale_values[];
    extern char sc_device[40];
    extern char controllerport[80];	// port for multi-mode controller
    extern char clusterlogin[];
    extern int cw_bandwidth;
    extern int change_rst;
    extern char rttyoutput[];
    extern int logfrequency;
    extern int ignoredupe;

    char commands[MAX_COMMANDS][30] = {
	"enable",		/* 0 */		/* deprecated */
	"disable",				/* deprecated */
	"F1=",
	"F2=",
	"F3=",
	"F4=",			/* 5 */
	"F5=",
	"F6=",
	"F7=",
	"F8=",
	"F9=",			/* 10 */
	"F10=",
	"F11=",
	"F12=",
	"S&P_TU_MSG=",
	"CQ_TU_MSG=",		/* 15 */
	"CALL=",
	"CONTEST=",
	"LOGFILE=",
	"KEYER_DEVICE=",
	"BANDOUTPUT",		/* 20 */
	"RECALL_MULTS",
	"ONE_POINT",
	"THREE_POINTS",
	"WYSIWYG_MULTIBAND",
	"WYSIWYG_ONCE",		/* 25 */
	"RADIO_CONTROL",
	"RIT_CLEAR",
	"SHORT_SERIAL",
	"LONG_SERIAL",
	"CONTEST_MODE",		/* 30 */
	"CLUSTER",
	"BANDMAP",
	"SPOTLIST",				/* deprecated */
	"SCOREWINDOW",
	"CHECKWINDOW",		/* 35 */
	"FILTER",				/* deprecated */
	"SEND_DE",
	"CWSPEED",
	"CWTONE",
	"WEIGHT",		/* 40 */
	"TXDELAY",
	"SUNSPOTS",
	"SFI",
	"SHOW_FREQUENCY",
	"EDITOR",		/* 45 */
	"PARTIALS",
	"USEPARTIALS",
	"POWERMULT_5",
	"POWERMULT_2",
	"POWERMULT_1",		/* 50 */
	"MANY_CALLS",				/* deprecated */
	"SERIAL_EXCHANGE",
	"COUNTRY_MULT",
	"2EU3DX_POINTS",
	"PORTABLE_MULT_2",	/* 55 */
	"MIXED",
	"TELNETHOST=",
	"TELNETPORT=",
	"TNCPORT=",
	"FIFO_INTERFACE",	/* 60 */
	"RIGMODEL=",
	"RIGSPEED=",
	"TNCSPEED=",
	"RIGPORT=",
	"NETKEYER",		/* 65 */
	"NETKEYERPORT=",
	"NETKEYERHOST=",
	"ADDNODE=",
	"THISNODE=",
	"CQWW_M2",		/* 70 */
	"LAN_DEBUG",
	"ALT_0=",
	"ALT_1=",
	"ALT_2=",
	"ALT_3=",		/* 75 */
	"ALT_4=",
	"ALT_5=",
	"ALT_6=",
	"ALT_7=",
	"ALT_8=",		/* 80 */
	"ALT_9=",
	"CALLUPDATE",
	"TIME_OFFSET=",
	"TIME_MASTER",
	"CTCOMPATIBLE",		/*  85  */
	"TWO_POINTS",
	"MULT_LIST=",
	"SERIAL+SECTION",
	"SECTION_MULT",
	"MARKERS=",		/* 90 */
	"DX_&_SECTIONS",
	"MARKERDOTS=",
	"MARKERCALLS=",
	"NOB4",
	/*LZ3NY */
	"COUNTRYLIST=",		//by lz3ny      /* 95 */
	"COUNTRY_LIST_POINTS=",	//by lz3ny
	"USE_COUNTRYLIST_ONLY",	//by lz3ny
	"MY_COUNTRY_POINTS=",	//by lz3ny
	"MY_CONTINENT_POINTS=",	//by lz3ny
	"DX_POINTS=",		//by lz3ny                 /* 100 */
	"SHOW_TIME",
	"RXVT",
	"VKM1=",
	"VKM2=",
	"VKM3=",		/* 105 */
	"VKM4=",
	"VKM5=",
	"VKM6=",
	"VKM7=",
	"VKM8=",		/* 110 */
	"VKM9=",
	"VKM10=",
	"VKM11=",
	"VKM12=",
	"VKSPM=",		/* 115 */
	"VKCQM=",
	"WAZMULT",
	"ITUMULT",
	"CQDELAY=",
	"PFX_MULT",		/* 120 */
	"CONTINENT_EXCHANGE",
	"RULES=",
	"NOAUTOCQ",
	"SSBMODE",
	"NO_BANDSWITCH_ARROWKEYS",	/* 125 */
	"RIGCONF",
	"TLFCOLOR1=",
	"TLFCOLOR2=",
	"TLFCOLOR3=",
	"TLFCOLOR4=",		/* 130 */
	"TLFCOLOR5=",
	"TLFCOLOR6=",
	"SYNCFILE=",
	"SSBPOINTS=",
	"CWPOINTS=",		/* 135 */
	"SOUNDCARD",
	"SIDETONE_VOLUME=",
	"S_METER=",
	"SC_DEVICE=",
	"MFJ1278_KEYER=",	/* 140 */
	"CLUSTERLOGIN=",
	"ORION_KEYER",
	"INITIAL_EXCHANGE=",
	"CWBANDWIDTH=",
	"LOWBAND_DOUBLE",	/* 145 */
	"CLUSTER_LOG",
	"SERIAL+GRID4",
	"CHANGE_RST",
	"GMFSK=",
	"RTTYMODE",		/* 150 */
	"DIGIMODEM=",
	"LOGFREQUENCY",
	"IGNOREDUPE",
	"CABRILLO=",
	"CW_TU_MSG=",		/* 155 */	/* deprecated */
	"VKCWR=",				/* deprecated */
	"VKSPR="				/* deprecated */
    };

    char teststring[80];
    char buff[40];
    char outputbuff[80];
    int ii;
    char *j;
    int jj, hh;
    char *tk_ptr;

    for (ii = 0; ii < MAX_COMMANDS; ii++) {

	teststring[0] = '\0';
	strncat(teststring, commands[ii], 79);

	if (strncmp(inputbuffer, teststring, strlen(teststring)) == 0) {
	    break;
	}
    }

    switch (ii) {

    case 0:{
	    KeywordNotSupported(teststring);
	    break;
	}
    case 1:{
	    KeywordNotSupported(teststring);
	    break;
	}
    case 2 ... 10:{	/* messages */
	    strcpy(message[ii - 2], inputbuffer + 3);
	    break;
	}
    case 11 ... 13:{
	    strcpy(message[ii - 2], inputbuffer + 4);
	    break;
	}
    case 14:{
	    strcpy(message[ii - 2], inputbuffer + 11);
	    strcpy(sp_return, message[12]);
	    break;
	}
    case 15:{
	    strcpy(message[ii - 2], inputbuffer + 10);
	    strcpy(cq_return, message[13]);
	    break;	/* end messages */
	}
    case 16:{
	    if (strlen(inputbuffer) > 6 + 20-1) {
		mvprintw(6,0,
			"WARNING: Defined call sign too long! exiting...\n");
		refreshp();
		exit(1);
	    }
	    if (strlen(inputbuffer) > 6)
		strcpy(call, inputbuffer + 5);
	    else {
		mvprintw(6, 0,
			 "WARNING: No callsign defined in logcfg.dat! exiting...\n");
		refreshp();
		exit(1);
	    }
	    // check that call sign can be found in cty database !!
	    break;
	}
    case 17:{
	    strcpy(whichcontest, inputbuffer + 8);
	    whichcontest[strlen(whichcontest) - 1] = '\0';
	    if (strlen(whichcontest) > 40) {
		showmsg
		    ("WARNING: contest name is too long! exiting...");
		exit(1);
	    }
	    setcontest();
	    break;
	}
    case 18:{
	    logfile[0] = '\0';
	    strcat(logfile, inputbuffer + 8);
	    logfile[strlen(logfile) - 1] = '\0';
	    break;
	}
    case 19:{

	    keyer_device[0] = '\0';
	    strncat(keyer_device, inputbuffer + 13, 9);
	    keyer_device[strlen(keyer_device) - 1] = '\0';
	    break;
	}
    case 20:{		// Use the bandswitch output on parport0
	    use_bandoutput = 1;
	    if (strlen(inputbuffer) > 12) {
		for (jj = 0; jj <= 9; jj++)	// 10x
		{
		    hh = ((int) (inputbuffer[jj + 10])) - 48;

		    if (hh >= 0 && hh <= 9)
			bandindexarray[jj] = hh;
		    else
			bandindexarray[jj] = 0;

		}
	    }
	    break;
	}
    case 21:{
	    recall_mult = 1;
	    break;
	}
    case 22:{
	    one_point = 1;
	    universal = 1;
	    break;
	}
    case 23:{
	    three_point = 1;
	    universal = 1;
	    break;
	}
    case 24:{
	    wysiwyg_multi = 1;
	    break;
	}
    case 25:{
	    wysiwyg_once = 1;
	    break;
	}
    case 26:{
	    trx_control = 1;
	    break;
	}
    case 27:{
	    rit = 1;
	    break;
	}
    case 28:{
	    shortqsonr = 1;
	    break;
	}
    case 29:{
	    shortqsonr = 0;
	    break;
	}
    case 30:{
	    contest = 1;
	    break;
	}
    case 31:{
	    cluster = CLUSTER;
	    break;
	}
    case 32:{
	    int len;
	    
	    cluster = MAP;

	    /* init bandmap filtering */
	    bm_config.allband = 1;
	    bm_config.allmode = 1;
	    bm_config.showdupes = 1;
	    bm_config.skipdupes = 0;
	    bm_config.livetime = 900;

	    /* Allow configuration of bandmap display if keyword
	     * is followed by a '='
	     * Parameter format is BANDMAP=<xxx>,<number>
	     * <xxx> - string parsed for the letters B, M, D and S
	     * <number> - spot livetime in seconds (>=300)
	     */
	    len = strlen(teststring);
	    if (inputbuffer[len] == '=') {
		char **fields;
		fields = g_strsplit(inputbuffer+len+1, ",", 2);
		if (fields[0] != NULL) {
		    char *ptr = fields[0];
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
			    default:
				      break;
			}
		    }
		}

		if (fields[1] != NULL) {
		    int livetime;
		    g_strstrip(fields[1]);
		    livetime = atoi(fields[1]);
		    if (livetime >= 300)
			/* aging called every 2 seconds */
			bm_config.livetime = livetime/2;
		}


		g_strfreev(fields);
	    }
	    break;
	}
    case 33:{
	    KeywordNotSupported(teststring);
	    break;
		}
    case 34:{
	    showscore_flag = 1;
	    break;
	}
    case 35:{
	    searchflg = 1;
	    break;
	}
    case 36:{
	    KeywordNotSupported(teststring);
	    break;
	}
    case 37:{
	    demode = 1;
	    break;
	}
    case 38:{
	    buff[0] = '\0';
	    strncat(buff, inputbuffer + 8, 2);
	    speed = speed_conversion(atoi(buff));
	    break;
	}
    case 39:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 5);
	    if ((atoi(buff) > -1) && (atoi(buff) < 1001)) {
		strncpy(tonestr, buff, 4);
		tonestr[3] = '\0';
	    }
	    break;
	}
    case 40:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 7);
	    weight = atoi(buff);
	    if (weight < -50)
		weight = -50;
	    if (weight > 50)
		weight = 50;
	    break;
	}
    case 41:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 8);
	    txdelay = atoi(buff);
	    if (txdelay > 50)
		txdelay = 50;
	    if (txdelay < 0)
		txdelay = 0;
	    break;
	}
    case 42:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 9);
	    outputbuff[0] = '\0';
	    sprintf(outputbuff, "WWV R=%d\n", atoi(buff));
	    strcpy(lastwwv, outputbuff);

	    break;
	}
    case 43:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 4);
	    outputbuff[0] = '\0';
	    sprintf(outputbuff, "WWV SFI=%d\n", atoi(buff));
	    strcpy(lastwwv, outputbuff);

	    break;
	}
    case 44:{
	    showfreq = 1;
	    break;
	}
    case 45:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 4);
	    if ((strncmp(buff, "MC", 2) == 0)
		|| (strncmp(buff, "mc", 2) == 0)) {
		editor = EDITOR_MC;
		break;
	    }

	    j = strstr(inputbuffer, "joe");
	    if (j != NULL) {
		editor = EDITOR_JOE;
		break;
	    }
	    j = strstr(inputbuffer, "vi");
	    if (j != NULL) {
		editor = EDITOR_VI;
		break;
	    } else {
		editor = EDITOR_E3;
		break;
	    }
	}
    case 46:{
	    partials = 1;
	    break;
	}

    case 47:{
	    use_part = 1;
	    break;
	}
    case 48:{
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
	}
    case 51:{
	    KeywordNotSupported(teststring);
	    break;
	}
    case 52:{
	    exchange_serial = 1;
	    break;
	}
    case 53:{
	    country_mult = 1;
	    break;
	}
    case 54:{
	    two_eu_three_dx_points = 1;
	    break;
	}
    case 55:{
	    portable_x2 = 1;
	    break;
	}
    case 56:{
	    mixedmode = 1;
	    break;
	}
    case 57:{
	    strncpy(pr_hostaddress, inputbuffer + 11, 47);
	    pr_hostaddress[strlen(pr_hostaddress) - 1] = '\0';
	    break;
	}
    case 58:{
	    buff[0] = '\0';
	    strncat(buff, inputbuffer + 11, 5);
	    portnum = atoi(buff);
	    packetinterface = TELNET_INTERFACE;
	    break;
	}
    case 59:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 8);
	    if (strlen(buff) > 2) {
		strncpy(tncportname, buff, 39);
	    } else
		tncport = atoi(buff) + 1;

	    packetinterface = TNC_INTERFACE;
	    break;
	}
    case 60:{
	    packetinterface = FIFO_INTERFACE;
	    break;
	}
    case 61:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 9);

	    if (strncmp(buff, "ORION", 5) == 0)
		rignumber = 2000;
	    else
		rignumber = atoi(buff);
#ifdef HAVE_LIBHAMLIB
	    myrig_model = (rig_model_t) rignumber;
#endif

	    break;
	}
    case 62:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 9);
	    serial_rate = atoi(buff);
	    break;
	}
    case 63:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 9);
	    tnc_serial_rate = atoi(buff);
	    break;
	}
    case 64:{
	    buff[0] = '\0';
	    strcat(buff, inputbuffer + 8);
	    if (buff[0] == '0' || buff[0] == '1') {
		rig_port = atoi(buff);
	    } else {
		strncpy(rigportname, buff, 39);
	    }
	    break;
	}
    case 65:{
	    keyerport = NET_KEYER;
	    break;
	}
    case 66:{
	    netkeyer_port = atoi(inputbuffer + 13);
	    break;
	}
    case 67:{
	    strncpy(netkeyer_hostaddress, inputbuffer + 13, 16);
	    netkeyer_hostaddress[strlen(netkeyer_hostaddress) -
				 1] = '\0';
	    break;
	}
    case 68:{
	    if (node < MAXNODES) {
		strncpy(bc_hostaddress[node], inputbuffer + 8, 16);
		bc_hostaddress[node][strlen(bc_hostaddress[node]) -
				     1] = '\0';
		if (node++ < MAXNODES)
		    nodes++;
	    }
	    lan_active = 1;
	    break;
	}
    case 69:{
	    thisnode = inputbuffer[9];
	    break;
	}
    case 70:{
	    cqwwm2 = 1;
	    break;
	}
    case 71:{
	    landebug = 1;
	    break;
	}
    case 72 ... 81:{	/* messages */
	    strcpy(message[ii - 58], inputbuffer + 6);
	    break;
	}
    case 82:{
	    call_update = 1;
	    break;
	}
    case 83:{
	    buff[0] = '\0';
	    strncat(buff, inputbuffer + 12, 3);
	    timeoffset = atoi(buff);
	    if (timeoffset > 23)
		timeoffset = 23;
	    if (timeoffset < -23)
		timeoffset = -23;
	    break;
	}
    case 84:{
	    time_master = 1;
	    break;
	}
    case 85:{
	    ctcomp = 1;
	    break;
	}
    case 86:{
	    two_point = 1;
	    universal = 1;
	    break;
	}
    case 87:{
	    multsfile[0] = '\0';
	    strncat(multsfile, inputbuffer + 10, 79);
	    g_strchomp(multsfile);
	    multlist = 1;
	    universal = 1;
	    break;
	}
    case 88:{
	    serial_section_mult = 1;
	    break;
	}
    case 89:{
	    sectn_mult = 1;
	    break;
	}
    case 90:{
	    markerfile[0] = '\0';
	    outputbuff[0] = '\0';
	    strcat(outputbuff, inputbuffer);

	    if (strlen(outputbuff) > 8) {
		strncpy(markerfile, outputbuff + 8,
			strlen(outputbuff) - 9);
		xplanet = 1;
	    }
	    break;
	}
    case 91:{
	    dx_arrlsections = 1;
	    setcontest();
	    break;
	}
    case 92:{
	    markerfile[0] = '\0';
	    outputbuff[0] = '\0';
	    strcat(outputbuff, inputbuffer);

	    if (strlen(outputbuff) > 11) {
		strncpy(markerfile, outputbuff + 11,
			strlen(outputbuff) - 12);
		xplanet = 2;
	    }
	    break;
	}
    case 93:{
	    markerfile[0] = '\0';
	    outputbuff[0] = '\0';
	    strcat(outputbuff, inputbuffer);

	    if (strlen(outputbuff) > 12) {
		strncpy(markerfile, outputbuff + 12,
			strlen(outputbuff) - 13);
		xplanet = 3;
	    }
	    break;
	}
    case 94:{
	    nob4 = 1;
	    break;
	}
/* LZ3NY mods */
    case 95:{
	    /* COUNTRY_LIST   (in file or listed in logcfg.dat)     LZ3NY
	       First of all we are checking if inserted data in
	       COUNTRY_LIST= is a file name.  If it is we start
	       parsing the file. If we got our case insensitive contest name,
	       we copy the multipliers from it into multipliers_list.
	       If the input was not a file name we directly copy it into
	       multiplier_list (must not have a preceeding contest name).
	       The last step is to parse the multipliers_list into an array
	       (mit_multiplier_list) for future use.
	     */

	    int mit_fg = 0;
	    static char multiplier_list[50] = ""; 	/* use only first
							   COUNTRY_LIST
							   definition */
	    char mit_multlist[255] = "";
	    char buffer[255] = "";
	    FILE *fp;

	    if (strlen(multiplier_list) == 0) {	/* if first definition */
		g_strlcpy(mit_multlist, inputbuffer + 12, sizeof(mit_multlist));
		g_strchomp(mit_multlist);	/* drop trailing whitespace */

		if ((fp = fopen(mit_multlist, "r")) != NULL) {

		    while ( fgets(buffer, sizeof(buffer), fp) != NULL ) {

			g_strchomp( buffer ); /* no trailing whitespace*/

			/* accept only a line starting with the contest name
			 * (CONTEST=) followed by ':' */
			if (strncasecmp (buffer, whichcontest, 
				strlen(whichcontest) - 1) == 0) {

			    strncpy(multiplier_list,
				    buffer + strlen(whichcontest) + 1,
				    strlen(buffer) - 1);
			}
		    }

		    fclose(fp);
		} else {	/* not a file */

		    if (strlen(mit_multlist) > 0)
			strcpy(multiplier_list, mit_multlist);
		}
	    }

	    /* LZ3NY creating the array */
	    mit_mult_array = strtok(multiplier_list, ":,.- \t");
	    mit_fg = 0;

	    if (mit_mult_array != NULL) {
		while (mit_mult_array) {
		    strcpy(mit_multiplier_list[mit_fg], mit_mult_array);
		    mit_mult_array = strtok(NULL, ":,.-_\t ");
		    mit_fg++;
		}
	    }

	    /* on which multiplier side of the rules we are */
	    getpx(call);
	    mult_side = exist_in_country_list();
	    setcontest();
	    break;
	}

    case 96:{		// COUNTRY_LIST_POINTS
	    g_strlcpy(c_temp, inputbuffer + 20, sizeof(c_temp));
	    if (countrylist_points == -1)
		countrylist_points = atoi(c_temp);

	    break;
	}
    case 97:{		// COUNTRY_LIST_ONLY
	    countrylist_only = 1;
	    if (mult_side == 1)
		countrylist_only = 0;

	    break;
	}
    case 98:{		//HOW Many points scores my country  lz3ny
	    g_strlcpy(c_temp, inputbuffer + 18, sizeof(c_temp));
	    if (my_country_points == -1)
		my_country_points = atoi(c_temp);

	    break;
	}
    case 99:{		//MY_CONTINENT_POINTS       lz3ny
	    g_strlcpy(c_temp, inputbuffer + 20, sizeof(c_temp));
	    if (my_cont_points == -1)
		my_cont_points = atoi(c_temp);

	    break;
	}
    case 100:{		//DX_CONTINENT_POINTS       lz3ny
	    g_strlcpy(c_temp, inputbuffer + 10, sizeof(c_temp));
	    if (dx_cont_points == -1)
		dx_cont_points = atoi(c_temp);

	    break;
	   }
/* end LZ3NY mod */
    case 101:{		// show time in searchlog window
		show_time = 1;
		break;
	    }
    case 102:{		// use rxvt colours
		use_rxvt = 1;
		break;
	    }
    case 103 ... 111:{	// get phone messages
		strncpy(ph_message[ii - 103], inputbuffer + 5, 70);
		ph_message[ii - 103][strlen(ph_message[ii - 103]) -
				     1] = '\0';
		mvprintw(15, 5, "A: Phone message #%d is %s", ii - 103, ph_message[ii - 103]);	// (W9WI)
		refreshp();
		//                             system ("sleep 2");
		if (strlen(ph_message[ii - 103]) > 0)
		    use_vk = 1;
		break;
	    }
    case 112 ... 116:{	// get phone messages
		strncpy(ph_message[ii - 103], inputbuffer + 6, 39);
		ph_message[ii - 103][strlen(ph_message[ii - 103]) -
				     1] = '\0';
		mvprintw(15, 5, "B: Phone message #%d is %s", ii - 103, ph_message[ii - 103]);	// (W9WI)
		refreshp();
		//                             system ("sleep 2");
		if (strlen(ph_message[ii - 103]) > 0)
		    use_vk = 1;
		break;
	    }
    case 117:{		// WAZ Zone is a Multiplier
		wazmult = 1;
		break;
	    }
    case 118:{		// ITU Zone is a Multiplier
		itumult = 1;
		break;
	    }
    case 119:{		// CQ Delay (sec)
		buff[0] = '\0';
		if (strlen(inputbuffer) >= 9) {
		    strncpy(buff, inputbuffer + 8, 3);
		    cqdelay = atoi(buff);
		}
		if ((cqdelay < 3) || (cqdelay > 60))
		    cqdelay = 20;

		break;
	    }
    case 120:{		// wpx style prefixes mult
		pfxmult = 1;	// enable set points
		wpx = 1;	// handle like wpx
		break;
	    }
    case 121:{		// exchange continent abbrev
		exc_cont = 1;
		break;
	    }
    case 122:{
		strcpy(whichcontest, inputbuffer + 6);	// RULES=
		whichcontest[strlen(whichcontest) - 1] = '\0';
		if (strlen(whichcontest) > 40) {
		    showmsg
			("WARNING: contest name is too long! exiting...");
		    sleep(5);
		    exit(1);
		}
		setcontest();
		break;
	    }
    case 123:{		// don't use auto_cq
		noautocq = 1;
		break;
	    }
    case 124:{		// start in SSB mode
		trxmode = SSBMODE;
		break;
	    }
    case 125:{		// arrow keys don't switch bands...
		no_arrows = 1;
		break;
	    }
    case 126:{		// Hamlib rig conf parameters
		if (strlen(inputbuffer + 8) >= 80) {
		    showmsg
			("WARNING: rigconf parameters too long! exiting...");
		    sleep(5);
		    exit(1);
		}
		strncpy(rigconf, inputbuffer + 8, 79);	// RIGCONF=
		rigconf[strlen(rigconf) - 1] = '\0';	// chop LF
		break;
	    }
    case 127:{		// define color GREEN (header)
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[1][0] = inputbuffer[10] - 48;
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[1][1] = inputbuffer[11] - 48;
		break;
	    }
    case 128:{		// define color CYAN (windows)
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[3][0] = inputbuffer[10] - 48;
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[3][1] = inputbuffer[11] - 48;
		break;
	    }
    case 129:{		// define color WHITE (log window)
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[4][0] = inputbuffer[10] - 48;
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[4][1] = inputbuffer[11] - 48;
		break;
	    }
    case 130:{		// define color MAGENTA (Marker / dupes)
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[5][0] = inputbuffer[10] - 48;
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[5][1] = inputbuffer[11] - 48;
		break;
	    }
    case 131:{		// define color BLUE (input fields)
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[6][0] = inputbuffer[10] - 48;
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[6][1] = inputbuffer[11] - 48;
		break;
	    }
    case 132:{		// define color YELLOW (window frames)
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[7][0] = inputbuffer[10] - 48;
		if (inputbuffer[10] >= 48 && inputbuffer[10] <= 57)
		    tlfcolors[7][1] = inputbuffer[11] - 48;
		break;
	    }
    case 133:{		// define name of synclogfile
		synclogfile[0] = '\0';
		strcat(synclogfile, inputbuffer + 9);
		synclogfile[strlen(synclogfile) - 1] = '\0';
		break;
	    }
    case 134:{		//SSBPOINTS=
		buff[0] = '\0';
		strcat(buff, inputbuffer + 10);
		ssbpoints = atoi(buff);
		break;
	    }
    case 135:{		//CWPOINTS=
		buff[0] = '\0';
		strcat(buff, inputbuffer + 9);
		cwpoints = atoi(buff);
		break;
	    }
    case 136:{		// SOUNDCARD, use soundcard for cw sidetone
		sc_sidetone = 1;
		break;
	    }
    case 137:{		// sound card volume (default = 70)
		buff[0] = '\0';
		strncat(buff, inputbuffer + 16, 2);
		if (atoi(buff) > -1 && atoi(buff) < 101)
		    strcpy(sc_volume, buff);
		else
		    strcpy(sc_volume, "70");
		break;
	    }
    case 138:{
		int i = 0;

		tk_ptr = strtok(inputbuffer + 8, ":,.-_\t ");

		if (tk_ptr != NULL) {
		    while (tk_ptr) {
			if (i < 20)
			    scale_values[i] = atoi(tk_ptr);
			tk_ptr = strtok(NULL, ":,.-_\t ");
			i++;
		    }
		}

		break;
	    }
    case 139:{		// dsp for s-meter
		strncpy(sc_device, inputbuffer + 10,
			sizeof(sc_device) - 1);
		sc_device[strlen(sc_device) - 1] = '\0';
		break;
	    }
    case 140:{
		keyerport = MFJ1278_KEYER;
		strncpy(controllerport, inputbuffer + 14,
			sizeof(controllerport) - 1);
		controllerport[strlen(controllerport) - 1] = '\0';
		break;
	    }
    case 141:{
		strcpy(clusterlogin, inputbuffer + 13);
		break;
	    }
    case 142:{
		keyerport = ORION_KEYER;
		break;
	    }
    case 143:{
		strncpy(exchange_list, inputbuffer + 17,
			sizeof(exchange_list) - 1);
		exchange_list[strlen(exchange_list) - 1] = '\0';
		break;
	    }
    case 144:{
		cw_bandwidth = atoi(inputbuffer + 12);
		break;
	    }
    case 145:{
		lowband_point_mult = 1;
		break;
	    }
    case 146:{
		clusterlog = 1;
		break;
	    }
    case 147:{
		serial_grid4_mult = 1;
		break;
	    }
    case 148:{
		change_rst = 1;
		break;
	    }
    case 149:{
		keyerport = GMFSK;
		strncpy(controllerport, inputbuffer + 6,
			sizeof(controllerport) - 1);
		controllerport[strlen(controllerport) - 1] = '\0';
		break;
	    }
    case 150:{		// start in digital mode
		trxmode = DIGIMODE;
		modem_mode[0] = '\0';
		strcat(modem_mode, "RTTY");
		break;
	    }
    case 151:{
		rttyoutput[0] = '\0';
		strncat(rttyoutput, inputbuffer + 10, 110);
		rttyoutput[strlen(rttyoutput) - 1] = '\0';
		break;
	    }
    case 152:{
		logfrequency = 1;
		break;
	    }
    case 153:{
		ignoredupe = 1;
		break;
	    }
    case 154:{		/* read name of cabrillo format to use */
	    	cabrillo = strdup(g_strchomp(inputbuffer + 9));
    		break;
	    }
    case 155:
    case 156:
    case 157:{
		KeywordNotSupported(teststring);
		break;
	    }

    default: {
		KeywordNotSupported(g_strstrip(inputbuffer));
		break;
	    }
    }

}

int speed_conversion(int cwspeed)
{

    int x;

    switch (cwspeed) {

    case 0 ... 6:{
	    x = 0;
	    break;
	}
    case 7 ... 12:{
	    x = 1;
	    break;
	}
    case 13 ... 14:{
	    x = 2;
	    break;
	}
    case 15 ... 16:{
	    x = 3;
	    break;
	}
    case 17 ... 18:{
	    x = 4;
	    break;
	}
    case 19 ... 20:{
	    x = 5;
	    break;
	}
    case 21 ... 22:{
	    x = 6;
	    break;
	}
    case 23 ... 24:{
	    x = 7;
	    break;
	}
    case 25 ... 26:{
	    x = 8;
	    break;
	}
    case 27 ... 28:{
	    x = 9;
	    break;
	}
    case 29 ... 30:{
	    x = 10;
	    break;
	}
    case 31 ... 36:{
	    x = 11;
	    break;
	}
    case 37 ... 42:{
	    x = 12;
	    break;
	}
    case 43 ... 48:{
	    x = 13;
	    break;
	}
    case 49 ... 50:{
	    x = 14;
	    break;
	}
    case 51 ... 54:{
	    x = 15;
	    break;
	}
    case 55 ... 57:{
	    x = 16;
	    break;
	}
    case 58 ... 60:{
	    x = 17;
	    break;
	}
    case 61 ... 63:{
	    x = 18;
	    break;
	}
    default:{
	    x = 19;
	    break;
	}
    }

    return (x);
}

/** Complain about not supported keyword */
void KeywordNotSupported(char *keyword) {
    char msgbuffer[100];
    sprintf(msgbuffer,
	    "Keyword '%s' not supported. See man page and README.\n",
	    keyword);
    attron(A_STANDOUT);
    showmsg(msgbuffer);
    attroff(A_STANDOUT);
    beep();
    sleep(2);
}
