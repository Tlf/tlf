/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011, 2014     Thomas Beierlein <tb@forth-ev.de>
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


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "background_process.h"
#include "cqww_simulator.h"
#include "err_utils.h"
#include "fldigixmlrpc.h"
#include "get_time.h"
#include "gettxinfo.h"
#include "lancode.h"
#include "log_to_disk.h"
#include "qsonr_to_str.h"
#include "qtc_log.h"
#include "qtcutil.h"
#include "qtcvars.h"
#include "rtty.h"
#include "splitscreen.h"
#include "tlf.h"
#include "write_keyer.h"

// don't start until we know what we are doing
static bool stop_backgrnd_process = true;

static pthread_mutex_t stop_backgrnd_process_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t start_backgrnd_process_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t backgrnd_process_stopped_cond = PTHREAD_COND_INITIALIZER;

void stop_background_process(void) {
    pthread_mutex_lock(&stop_backgrnd_process_mutex);
    assert(!stop_backgrnd_process);
    stop_backgrnd_process = true;
    pthread_cond_wait(&backgrnd_process_stopped_cond, &stop_backgrnd_process_mutex);
    pthread_mutex_unlock(&stop_backgrnd_process_mutex);
}

void start_background_process(void) {
    pthread_mutex_lock(&stop_backgrnd_process_mutex);
    assert(stop_backgrnd_process);
    stop_backgrnd_process = false;
    pthread_cond_broadcast(&start_backgrnd_process_cond);
    pthread_mutex_unlock(&stop_backgrnd_process_mutex);
}

static void background_process_wait(void) {
    pthread_mutex_lock(&stop_backgrnd_process_mutex);
    if (stop_backgrnd_process) {
	pthread_cond_broadcast(&backgrnd_process_stopped_cond);
	pthread_cond_wait(&start_backgrnd_process_cond, &stop_backgrnd_process_mutex);
    }
    pthread_mutex_unlock(&stop_backgrnd_process_mutex);
}

void *background_process(void *ptr) {

    char *prmessage;
    static int lantimesync = 0;
    static int fldigi_rpc_cnt = 0;

    int n;

    char debugbuffer[160];
    FILE *fp;

    while (1) {

	background_process_wait();

	usleep(10000);

	if (packetinterface != 0) {
	    receive_packet();

	}

	if (trxmode == DIGIMODE && digikeyer != NO_KEYER)
	    rx_rtty();

	/*
	 * calling Fldigi XMLRPC method, which reads the Fldigi's carrier:
	 * fldigi_xmlrpc_get_carrier()
	 * this function helps to show the correct freq of the RIG: reads
	 * the carrier value from Fldigi, and stores in a variable; then
	 * it readable by fldigi_get_carrier()
	 * only need at every 2nd cycle
	 * see fldigixmlrpc.[ch]
	 *
	 * There are two addition routines
	 *   fldigi_get_log_call() reads the callsign, if user clicks to a string in Fldigi's RX window
	 *   fldigi_get_log_serial_number() reads the exchange
	 */
	if (digikeyer == FLDIGI && fldigi_isenabled() && trx_control) {
	    if (fldigi_rpc_cnt == 0) {
		fldigi_xmlrpc_get_carrier();
		fldigi_get_log_call();
		fldigi_get_log_serial_number();
	    }
	    fldigi_rpc_cnt = 1 - fldigi_rpc_cnt;
	}

	if (!stop_backgrnd_process) {
	    write_keyer();
	    cqww_simulator();
	}

	if (lan_active) {
	    if (lan_message[0] == '\0') {

		if (lan_recv() >= 0) {
		    lan_message[strlen(lan_message) - 1] = '\0';
		}
	    }

	    if (landebug == 1 && strlen(lan_message) > 2) {
		if ((fp = fopen("debuglog", "a")) == NULL) {
		    printf("store_qso.c: Error opening debug file.\n");
		} else {
		    format_time(debugbuffer, sizeof(debugbuffer), "%H:%M:%S-");
		    fputs(debugbuffer, fp);
		    fputs(lan_message, fp);
		    fputs("\n", fp);
		    fclose(fp);
		}
	    }
	    if ((*lan_message != '\0') && (lan_message[0] == thisnode)) {
		TLF_LOG_WARN("%s", "Warning: NODE ID CONFLICT ?! You should use another ID! ");
	    }

	    if ((*lan_message != '\0')
		    && (lan_message[0] != thisnode)
		    && !stop_backgrnd_process) {

		switch (lan_message[1]) {

		    case LOGENTRY:

			log_to_disk(true);
			break;

		    case QTCRENTRY:

			store_qtc(lan_message + 2, RECV, QTC_RECV_LOG);
			break;

		    case QTCSENTRY:

			store_qtc(lan_message + 2, SEND, QTC_SENT_LOG);
			break;

		    case QTCFLAG:

			parse_qtc_flagline(lan_message + 2);
			break;

		    case CLUSTERMSG:
			prmessage = g_strndup(lan_message + 2, 80);
			if (strstr(prmessage, my.call) != NULL) {	// alert for cluster messages
			    TLF_LOG_INFO(prmessage);
			}

			addtext(prmessage);
			g_free(prmessage);
			break;
		    case TLFSPOT:
			prmessage = g_strndup(lan_message + 2, 80);
			lanspotflg = true;
			addtext(prmessage);
			lanspotflg = false;
			g_free(prmessage);
			break;
		    case TLFMSG:
			for (int t = 0; t < 4; t++)
			    strcpy(talkarray[t], talkarray[t + 1]);

			talkarray[4][0] = lan_message[0];
			talkarray[4][1] = ':';
			talkarray[4][2] = '\0';
			g_strlcat(talkarray[4], lan_message + 2,
				  sizeof(talkarray[4]));
			TLF_LOG_INFO(" MSG from %s", talkarray[4]);
			break;
		    case FREQMSG:
			if ((lan_message[0] >= 'A')
				&& (lan_message[0] <= 'A' + MAXNODES)) {
			    node_frequencies[lan_message[0] - 'A'] =
				atof(lan_message + 2) * 1000.0;
			    break;
			}
		    case INCQSONUM:

			n = atoi(lan_message + 2);

			if (highqsonr < n)
			    highqsonr = n;

			if ((qsonum <= n) && (n > 0)) {
			    qsonum = highqsonr + 1;
			    qsonr_to_str();
			}
			lan_message[0] = '\0';

		    case TIMESYNC:
			if ((lan_message[0] >= 'A')
				&& (lan_message[0] <= 'A' + MAXNODES)) {
			    time_t lantime = atoi(lan_message + 2);

			    time_t delta = lantime - (get_time() - timecorr);

			    if (lantimesync == 1) {
				timecorr = (4 * timecorr + delta) / 5;
			    } else {
				timecorr = delta;
				lantimesync = 1;
			    }

			    break;
			}
		}

		lan_message[0] = '\0';
		lan_message[1] = '\0';

	    }

	}

	gettxinfo();		/* get freq info from TRX */

    }

}

