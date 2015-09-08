/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
 * 		 2015		Thomas Beierlein <tb@forth-ev.de>
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
	 *    make sure QTC logfile is present and can be opened for append
	 *    - create one if it does not exist
	 *
	 *--------------------------------------------------------------*/

#include <stdio.h>
#include "tlf.h"
#include "startmsg.h"
#include "checkqtclogfile.h"
#include "qtcvars.h"

int checkqtclogfile()
{
    FILE *fp;

    /* check if logfile exist and can be opened for read */
    if (qtcdirection & SEND) {
	showstring( "Checking:", QTC_SENT_LOG);
	if ((fp = fopen(QTC_SENT_LOG, "r")) == NULL) {

	    if (errno == EACCES) {
		showmsg( "Can not access QTC log file");
		return 1;
	    }

	    if (errno == ENOENT) {
		/* File not found, create new one */
		showmsg( "Log file not found, creating new one");
		sleep(1);
		if ((fp = fopen(QTC_SENT_LOG, "w")) == NULL) {
		    /* cannot create logfile */
		    showmsg( "Creating QTC logfile not possible");
		    return 1;
		}
	    }
	}
	fclose(fp);
    }

    if (qtcdirection & RECV) {
	showstring( "Checking:", QTC_RECV_LOG);
	if ((fp = fopen(QTC_RECV_LOG, "r")) == NULL) {

	    if (errno == EACCES) {
		showmsg( "Can not access QTC log file");
		return 1;
	    }

	    if (errno == ENOENT) {
		/* File not found, create new one */
		showmsg( "Log file not found, creating new one");
		sleep(1);
		if ((fp = fopen(QTC_RECV_LOG, "w")) == NULL) {
		    /* cannot create logfile */
		    showmsg( "Creating QTC logfile not possible");
		    return 1;
		}
	    }
	}
	fclose(fp);
    }

    return 0;
}
