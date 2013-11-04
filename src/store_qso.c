/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
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
 	*        Store_qso  writes qso to disk
 	*
 	*--------------------------------------------------------------*/

#include "globalvars.h"
#include "store_qso.h"

#include "log_sent_qtc_to_disk.h"

int store_qso(char *loglineptr)
{
	FILE *fp;

	if  ( (fp = fopen(logfile,"a"))  == NULL){
		fprintf(stdout,  "store_qso.c: Error opening file.\n");
		endwin();
		exit(1);
	}

	if (waedc_flg == 1 && qtclist.totalsent > 0) {
	    log_sent_qtc_to_disk((nr_qsos+1));
	}
	if (waedc_flg == 1 && qtcreclist.count > 0) {
	    log_recv_qtc_to_disk((nr_qsos+1));
	}
	strcpy(qsos[nr_qsos], loglineptr);
	nr_qsos++;
	strcat(loglineptr, "\n");	// pa3fwm, 20040113: this looks suspicious, repeated calls to store_qso() could add multiple \n's
	fputs  (loglineptr, fp);

	fclose(fp);

	return(0);
}

