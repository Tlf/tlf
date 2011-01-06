/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
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
	/* ------------------------------------------------------------
	 *   write cabrillo  file
	 *
	 *--------------------------------------------------------------*/
#include "writecabrillo.h"
#include "printcall.h"
#include "curses.h"

int write_cabrillo(void)
{
    extern char backgrnd_str[];
    extern char logfile[];
    extern char call[];
    extern int cqww;
    extern int arrldx_usa;
    extern int other_flg;
    extern int wysiwyg_multi;
    extern int wysiwyg_once;
    extern int serial_grid4_mult;
    extern char mycqzone[];
    extern char exchange[];
    extern int cqwwm2;
    extern int arrlss;
    extern int wpx;
    extern char whichcontest[];

    int rc;
    char standardexchange[70] = "";
    char buf[181];
    char buffer[4000] = "";

    FILE *fp1, *fp2;

    getsummary();

    if (strlen(exchange) > 0)
	strcpy(standardexchange, exchange);

    if ((fp1 = fopen(logfile, "r")) == NULL) {
	fprintf(stdout, "Opening logfile not possible.\n");
	return (1);
    }
    if ((fp2 = fopen("./cabrillo", "w")) == NULL) {
	fprintf(stdout, "Opening cbr  file not possible.\n");
	fclose(fp1);		//added by F8CFE
	return (2);
    }
    if (strlen(standardexchange) == 0) {
	nicebox(14, 0, 1, 78, "Exchange used:");
	mvprintw(15, 1,
		 "                                                       ");
	mvprintw(15, 1, "");
	attron(COLOR_PAIR(7) | A_STANDOUT);
	echo();
	if (arrlss == 1)
	    getnstr(standardexchange, 6);
	else
	    getnstr(standardexchange, 10);

	noecho();
    }

    while ( fgets(buf, 180, fp1) != NULL ) {

	if (buf[0] != ';' && strlen(buf) > 60) {

	    buffer[0] = '\0';

	    strcat(buffer, "QSO: ");
/*------------------------------------------------------------------
frequency
-------------------------------------------------------------------*/
	    if (buf[1] == '6')
		strcat(buffer, " 1800");
	    else if (buf[1] == '8')
		strcat(buffer, " 3500");
	    else if (buf[1] == '4')
		strcat(buffer, " 7000");
	    else if (buf[1] == '2')
		strcat(buffer, "14000");
	    else if (buf[1] == '1' && buf[2] == '5')
		strcat(buffer, "21000");
	    else if (buf[1] == '1' && buf[2] == '0')
		strcat(buffer, "28000");

/*------------------------------------------------------------------
mode
-------------------------------------------------------------------*/

	    if (buf[3] == 'C')
		strcat(buffer, " CW 20");
	    else if (buf[3] == 'S')
		strcat(buffer, " PH 20");
	    else
		strcat(buffer, " RY 20");

/*------------------------------------------------------------------
date
-------------------------------------------------------------------*/

	    strncat(buffer, buf + 14, 2);	/* year */

	    if (buf[10] == 'J' && buf[11] == 'a')
		strcat(buffer, "-01-");
	    if (buf[10] == 'F')
		strcat(buffer, "-02-");
	    if (buf[10] == 'M' && buf[12] == 'r')
		strcat(buffer, "-03-");
	    if (buf[10] == 'A' && buf[12] == 'r')
		strcat(buffer, "-04-");
	    if (buf[10] == 'M' && buf[12] == 'y')
		strcat(buffer, "-05-");
	    if (buf[10] == 'J' && buf[11] == 'u' && buf[12] == 'n')
		strcat(buffer, "-06-");
	    if (buf[10] == 'J' && buf[12] == 'l')
		strcat(buffer, "-07-");
	    if (buf[10] == 'A' && buf[12] == 'g')
		strcat(buffer, "-08-");
	    if (buf[10] == 'S')
		strcat(buffer, "-09-");
	    if (buf[10] == 'O')
		strcat(buffer, "-10-");
	    if (buf[10] == 'N')
		strcat(buffer, "-11-");
	    if (buf[10] == 'D')
		strcat(buffer, "-12-");
/*------------------------------------------------------------------
day
-------------------------------------------------------------------*/

	    strncat(buffer, buf + 7, 2);
/*------------------------------------------------------------------
time
-------------------------------------------------------------------*/

	    strncat(buffer, buf + 16, 3);
	    strncat(buffer, buf + 20, 3);
/*------------------------------------------------------------------
mycall
-------------------------------------------------------------------*/

	    strncat(buffer, call, strlen(call) - 1);	/* strip the \n */
	    strncat(buffer, backgrnd_str, 15 - strlen(call));
/*------------------------------------------------------------------
exchange given
-------------------------------------------------------------------*/

	    if (arrlss == 1) {
/*------------------------------------------------------------------
report given
-------------------------------------------------------------------*/

		sprintf(buffer + 41, "%4d", atoi(buf + 22));
		strcat(buffer, "                    ");
/*------------------------------------------------------------------
exchange given
-------------------------------------------------------------------*/
		sprintf(buffer + 46, "%c", standardexchange[0]);
		strcat(buffer, "                    ");
		sprintf(buffer + 48, "%2d", atoi(standardexchange + 1));
		strcat(buffer, "                    ");
		sprintf(buffer + 51, "%s", standardexchange + 3);
		strcat(buffer, "                    ");
		sprintf(buffer + 55, "%s", buf + 29);
		strcat(buffer, "                    ");
/*------------------------------------------------------------------
exchange received
-------------------------------------------------------------------*/

		sprintf(buffer + 66, "%4d", atoi(buf + 54));
		strcat(buffer, "                    ");
		sprintf(buffer + 71, "%c", buf[59]);
		strcat(buffer, "                    ");
		sprintf(buffer + 72, "%s", buf + 60);
		strcat(buffer, "                    ");
		sprintf(buffer + 75, "%s", buf + 63);
		strcat(buffer, "                    ");
		buffer[79] = '\0';
		strcat(buffer, "\n");

	    } else		// not arllss
	    {
/*------------------------------------------------------------------
report given
-------------------------------------------------------------------*/

		if (buf[3] == 'S')
		    strcat(buffer, "59  ");
		else
		    strcat(buffer, "599 ");
/*------------------------------------------------------------------
exchange given
-------------------------------------------------------------------*/

		if (other_flg == 1 || wysiwyg_multi == 1
		    || wysiwyg_once == 1) {
		    strcat(buffer, standardexchange);
		    strncat(buffer, "            ",
			    7 - strlen(standardexchange));
		}
		/* end other (wysiwyg) */
		else if ((wpx == 1) || ((standardexchange[0] == '#')
					&& (strcmp(whichcontest, "ssa_mt")
					    != 0))) {
		    strncat(buffer, buf + 23, 4);
		    strncat(buffer, standardexchange + 1, 7);
		    strcat(buffer, " ");
		}

		else if (cqww == 1) {
		    strcat(buffer, mycqzone);
		    strcat(buffer, "     ");
		}

		else if (arrldx_usa == 1) {
		    strncat(buffer, exchange, 2);
		    strcat(buffer, "     ");

		} else if (serial_grid4_mult == 1) {
		    strcat(buffer, "  ");
		    sprintf(buffer + 49, "%s", buf + 24);
		    sprintf(buffer + 52, "%s", standardexchange + 1);

		    strcat(buffer, "                ");
		    sprintf(buffer + 60, "%s          ", buf + 29);
		    buffer[74] = '\0';
		} else {

		    strncat(buffer, standardexchange, 10);
		    strncat(buffer, "     ", strlen(buffer) - 8);
		}

/*------------------------------------------------------------------
his call
-------------------------------------------------------------------*/

		if (strcmp(whichcontest, "ssa_mt") != 0)
		    strncat(buffer, buf + 29, 14);

/*------------------------------------------------------------------
rprt given
-------------------------------------------------------------------*/

		if (buf[3] == 'S')
		    strcat(buffer, "59  ");
		else
		    strcat(buffer, "599 ");

		if (serial_grid4_mult == 1) {
		    char ssa_mt_exchange[30];
		    int i = 0, j = 0, k = 0;
//                                      strncat(buffer, buf+54, 9);  // tbf for all contests? RC
		    strcat(buffer, "                      ");

		    sprintf(buffer + 79, "%03d   ", atoi(buf + 54));

		    for (i = 0; i < 12; i++) {
			if (isalpha(buf[54 + i])) {
			    for (j = 0; j < (13 - i); j++) {
				if (isalnum(buf[54 + i + j])) {
				    ssa_mt_exchange[k] = buf[54 + i + j];
				    k++;
				} else {
				    if (j > 0
					&& isspace(buf[54 + i + j - 1])) {
					ssa_mt_exchange[k] = '\0';
					break;
				    }
				}
			    }
			    if (j > 0)
				break;
			}
		    }

		    sprintf(buffer + 83, "%s        ", ssa_mt_exchange);
		    sprintf(buffer + 90, "%s", "0");
		} else
		    strncat(buffer, buf + 54, 6);

		strcat(buffer, "  ");

		if ((cqww == 1) && (cqwwm2 == 1)) {	// cqww M2 mode
		    if (buf[79] == '*') {
			strcat(buffer, " 1\n");
		    } else
			strcat(buffer, " 0\n");
		} else {
		    if (strcmp(whichcontest, "ssa_mt") == 1)
			strcat(buffer, " 0\n");
		    else
			strcat(buffer, "\n");
		}
	    }			// end else arrlss

	    if (strlen(buffer) > 11)
		fputs(buffer, fp2);

	}

    }				// end while !eof

    fclose(fp1);
    fclose(fp2);

    fp2 = fopen("cabrillo", "a");
    fputs("END-OF-LOG:\n", fp2);
    fclose(fp2);

    rc = system("cat cabrillo >> header");
    rc = system("cp header cabrillo");
    rc = system("mv header summary.txt");

    return (0);
}

/* just >>TRIM<< */
char *trim(char *string)
{
    int runner;
    size_t strLength = strlen(string);

    for (runner = 0; runner < strLength; runner++) {
	if (*(string + runner) != ' ' && *(string + runner) != '\t')
	    break;
    }

    strcpy(string, string + runner);
    strLength = strlen(string);

    for (runner = 1; runner < strLength; runner++) {
	if (*(string + strLength - runner) != ' '
	    && *(string + strLength - runner) != '\t')
	    break;
    }
    *(string + strLength - runner + 1) = 0;
    return (string);
}

/*
    The ADIF function has been written according ADIF v1.00 specifications
    as shown on http://home.no.net/jlog/adif/adif.html or http://www.adif.org
    LZ3NY
*/

int write_adif(void)
{

    extern char logfile[];
    extern char exchange[];
    extern char whichcontest[];
    extern int exchange_serial;
    extern char modem_mode[];

//  char buf[81]="";    ### bug fix
    char buf[181] = "";
    char buffer[181] = "";
    char standardexchange[70] = "";
    char adif_tmp_name[40] = "";
    char adif_tmp_call[13] = "";
    char adif_tmp_str[2] = "";
    char adif_year_check[3] = "";
    char adif_rcvd_num[16] = "";
    char resultat[16];
    char adif_tmp_rr[5] = "";

    int adif_mode_dep = 0;

    FILE *fp1, *fp2;

    if (strlen(exchange) > 0)
	strcpy(standardexchange, exchange);

    if ((fp1 = fopen(logfile, "r")) == NULL) {
	fprintf(stdout, "Opening logfile not possible.\n");
	fclose(fp1);		//added by F8CFE
	return (1);
    }
    strcpy(adif_tmp_name, whichcontest);
    strcat(adif_tmp_name, ".adif");

    if ((fp2 = fopen(adif_tmp_name, "w")) == NULL) {
	fprintf(stdout, "Opening ADIF file not possible.\n");
	return (2);
    } else {
	fputs
	    ("######################################################################################\n",
	     fp2);
	fputs
	    ("#                     ADIF v1.00 data file exported by TLF\n",
	     fp2);
	fputs
	    ("# according to specifications on http://home.no.net/jlog/adif/adif.html\n",
	     fp2);
	fputs("#\n", fp2);
	fputs
	    ("######################################################################################\n",
	     fp2);
	fputs("<adif_ver:4>1.00\n<eoh>\n", fp2);
    }

    /* in case using write_adif() without write_cabrillo() */
    if ((strlen(standardexchange) == 0) && (exchange_serial != 1)) {
	nicebox(14, 0, 1, 78, "Exchange used:");
	mvprintw(15, 1,
		 "                                                       ");
	mvprintw(15, 1, "");
	attron(COLOR_PAIR(7) | A_STANDOUT);
	echo();

	getnstr(standardexchange, 30);
	noecho();
    }
//while  (fgets (buf,  180,  fp1))              ### bug fix
    while (fgets(buf, sizeof(buf), fp1)) {
	if ((buf[0] != ';') && ((buf[0] != ' ') || (buf[1] != ' '))
	    && (buf[0] != '#') && (buf[0] != '\n') && (buf[0] != '\r')) {
	    buffer[0] = '\0';

/* CALLSIGN */
	    strcat(buffer, "<CALL:");
	    strncpy(adif_tmp_call, buf + 29, 12);
	    strcpy(adif_tmp_call, trim(adif_tmp_call));
	    snprintf(resultat, sizeof(resultat), "%d",
		     strlen(adif_tmp_call));
	    strcat(buffer, resultat);
	    strcat(buffer, ">");
	    strcat(buffer, adif_tmp_call);

/* BAND */
	    if (buf[1] == '6')
		strcat(buffer, "<BAND:4>160M");
	    else if (buf[1] == '8')
		strcat(buffer, "<BAND:3>80M");
	    else if (buf[1] == '4')
		strcat(buffer, "<BAND:3>40M");
	    else if (buf[1] == '3')
		strcat(buffer, "<BAND:3>30M");
	    else if (buf[1] == '2')
		strcat(buffer, "<BAND:3>20M");
	    else if (buf[1] == '1' && buf[2] == '5')
		strcat(buffer, "<BAND:3>15M");
	    else if (buf[1] == '1' && buf[2] == '7')
		strcat(buffer, "<BAND:3>17M");
	    else if (buf[1] == '1' && buf[2] == '0')
		strcat(buffer, "<BAND:3>10M");

/* QSO MODE */
	    if (buf[3] == 'C')
		strcat(buffer, "<MODE:2>CW");
	    else if (buf[3] == 'S')
		strcat(buffer, "<MODE:3>SSB");
	    else if (strcmp(modem_mode, "RTTY") == 0)
		strcat(buffer, "<MODE:4>RTTY");
	    else
		strcat(buffer, "<MODE:4>DIGI");

/* QSO_DATE */
	    /* Y2K :) */
	    adif_year_check[0] = '\0';
	    strncpy(adif_year_check, buf + 14, 2);
	    if (atoi(adif_year_check) <= 70)
		strcat(buffer, "<QSO_DATE:8>20");
	    else
		strcat(buffer, "<QSO_DATE:8>19");

	    /* year */
	    strncat(buffer, buf + 14, 2);

	    /*month */
	    if (buf[10] == 'J' && buf[11] == 'a')
		strcat(buffer, "01");
	    if (buf[10] == 'F')
		strcat(buffer, "02");
	    if (buf[10] == 'M' && buf[12] == 'r')
		strcat(buffer, "03");
	    if (buf[10] == 'A' && buf[12] == 'r')
		strcat(buffer, "04");
	    if (buf[10] == 'M' && buf[12] == 'y')
		strcat(buffer, "05");
	    if (buf[10] == 'J' && buf[11] == 'u' && buf[12] == 'n')
		strcat(buffer, "06");
	    if (buf[10] == 'J' && buf[12] == 'l')
		strcat(buffer, "07");
	    if (buf[10] == 'A' && buf[12] == 'g')
		strcat(buffer, "08");
	    if (buf[10] == 'S')
		strcat(buffer, "09");
	    if (buf[10] == 'O')
		strcat(buffer, "10");
	    if (buf[10] == 'N')
		strcat(buffer, "11");
	    if (buf[10] == 'D')
		strcat(buffer, "12");

	    /*date */
	    strncat(buffer, buf + 7, 2);

/* TIME_ON */
	    strcat(buffer, "<TIME_ON:4>");
	    strncat(buffer, buf + 17, 2);
	    strncat(buffer, buf + 20, 2);

	    /* RS(T) flag */
	    if (buf[3] == 'S')
		adif_mode_dep = 2;
	    else
		adif_mode_dep = 3;

/* RST_SENT */
	    strcat(buffer, "<RST_SENT:");
	    adif_tmp_str[1] = '\0';	/*       PA0R 02/10/2003  */
	    adif_tmp_str[0] = adif_mode_dep + 48;
	    strcat(buffer, adif_tmp_str);
	    strcat(buffer, ">");
	    strncat(buffer, buf + 44, adif_mode_dep);

/* STX - sent contest number */
	    strcat(buffer, "<STX:");

	    if ((exchange_serial == 1) || (standardexchange[0] == '#')) {
		strcat(buffer, "4>");
		strncat(buffer, buf + 23, 4);
	    } else {
		snprintf(resultat, sizeof(resultat), "%d",
			 strlen(standardexchange));
		strcat(buffer, resultat);
		strcat(buffer, ">");
		strcat(buffer, trim(standardexchange));
	    }

/* RST_RCVD */
	    strncpy(adif_tmp_rr, buf + 49, 4);
	    strcpy(adif_tmp_rr, trim(adif_tmp_rr));
	    strcat(buffer, "<RST_RCVD:");
	    snprintf(resultat, sizeof(resultat), "%d",
		     strlen(adif_tmp_rr));
	    strcat(buffer, resultat);
	    strcat(buffer, ">");
	    strncat(buffer, buf + 49, adif_mode_dep);

/* SRX - received contest number */
	    strncpy(adif_rcvd_num, buf + 54, 14);
	    strcpy(adif_rcvd_num, trim(adif_rcvd_num));
	    snprintf(resultat, sizeof(resultat), "%d",
		     strlen(adif_rcvd_num));
	    strcat(buffer, "<SRX:");
	    strcat(buffer, resultat);
	    strcat(buffer, ">");
	    if (strcmp(buf + 54, " ") != 0)
		strcat(buffer, adif_rcvd_num);

/* <EOR> */
	    strcat(buffer, "<eor>\n");	//end of ADIF row
	}

	if (strlen(buffer) > 1)
	    fputs(buffer, fp2);
	buffer[0] = '\0';

    }				// end fgets() loop

    fclose(fp1);
    fclose(fp2);

    return (0);
}				// end write_adif
