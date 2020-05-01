/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011, 2014, 2016 Thomas Beierlein <tb@forth-ev.de>
 *               2014, 2016       Ervin Hegedus <airween@gmail.com>
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

#include <ctype.h>
#include <glib.h>
#include <pthread.h>
#include <stdlib.h>	// need for abs()
#include <string.h>
#include <stdarg.h>	// need for va_list...
#include <unistd.h>

#include <config.h>

#ifdef HAVE_LIBXMLRPC
# include <xmlrpc-c/base.h>
# include <xmlrpc-c/client.h>
# include <xmlrpc-c/client_global.h>
#endif

#include <hamlib/rig.h>

#include "err_utils.h"
#include "fldigixmlrpc.h"
#include "getctydata.h"
#include "logit.h"
#include "printcall.h"
#include "searchlog.h"
#include "startmsg.h"
#include "tlf_curses.h"
#include "ui_utils.h"

#define XMLRPCVERSION "1.0"

int fldigi_set_callfield = 0;

typedef struct xmlrpc_res_s {
    int			intval;
    const char		*stringval;
    const unsigned char	*byteval;
} xmlrpc_res;

#define CENTER_FREQ 2210	/* low: 2125Hz, high: 2295Hz, shift: 170Hz,
				    center: 2125+(170/2) = 2210Hz */
#define MAXSHIFT 20		/* max shift value in Fldigi, when Tlf set
				   it back to RIG carrier */

extern char fldigi_url[50];
static int use_fldigi;

int fldigi_var_carrier = 0;
int fldigi_var_shift_freq = 0;
#ifdef HAVE_LIBXMLRPC
static int initialized = 0;
#endif
static int connerr = 0;

char thiscall[20] = "";
char tcomment[20] = "";

pthread_mutex_t xmlrpc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t xmlrpc_get_rx_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Used XML RPC methods, and its formats of arguments
 * ==================================================
 main.rx                n:n  - RX
 main.tx                n:n  - TX
 main.get_trx_state     s:n  - get RX/TX state, 's' could be "RX" | "TX"
   rx.get_data          6:n (bytes:) - get content of RX window since last query
 text.add_tx            n:s  - add content to TX window
modem.get_name          s:n  - Returns the name of the current modem
modem.get_carrier       i:n  - get carrier of modem
modem.set_carrier       i:i  - set carrier of modem
  log.get_call          s:n  - Returns the Call field contents
  log.set_call          n:s  - Sets the Call field contents
  log.get_serial_number s:n  - Returns the serial number field contents
  rig.set_frequency     d:d  - Sets the RF carrier frequency. Returns the old value
  log.get_exchange      s:n  - Returns the contest exchange field contents
  log.set_exchange      n:s  - Sets the contest exchange field contents
  rig.set_modes         n:A  - Sets the list of available rig modes
  rig.set_mode          n:s  - Selects a mode previously added by rig.set_modes


 // other usable functions
 text.get_rx_length  i:n  - get length of content of RX window
 text.get_rx         6:ii - (bytes:int|int) - get part content of RX window
			    [start:length]
   tx.get_data       6:n  - (bytes:) - get content of TX window since last query
*/

#ifdef HAVE_LIBXMLRPC
xmlrpc_env env;
xmlrpc_server_info *serverInfoP = NULL;
#endif

void fldigi_clear_connerr() {
    pthread_mutex_lock(&xmlrpc_mutex);
    connerr = 0;
    pthread_mutex_unlock(&xmlrpc_mutex);
}

int fldigi_toggle(void) {
    int ret;

    pthread_mutex_lock(&xmlrpc_mutex);
    use_fldigi = !use_fldigi;
    ret = use_fldigi;
    pthread_mutex_unlock(&xmlrpc_mutex);
    return ret;
}

int fldigi_isenabled(void) {
    int ret;

    pthread_mutex_lock(&xmlrpc_mutex);
    ret = use_fldigi;
    pthread_mutex_unlock(&xmlrpc_mutex);
    return ret;
}

void xmlrpc_res_init(xmlrpc_res *res) {
#ifdef HAVE_LIBXMLRPC
    res->stringval = NULL;
    res->byteval = NULL;
#endif
}


int fldigi_xmlrpc_init() {
#ifdef HAVE_LIBXMLRPC
    pthread_mutex_lock(&xmlrpc_mutex);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, PACKAGE_NAME,
			XMLRPCVERSION, NULL, 0);
    serverInfoP = xmlrpc_server_info_new(&env, fldigi_url);
    if (env.fault_occurred != 0) {
	serverInfoP = NULL;
	initialized = 0;
	pthread_mutex_unlock(&xmlrpc_mutex);
	return -1;
    }

    initialized = 1;
    pthread_mutex_unlock(&xmlrpc_mutex);
#endif
    return 0;
}

int fldigi_xmlrpc_cleanup() {
#ifdef HAVE_LIBXMLRPC
    pthread_mutex_lock(&xmlrpc_mutex);
    if (serverInfoP != NULL) {
	xmlrpc_server_info_free(serverInfoP);
	serverInfoP = NULL;
	initialized = 0;
    }
    pthread_mutex_unlock(&xmlrpc_mutex);
#endif
    return 0;
}

#ifdef HAVE_LIBXMLRPC
int fldigi_xmlrpc_query(xmlrpc_res *local_result, xmlrpc_env *local_env,
			char *methodname, char *format, ...) {

    static unsigned int connerrcnt = 0;
    xmlrpc_value *callresult;
    xmlrpc_value *pcall_array = NULL;
    xmlrpc_value *va_param = NULL;
    va_list argptr;
    int restype;
    size_t bytesize = 0;
    int ret;


    pthread_mutex_lock(&xmlrpc_mutex);

    if (initialized == 0) {
	pthread_mutex_unlock(&xmlrpc_mutex);
	return -1;
    }
    /*
    if connerr had been set up to 1, that means an error
    occured at last xmlrpc_call() method
    if that true, then we count the number of calling this
    function (xmlrpc()), if counter reaches 10, then clear
    it, and try again
    this handles the xmlrpc_call() errors, eg. Fldigi is
    unreacheable, but it will check again and again, not
    need to restart Tlf, or type ":FLDIGI" to turn on again
    */
    if (connerr == 1 && use_fldigi == 1) {
	if (connerrcnt == 10) {
	    use_fldigi = 0;
	    TLF_LOG_WARN("Fldigi: lost connection!");
	} else {
	    connerrcnt++;
	}
    } else {
	connerrcnt = 0;
    }

    local_result->stringval = NULL;
    local_result->byteval = NULL;
    if (connerr == 0 && use_fldigi == 1) {
	va_start(argptr, format);
	xmlrpc_env_init(local_env);
	pcall_array = xmlrpc_array_new(local_env);
	while (*format != '\0') {
	    if (*format == 's') {
		char *s = va_arg(argptr, char *);
		va_param = xmlrpc_string_new(local_env, s);
		if (local_env->fault_occurred) {
		    va_end(argptr);
		    connerr = 1;
		    pthread_mutex_unlock(&xmlrpc_mutex);
		    return -1;
		}
		xmlrpc_array_append_item(local_env, pcall_array, va_param);
		if (local_env->fault_occurred) {
		    va_end(argptr);
		    connerr = 1;
		    pthread_mutex_unlock(&xmlrpc_mutex);
		    return -1;
		}
		xmlrpc_DECREF(va_param);
	    } else if (*format == 'd') {
		int d = va_arg(argptr, int);
		va_param = xmlrpc_int_new(local_env, d);
		xmlrpc_array_append_item(local_env, pcall_array, va_param);
		if (local_env->fault_occurred) {
		    va_end(argptr);
		    connerr = 1;
		    pthread_mutex_unlock(&xmlrpc_mutex);
		    return -1;
		}
		xmlrpc_DECREF(va_param);
	    } else if (*format == 'f') {
		double f = va_arg(argptr, double);
		va_param = xmlrpc_double_new(local_env, f);
		xmlrpc_array_append_item(local_env, pcall_array, va_param);
		if (local_env->fault_occurred) {
		    va_end(argptr);
		    connerr = 1;
		    pthread_mutex_unlock(&xmlrpc_mutex);
		    return -1;
		}
		xmlrpc_DECREF(va_param);
	    }
	    format++;
	}

	va_end(argptr);

	callresult = xmlrpc_client_call_server_params(local_env, serverInfoP,
		     methodname, pcall_array);
	if (local_env->fault_occurred) {
	    // error till xmlrpc_call
	    connerr = 1;
	    xmlrpc_DECREF(pcall_array);
	    xmlrpc_env_clean(local_env);
	    pthread_mutex_unlock(&xmlrpc_mutex);
	    return -1;
	} else {
	    restype = xmlrpc_value_type(callresult);
	    if (restype == 0xDEAD) {
		xmlrpc_DECREF(callresult);
		xmlrpc_DECREF(pcall_array);
		xmlrpc_env_clean(local_env);
		pthread_mutex_unlock(&xmlrpc_mutex);
		return -1;
	    } else {
		local_result->intval = 0;
	    }
	    switch (restype) {
		// int
		case XMLRPC_TYPE_INT:
		    xmlrpc_read_int(local_env, callresult,
				    &local_result->intval);
		    break;
		// string
		case XMLRPC_TYPE_STRING:
		    xmlrpc_read_string(local_env, callresult,
				       &local_result->stringval);
		    break;
		// byte stream
		case XMLRPC_TYPE_BASE64:
		    xmlrpc_read_base64(local_env, callresult,
				       &bytesize, &local_result->byteval);
		    local_result->intval = (int)bytesize;
		    break;
	    }
	    xmlrpc_DECREF(callresult);
	}

	xmlrpc_DECREF(pcall_array);
    }
    if (connerr == 0 && use_fldigi)
	ret = 0;
    else
	ret = -1;
    pthread_mutex_unlock(&xmlrpc_mutex);
    return ret;
}
#endif

/* command fldigi to RX now */
void fldigi_to_rx() {
#ifdef HAVE_LIBXMLRPC
    xmlrpc_res result;
    xmlrpc_env env;
    fldigi_xmlrpc_query(&result, &env, "main.rx", "");
    if (result.stringval != NULL) {
	free((void *)result.stringval);
    }
#endif
}

/* send message to Fldigi TX window, transmit it */
int fldigi_send_text(char *line) {
    int rc = 0;

#ifdef HAVE_LIBXMLRPC
    xmlrpc_res result;
    xmlrpc_env env;

    // check the RX/TX status
    rc = fldigi_xmlrpc_query(&result, &env, "main.get_trx_state", "");
    if (rc != 0) {
	return -1;
    }

    // if state is TX, stop it
    // if the RX success, clear the previous message from TX text window
    if (strcmp(result.stringval, "TX") == 0) {
	free((void *)result.stringval);
	result.stringval = NULL;
	rc = fldigi_xmlrpc_query(&result, &env, "main.rx", "");
	if (rc != 0) {
	    return -1;
	}
	rc = fldigi_xmlrpc_query(&result, &env, "text.clear_tx", "");
	if (rc != 0) {
	    return -1;
	}
	sleep(2);
    }
    if (result.stringval != NULL) {
	free((void *)result.stringval);
    }

    // add message to
    rc = fldigi_xmlrpc_query(&result, &env, "text.add_tx", "s", line);
    if (rc != 0) {
	return -1;
    }
    /* switch to rx afterwards */
    rc = fldigi_xmlrpc_query(&result, &env, "text.add_tx", "s", "^r");
    if (rc != 0) {
	return -1;
    }

    if (result.stringval != NULL) {
	free((void *)result.stringval);
    }
    /* switch to tx */
    rc = fldigi_xmlrpc_query(&result, &env, "main.tx", "");

    if (result.stringval != NULL) {
	free((void *)result.stringval);
    }
#endif
    return rc;
}

/* read the text from Fldigi's RX window, from last read position */
/*
 * Since this uses a static variable, it is not thread-safe and must
 * use a protective mutex.
 */
int fldigi_get_rx_text(char *line, int len) {
#ifdef HAVE_LIBXMLRPC
    int rc;
    xmlrpc_res result;
    xmlrpc_env env;
    static int lastpos = 0;
    int textlen = 0;
    int retval = 0;
    int linelen = 0;

    pthread_mutex_lock(&xmlrpc_get_rx_mutex);
    rc = fldigi_xmlrpc_query(&result, &env, "text.get_rx_length", "");
    if (rc != 0) {
	pthread_mutex_unlock(&xmlrpc_get_rx_mutex);
	return -1;
    }

    textlen = result.intval;
    if (lastpos == 0) {
	lastpos = textlen;
    } else {
	if (lastpos < textlen) {
	    rc = fldigi_xmlrpc_query(&result, &env, "text.get_rx", "dd",
				     lastpos,
				     textlen - lastpos >= len ?
				     len - 1 : textlen - lastpos);
	    if (rc != 0) {
		pthread_mutex_unlock(&xmlrpc_get_rx_mutex);
		return -1;
	    }

	    if (result.intval > 0 && result.byteval != NULL) {
		linelen = result.intval;
		if (result.intval >= len) {
		    linelen = len - 1;
		}
		memcpy(line, result.byteval, linelen);
		line[linelen] = '\0';
		retval = linelen;
	    }
	    if (result.byteval != NULL) {
		free((void *)result.byteval);
	    }
	}
    }
    lastpos = textlen;
    pthread_mutex_unlock(&xmlrpc_get_rx_mutex);
    return retval;

#else
    return 0;
#endif

}

/* get the carrier value of Fldigi waterfall window */
int fldigi_xmlrpc_get_carrier() {
#ifdef HAVE_LIBXMLRPC
    int rc;
    xmlrpc_res result;
    xmlrpc_env env;
    extern rmode_t rigmode;
    extern int trx_control;
    extern freq_t freq;
    char fldigi_mode[6] = "";

    rc = fldigi_xmlrpc_query(&result, &env, "modem.get_carrier", "");
    if (rc != 0) {
	return -1;
    }

    fldigi_var_carrier = (int)result.intval;

    if (!trx_control || rigmode == RIG_MODE_NONE) {
	return 0;
    }

    /* if mode == RTTY(R), and Hamlib configured, set VFO to new freq where the signal
     * will placed on 2210 Hz - the FSK center freq
     */

    if (rigmode == RIG_MODE_RTTY || rigmode == RIG_MODE_RTTYR) {
	if (fldigi_var_carrier != CENTER_FREQ &&
		abs(CENTER_FREQ - fldigi_var_carrier) > MAXSHIFT) {
	    if (fldigi_var_shift_freq == 0) {
		rc = fldigi_xmlrpc_query(&result, &env,
					 "modem.set_carrier", "d",
					 (xmlrpc_int32) CENTER_FREQ);
		if (rc != 0) {
		    return -1;
		}
		fldigi_var_shift_freq = CENTER_FREQ - fldigi_var_carrier;
	    }
	}
    }

    // determine mode shift (currently for plain RTTY only)
    int modeshift = 0;
    int signum = 0;
    rc = fldigi_xmlrpc_query(&result, &env, "modem.get_name", "");
    if (rc != 0) {
	return -1;
    }

    if (strcmp(result.stringval, "RTTY") == 0) {
	modeshift = 170 / 2;
    }
    free((void *)result.stringval);

    switch (rigmode) {
	case RIG_MODE_USB:
	    signum = 1;
	    strcpy(fldigi_mode, "USB");
	    break;
	case RIG_MODE_LSB:
	    signum = -1;
	    strcpy(fldigi_mode, "LSB");
	    break;
	case RIG_MODE_RTTY:
	    signum = 0;
	    modeshift = 0;
	    strcpy(fldigi_mode, "RTTY");
	    break;
	case RIG_MODE_RTTYR:
	    signum = 0;		// not checked - I don't have RTTY-REV mode on my RIG
	    modeshift = 0;
	    strcpy(fldigi_mode, "RTTYR");
	    break;
	case RIG_MODE_CW:
	    signum = 0;
	    modeshift = 0;
	    strcpy(fldigi_mode, "CW");
	    break;
	case RIG_MODE_CWR:
	    signum = -1;	// not checked - I don't have CW-REV mode on my RIG
	    modeshift = 0;
	    strcpy(fldigi_mode, "CWR");
	    break;
	default:
	    signum = 0;	// this is the "normal"
	    modeshift = 0;
	    strcpy(fldigi_mode, "CW");
    }

    /* set the mode in Fldigi */
    rc = fldigi_xmlrpc_query(&result, &env, "rig.set_mode", "s", fldigi_mode);
    if (rc != 0) {
	return -1;
    }
    fldigi_var_carrier = signum * fldigi_var_carrier + modeshift;

    /* also set the freq value in Fldigi FREQ block */
    rc = fldigi_xmlrpc_query(&result, &env,
			     "rig.set_frequency", "f",
			     (xmlrpc_double)(freq - fldigi_var_carrier));
    if (rc != 0) {
	return -1;
    }
#endif
    return 0;
}

/* give back the current carrier value, which stored in variable */
int fldigi_get_carrier() {
#ifdef HAVE_LIBXMLRPC
    return fldigi_var_carrier;
#else
    return 0;
#endif
}

/* read callsign field in Fldigi, and sets the CALL in Tlf */
int fldigi_get_log_call() {
#ifdef HAVE_LIBXMLRPC
    int rc;
    xmlrpc_res result;
    xmlrpc_env env;

    xmlrpc_res_init(&result);

    extern char hiscall[];
    char tempstr[20];
    int i, j;


    rc = fldigi_xmlrpc_query(&result, &env, "log.get_call", "");
    if (rc != 0) {
	return -1;
    } else {
	if (result.stringval != NULL) {
	    j = 0;
	    // accept only alphanumeric chars and '/' in callsign
	    // in case of QRM, there are many several metachar
	    for (i = 0; i < 20 && result.stringval[i] != '\0'; i++) {
		if (isalnum(result.stringval[i]) || result.stringval[i] == '/') {
		    tempstr[j++] = result.stringval[i];
		}
	    }
	    tempstr[j] = '\0';

	    // check the current call in Tlf; if the previous local callsign isn't empty,
	    // that means the OP clean up the callsign field, so it needs to clean in Fldigi too
	    if (hiscall[0] == '\0' && thiscall[0] != '\0') {
		thiscall[0] = '\0';
		rc = fldigi_xmlrpc_query(&result, &env, "log.set_call", "s", "");
		if (rc != 0) {
		    return -1;
		}
	    }
	    // otherways, fill the callsign field in Tlf
	    else {
		if (strlen(tempstr) >= 3) {
		    if (hiscall[0] == '\0') {
			strcpy(hiscall, tempstr);
			hiscall[strlen(tempstr)] = '\0';
			strcpy(thiscall, hiscall);
			printcall();
			getctydata_pfx(hiscall);
			searchlog();
			fldigi_set_callfield = 1;
		    }
		}
	    }
	}
	free((void *)result.stringval);
	if (result.byteval != NULL) {
	    free((void *)result.byteval);
	}
    }
#endif
    return 0;
}

/* read exchange field in Fldigi, and sets that in Tlf */
int fldigi_get_log_serial_number() {
#ifdef HAVE_LIBXMLRPC
    int rc;
    xmlrpc_res result;
    xmlrpc_env env;

    xmlrpc_res_init(&result);

    extern char comment[];
    char tempstr[20];
    int i, j;

    rc = fldigi_xmlrpc_query(&result, &env, "log.get_exchange", "");
    if (rc != 0) {
	return -1;
    } else {
	if (result.stringval != NULL) {
	    j = 0;
	    // accept only alphanumeric chars
	    for (i = 0; i < 20 && result.stringval[i] != '\0'; i++) {
		if (isalnum(result.stringval[i])) {
		    tempstr[j++] = result.stringval[i];
		}
	    }
	    tempstr[j] = '\0';

	    // if the previous exchange isn't empty, but the current value is it,
	    // that means the OP cleaned up the field, so we need to clean up it in Fldigi
	    if (comment[0] == '\0' && tcomment[0] != '\0') {
		tcomment[0] = '\0';
		rc = fldigi_xmlrpc_query(&result, &env, "log.set_exchange", "s", "");
		if (rc != 0) {
		    return -1;
		}
	    }
	    // otherways we need to fill the Tlf exchange field
	    else {
		if (strlen(tempstr) > 0 && comment[0] == '\0') {
		    strcpy(comment, tempstr);
		    comment[strlen(tempstr)] = '\0';
		    strcpy(tcomment, comment);
		    refresh_comment();
		}
	    }
	}
	free((void *)result.stringval);
	if (result.stringval != NULL) {
	    free((void *)result.byteval);
	}
    }
#endif
    return 0;
}

int fldigi_get_shift_freq() {
#ifdef HAVE_LIBXMLRPC
    int t;				/* temp var to store real variable
					   before cleaning it up */
    t = fldigi_var_shift_freq;	/* clean is necessary to check that
					   it readed by called this function */
    fldigi_var_shift_freq = 0;	/* needs to keep in sync with the
					   rig VFO */
    return t;
#else
    return 0;
#endif
}

void xmlrpc_showinfo() {
#ifdef HAVE_LIBXMLRPC		// Show xmlrpc status
    showmsg("XMLRPC compiled in");
#else
    showmsg("XMLRPC NOT compiled");
#endif
}
