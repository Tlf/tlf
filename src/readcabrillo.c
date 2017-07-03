/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2017 Ervin Hegedus <airween@gmail.com>
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

#include "readcabrillo.h"
#include "globalvars.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>
#include "tlf_curses.h"

#include "cabrillo_utils.h"
#include "getsummary.h"
#include "log_to_disk.h"
#include "startmsg.h"
#include "addmult.h"
#include "getexchange.h"

#define MAX_CABRILLO_LEN 255

static int cablinecnt = 0;

char spinner_syms[] = "|/-\\";

/* set band from freq
 *
 * set band value based on the freq, which readed from QSO line
 *
 */

int set_band_from_freq(int freq) {

	int cab_bandinx;

	switch ((int)freq) {
		case 1800 ... 2000:{
			cab_bandinx = BANDINDEX_160;
			break;
		    }
		case 3500 ... 4000:{
			cab_bandinx = BANDINDEX_80;
			break;
		    }
		case 7000 ... 7300:{
			cab_bandinx = BANDINDEX_40;
			break;
		    }
		case 10100 ... 10150:{
			cab_bandinx = BANDINDEX_30;
			break;
		    }
		case 14000 ... 14350:{
			cab_bandinx = BANDINDEX_20;
			break;
		    }
		case 18068 ... 18168:{
			cab_bandinx = BANDINDEX_17;
			break;
		    }
		case 21000 ... 21450:{
			cab_bandinx = BANDINDEX_15;
			break;
		    }
		case 24890 ... 24990:{
			cab_bandinx = BANDINDEX_12;
			break;
		    }
		case 28000 ... 29700:{
			cab_bandinx = BANDINDEX_10;
			break;
		    }
		default:
			cab_bandinx = NBANDS;	/* out of band */
	}

	return cab_bandinx;
}

void concat_comment(char * exchstr) {
    if (strlen(comment) > 0) {
	strcat(comment, " ");
    }
    strcat(comment, exchstr);
}

/* cabrillo QSO to Tlf format
 *
 * walk through the lines which starts with QSO/X-QSO, and
 * build a virtual QSO; then it calls the existing functions
 * to add to the real log, used by the Cabrillo datas (eg. freq,
 * date, time, band, ...) instead of the real
 */

void cab_qso_to_tlf(char * line, struct cabrillo_desc *cabdesc) {

    extern float freq;
    extern struct tm *time_ptr_cabrillo;

    int item_count;
    struct line_item *item;
    GPtrArray *item_array;
    int i, t_qsonum, t_bandinx;
    item_count = cabdesc->item_count;
    item_array = cabdesc->item_array;
    int shift = 0, pos = 0;
    char tempstr[80], timestr[3], t_qsonrstr[5], *gridmult = "";

    // [UNIVERSAL]
    // QSO=FREQ,5;MODE,2;DATE,10;TIME,4;MYCALL,13;RST_S,3;EXC_S,6;HISCALL,13;RST_R,3;EXCH,6

    // [WAEDC]
    // QSO=FREQ,5;MODE,2;DATE,10;TIME,4;MYCALL,13;RST_S,3;EXC_S,6;HISCALL,13;RST_R,3;EXCH,6
    // QSO: 14043 CW 2016-08-13 0022 HA2OS         599 0004   KL7SB/VY2     599 025
    // QSO:  7002 CW 2016-08-13 0033 HA2OS         599 0008   K6ND          599 044
    //
    // Tlf log:
    //  20CW  13-Aug-16 00:22 0004  KL7SB/VY2      599  599  025           KL7      1  14043.5
    //  40CW  13-Aug-16 00:33 0008  K6ND           599  599  044           K6       1   7002.8

    if (strncmp(line, "QSO", 3) == 0) {
	shift = 5;
	cablinecnt++;
    }
    else if (strncmp(line, "X-QSO", 5) == 0) {
	shift = 7;
	cablinecnt++;
    }
    pos = shift;
    if (time_ptr_cabrillo == NULL) {
	time_ptr_cabrillo = malloc(sizeof (struct tm));
    }
    for  (i = 0; i < item_count; i++) {
	item = g_ptr_array_index( item_array, i );
	strncpy(tempstr, line+pos, item->len);
	tempstr[item->len] = '\0';
	g_strchomp(tempstr);
	pos += item->len;
	pos++;		// space between fields
	switch (item->tag) {
	    case FREQ:
		freq = atoi(tempstr)*1.0;
		t_bandinx = bandinx;
		bandinx = set_band_from_freq(freq);
		break;
	    case MODE:
		if (strcmp(tempstr, "CW") == 0) {
		    trxmode = CWMODE;
		}
		else if (strcmp(tempstr, "PH") == 0) {
		    trxmode = SSBMODE;
		}
		else {
		    trxmode = DIGIMODE;
		}
		break;
	    case DATE:
		strptime(tempstr, "%Y-%m-%d", time_ptr_cabrillo);
		break;
	    case TIME:
		timestr[0] = tempstr[0];
		timestr[1] = tempstr[1];
		timestr[2] = '\0';
		time_ptr_cabrillo->tm_hour = atoi(timestr);
		timestr[0] = tempstr[2];
		timestr[1] = tempstr[3];
		timestr[2] = '\0';
		time_ptr_cabrillo->tm_min = atoi(timestr);
		break;
	    case MYCALL:
		break;
	    case HISCALL:
		strncpy(hiscall, tempstr, strlen(tempstr));
		hiscall[strlen(tempstr)] = '\0';
		break;
	    case RST_S:
		strncpy(my_rst, tempstr, strlen(tempstr));
		my_rst[strlen(tempstr)] = '\0';
		break;
	    case RST_R:
		strncpy(his_rst, tempstr, strlen(tempstr));
		his_rst[strlen(tempstr)] = '\0';
		break;
	    case EXCH:
		strncpy(comment, tempstr, strlen(tempstr));
		comment[strlen(tempstr)] = '\0';
		break;
	    case EXC1:
		concat_comment(tempstr);
		break;
	    case EXC2:
		concat_comment(tempstr);
		break;
	    case EXC3:
		concat_comment(tempstr);
		break;
	    case EXC4:
		concat_comment(tempstr);
		break;
	    case EXC_S:
		break;
	    case TX:
		break;
	    case QTCRCALL:
		break;
	    case QTCHEAD:
		break;
	    case QTCSCALL:
		break;
	    case QTC:
	    case NO_ITEM:
	    default:
		break;
	}

    }
    t_bandinx = bandinx;
    strcpy(t_qsonrstr, qsonrstr);
    t_qsonum = qsonum;
    qsonum = cablinecnt;
    sprintf(qsonrstr, "%04d", cablinecnt);
    if (serial_grid4_mult == 1) {
	gridmult = getgrid(comment);
	strcpy(section, gridmult);
    }
    checkexchange(0);
    log_to_disk(0);
    strcpy(qsonrstr, t_qsonrstr);
    qsonum = t_qsonum;
    bandinx = t_bandinx;
}

void show_readcab_msg(int mode, char *msg) {

    if (mode == READCAB_MODE_CLI) {
	showmsg(msg);
	refreshp();
    }
}

/** readcabrillo
 *
 * Main routine to read the cabrillo lines, parses them, and
 * creates a new Tlf compatible log.
 *
 */

int readcabrillo(int mode)
{
    extern char* cabrillo;
    extern char call[];

    char* cab_dfltfile;
    struct cabrillo_desc *cabdesc;
    char input_logfile[80];
    char output_logfile[80], temp_logfile[80];
    char logline[MAX_CABRILLO_LEN];
    char tempstr[80];
    int linnr = 0;

    FILE *fp1, *fp2;

    do_cabrillo = 1;

    if (cabrillo == NULL) {
	show_readcab_msg(mode, "Missing CABRILLO= keyword (see man page)");
	sleep(2);
	do_cabrillo = 0;
	return(1);
    }

    /* Try to read cabrillo format first from local directory.
     * Try also in default data dir if not found.
     */
    cabdesc = read_cabrillo_format("cabrillo.fmt", cabrillo);
    if (!cabdesc) {
	cab_dfltfile = g_strconcat(PACKAGE_DATA_DIR, G_DIR_SEPARATOR_S,
	    "cabrillo.fmt", NULL);
	cabdesc = read_cabrillo_format(cab_dfltfile, cabrillo);
	g_free(cab_dfltfile);
    }

    if (!cabdesc) {
        show_readcab_msg(mode, "Cabrillo format specification not found!");
	sleep(2);
	do_cabrillo = 0;
	return(2);
    }
    else {
	sprintf(tempstr, "CABRILLO format: %s", cabrillo);
	show_readcab_msg(mode, tempstr);
	sleep(1);
    }

    strcpy(input_logfile, call);
    g_strchomp(input_logfile); /* drop \n */
    strcat(input_logfile, ".cbr");

    strcpy(output_logfile, "IMPORT_");
    strcat(output_logfile, logfile);
    strcpy(temp_logfile, logfile);
    strcpy(logfile, output_logfile);

    if ((fp2 = fopen(output_logfile, "w")) == NULL) {
	sprintf(tempstr, "Can't open output logfile: %s.", output_logfile);
	show_readcab_msg(mode, tempstr);
	sleep(2);
	do_cabrillo = 0;
	free_cabfmt( cabdesc );
	return (1);
    }

    if ((fp1 = fopen(input_logfile, "r")) == NULL) {
	sprintf(tempstr, "Can't open input logfile: %s.", input_logfile);
	show_readcab_msg(mode, tempstr);
	sleep(2);
	do_cabrillo = 0;
	free_cabfmt( cabdesc );
	return (1);
    }

    while(fgets(logline, MAX_CABRILLO_LEN, fp1) != NULL) {
	if (strncmp(logline, "QSO", 3) == 0 || strncmp(logline, "X-QSO", 5) == 0) {
	  linnr++;
	  cab_qso_to_tlf(logline, cabdesc);
	}
    }

    fclose(fp1);

    free_cabfmt( cabdesc );

    strcpy(logfile, temp_logfile);
    do_cabrillo = 0;

    return 0;
}
