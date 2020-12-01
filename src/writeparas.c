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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* ------------------------------------------------------------
 *        Write parameter file to disk
 *
 *--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "background_process.h"
#include "cw_utils.h"
#include "err_utils.h"
#include "globalvars.h"
#include "tlf.h"
#include "tlf_curses.h"

/* write a .paras file */
int writeparas_file(void) {

    extern char message[][80];
    extern const char headerline[];
    extern char logfile[];
    extern char whichcontest[];
    extern int shortqsonr;
    extern char para_word[];
    extern int cluster;
    extern int searchflg;
    extern int demode;
    extern int announcefilter;
    extern int showscore_flag;
    extern int cqdelay;
    extern int trxmode;

    FILE *fp;
    int i;

    if (strlen(my.call) <= 3) {
	TLF_LOG_WARN("Cannot write parameters file: data corrupt... ");
	return (-1);
    }

    if ((fp = fopen(".paras", "w")) == NULL) {
	TLF_LOG_ERR("writeparas.c: Error opening file.");
    }
    fputs("# Call  ----------------------------------\n", fp);
    fputs(my.call, fp);
    fputs("# Messages  F1...F12 ---------------------\n", fp);

    for (i = 0; i <= 13; i++) {
	if (i == 12)
	    fputs("# TU message S&P mode---------------------\n", fp);
	if (i == 13)
	    fputs("# TU  message CQ mode---------------------\n", fp);
	fputs(message[i], fp);
    }

    fputs("# Digi Messages  F1...F12 ---------------------\n", fp);

    for (i = 0; i <= 13; i++) {
	if (i == 12)
	    fputs("# TU message S&P mode---------------------\n", fp);
	if (i == 13)
	    fputs("# TU  message CQ mode---------------------\n", fp);
	fputs(digi_message[i], fp);
    }

    fputs("# Info for top status line----------------\n", fp);

    fputs(headerline, fp);

    fputs("# Logfile--------------------\n", fp);

    fputs(logfile, fp);
    fputs("\n", fp);

    fputs("# Contest--------------------\n", fp);

    fputs(whichcontest, fp);
    fputs("\n", fp);

    fputs("# Parameters--don't change----\n", fp);

    if (shortqsonr == 1)
	para_word[0] = 'S';	/* short */
    else
	para_word[0] = 'L';	/* long  */

    if (cluster == 0)
	para_word[1] = 'O';	/* OFF */
    else if (cluster == 1)
	para_word[1] = 'M';	/* MAP */
    else if (cluster == 2)
	para_word[1] = 'S';	/* SPOTS  */
    else if (cluster == 3)
	para_word[1] = 'A';	/* All  */

    if (searchflg == 1)
	para_word[2] = 'D';	/* DISPLAY */
    else
	para_word[2] = 'N';	/* NO DISPLAY */

    if (demode == 1)
	para_word[3] = 'D';	/* DE mode on */
    else
	para_word[3] = 'N';	/*  DE mode off */

    if (iscontest)
	para_word[4] = 'C';	/* contest  mode */
    else
	para_word[4] = 'G';	/* general qso mode */

    if (announcefilter == 1)
	para_word[5] = 'F';	/* filter  on  */
    else
	para_word[5] = 'N';	/* off */

    if (showscore_flag == 0)
	para_word[6] = 'N';	/* No score window */
    else
	para_word[6] = 'S';	/* show score window */

    para_word[7] = 48 + speed;

    if (cqdelay > 0 && cqdelay < 23)
	para_word[8] = 48 + cqdelay;

    if (trxmode == CWMODE)	/* use fifo for cw output */
	para_word[9] = 'C';
    else
	para_word[9] = 'P';

    para_word[10] = '\n';
    para_word[11] = '\0';

    fputs(para_word, fp);

    fclose(fp);

    return (0);
}


/* write .paras fiel with background thread stopped */
int writeparas(void) {
    int result;

    stop_background_process();
    result = writeparas_file();
    start_background_process();
    return result;
}

