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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include <assert.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>

#include "background_process.h"
#include "cqww_simulator.h"
#include "cluster.h"
#include "err_utils.h"
#include "fldigixmlrpc.h"
#include "globalvars.h"
#include "get_time.h"
#include "gettxinfo.h"
#include "lancode.h"
#include "rtty.h"
#include "tlf.h"
#include "write_keyer.h"

static bool go = true;
// don't start until we know what we are doing
static bool pause_backgrnd_process = true;

static pthread_mutex_t pause_backgrnd_process_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t resume_backgrnd_process_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t backgrnd_process_paused_cond = PTHREAD_COND_INITIALIZER;

void terminate_background_process() {
    go = false;
}

void pause_background_process(void) {
    pthread_mutex_lock(&pause_backgrnd_process_mutex);
    assert(!pause_backgrnd_process);
    pause_backgrnd_process = true;
    pthread_cond_wait(&backgrnd_process_paused_cond, &pause_backgrnd_process_mutex);
    pthread_mutex_unlock(&pause_backgrnd_process_mutex);
}

void resume_background_process(void) {
    pthread_mutex_lock(&pause_backgrnd_process_mutex);
    assert(pause_backgrnd_process);
    pause_backgrnd_process = false;
    pthread_cond_broadcast(&resume_backgrnd_process_cond);
    pthread_mutex_unlock(&pause_backgrnd_process_mutex);
}

static void background_process_wait(void) {
    pthread_mutex_lock(&pause_backgrnd_process_mutex);
    if (pause_backgrnd_process) {
	pthread_cond_broadcast(&backgrnd_process_paused_cond);
	pthread_cond_wait(&resume_backgrnd_process_cond, &pause_backgrnd_process_mutex);
    }
    pthread_mutex_unlock(&pause_backgrnd_process_mutex);
}


void *background_process(void *ptr) {

    static int fldigi_rpc_cnt = 0;

    while (go) {

	background_process_wait();

	usleep(10000);

	receive_packet();

	if (trxmode == DIGIMODE && digikeyer != NO_KEYER)
	    rx_rtty();

	/*
	 *   fldigi_get_log_call() reads the callsign, if user clicks to a string in Fldigi's RX window
	 *   fldigi_get_log_serial_number() reads the exchange
	 */
	if (digikeyer == FLDIGI && fldigi_isenabled() && trx_control) {
	    if (fldigi_rpc_cnt == 0) {
		fldigi_auto_qsy();
		fldigi_get_log_call();
		fldigi_get_log_serial_number();
	    }
	    fldigi_rpc_cnt = (fldigi_rpc_cnt + 1) % 10;
	}

	if (!pause_backgrnd_process) {
	    write_keyer();
	    cqww_simulator();
	}

	receive_lan();

	gettxinfo();		/* get freq info from TRX */

    }

    pthread_exit(NULL);
}

