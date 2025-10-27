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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <ctype.h>
#include <glib.h>
#include <pthread.h>
#include <stdlib.h>	// need for abs()
#include <string.h>
#include <stdarg.h>	// need for va_list...
#include <unistd.h>
#include <math.h>       // for round()

#include <config.h>

#ifdef HAVE_LIBXMLRPC
# include <xmlrpc-c/base.h>
# include <xmlrpc-c/client.h>
# include <xmlrpc-c/client_global.h>
#endif

#include "err_utils.h"
#include "fldigixmlrpc.h"
#include "globalvars.h"
#include "keystroke_names.h"
#include "logit.h"
#include "callinput.h"      // for valid_call_char()
#include "showmsg.h"
#include "tlf_curses.h"
#include "utils.h"

bool fldigi_set_callfield = false;

#ifdef HAVE_LIBXMLRPC
typedef struct {
    int			intval;
    xmlrpc_double       doubleval;
    const char		*stringval;
    const unsigned char	*byteval;
} xmlrpc_res;
#endif

#define CENTER_FREQ 2210	/* low: 2125Hz, high: 2295Hz, shift: 170Hz,
				    center: 2125+(170/2) = 2210Hz */
#define MAXSHIFT 20		/* max shift value in Fldigi, when Tlf set
				   it back to RIG carrier */


#ifdef HAVE_LIBXMLRPC
static bool use_fldigi = false;

static int fldigi_var_carrier = 0;
static int fldigi_var_shift_freq = 0;
static bool initialized = false;
static bool connerr = false;

static bool call_set = false;
static bool comment_set = false;

static pthread_mutex_t xmlrpc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xmlrpc_get_rx_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Used Fldigi XML-RPC methods
 * ==================================================
Method Name 	 Sig (ret:arg) Description
-----------------------------------------------------
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


 * XML-RPC Format Specifiers (subset used by Fldigi)
 * =================================================
    i   32 bit integer
    d   double precision floating point number
    s   string
    6   base64-encoded byte string
    n   nil
    A   array
*/

#ifdef HAVE_LIBXMLRPC
static xmlrpc_server_info *serverInfoP = NULL;
static xmlrpc_client *clientP = NULL;
#endif

bool fldigi_toggle(void) {
    bool ret = false;

#ifdef HAVE_LIBXMLRPC
    pthread_mutex_lock(&xmlrpc_mutex);
    use_fldigi = !use_fldigi;
    ret = use_fldigi;
    connerr = false;
    pthread_mutex_unlock(&xmlrpc_mutex);
#endif

    return ret;
}

bool fldigi_isenabled(void) {
    bool ret = false;

#ifdef HAVE_LIBXMLRPC
    pthread_mutex_lock(&xmlrpc_mutex);
    ret = use_fldigi;
    pthread_mutex_unlock(&xmlrpc_mutex);
#endif

    return ret;
}

#ifdef HAVE_LIBXMLRPC
static void xmlrpc_res_init(xmlrpc_res *res) {
    res->intval = 0;
    res->doubleval = 0.0;
    res->stringval = NULL;
    res->byteval = NULL;
}

static void xmlrpc_res_free(xmlrpc_res *res) {
    if (res->byteval != NULL) {
	free((void *)res->byteval);
    }
    res->byteval = NULL;

    if (res->stringval != NULL) {
	free((void *)res->stringval);
    }
    res->stringval = NULL;
}

static void xmlrpc_release() {
    if (clientP != NULL) {
	xmlrpc_client_destroy(clientP);
	clientP = NULL;
    }
    if (serverInfoP != NULL) {
	xmlrpc_server_info_free(serverInfoP);
	serverInfoP = NULL;
    }
    initialized = false;
}

// set up local xmlrpc client
// env contains error information
static void fldigi_xmlrpc_setup(xmlrpc_env *env) {
    xmlrpc_client_setup_global_const(env);
    if (env->fault_occurred) {
	return;
    }

    clientP = NULL;
    xmlrpc_client_create(env, XMLRPC_CLIENT_NO_FLAGS,
			 PACKAGE_NAME, PACKAGE_VERSION, NULL, 0, &clientP);
    if (env->fault_occurred) {
	return;
    }

    serverInfoP = xmlrpc_server_info_new(env, fldigi_url);
    if (env->fault_occurred) {
	return;
    }
}
#endif

int fldigi_xmlrpc_init() {
    int rc = 0;
#ifdef HAVE_LIBXMLRPC
    pthread_mutex_lock(&xmlrpc_mutex);

    xmlrpc_env env;
    xmlrpc_env_init(&env);

    fldigi_xmlrpc_setup(&env);
    if (env.fault_occurred) {
	xmlrpc_release();
	rc = -1;
    } else {
	initialized = true;
    }

    xmlrpc_env_clean(&env);

    pthread_mutex_unlock(&xmlrpc_mutex);
#endif
    return rc;
}

int fldigi_xmlrpc_cleanup() {
#ifdef HAVE_LIBXMLRPC
    pthread_mutex_lock(&xmlrpc_mutex);
    xmlrpc_release();
    pthread_mutex_unlock(&xmlrpc_mutex);
#endif
    return 0;
}

#ifdef HAVE_LIBXMLRPC

// build parameter array for xmlrpc_client_call_server_params()
// returns: the array to be freed by the caller
//      on error: NULL
static xmlrpc_value *build_param_array(xmlrpc_env *local_env,
				       char *format, va_list argptr) {

    xmlrpc_value *pcall_array = xmlrpc_array_new(local_env);

    xmlrpc_value *va_param = NULL;

    while (*format != '\0') {
	if (*format == 's') {
	    char *s = va_arg(argptr, char *);
	    va_param = xmlrpc_string_new(local_env, s);
	    if (local_env->fault_occurred) {
		break;
	    }
	    xmlrpc_array_append_item(local_env, pcall_array, va_param);
	} else if (*format == 'i') {
	    int d = va_arg(argptr, int);
	    va_param = xmlrpc_int_new(local_env, d);
	    xmlrpc_array_append_item(local_env, pcall_array, va_param);
	} else if (*format == 'd') {
	    double f = va_arg(argptr, double);
	    va_param = xmlrpc_double_new(local_env, f);
	    xmlrpc_array_append_item(local_env, pcall_array, va_param);
	}

	if (local_env->fault_occurred) {
	    break;
	}

	xmlrpc_DECREF(va_param);
	va_param = NULL;

	format++;
    }

    if (local_env->fault_occurred) {
	if (va_param != NULL) {
	    xmlrpc_DECREF(va_param);
	}
	xmlrpc_DECREF(pcall_array);
	return NULL;
    }

    return pcall_array;
}

static int parse_call_result(xmlrpc_env *local_env, xmlrpc_value *callresult,
			     xmlrpc_res *result) {

    int restype = xmlrpc_value_type(callresult);
    if (restype == XMLRPC_TYPE_DEAD) {
	return -1;
    }

    if (result == NULL) {   // we are not interested in the result...
	return 0;
    }

    size_t bytesize;

    switch (restype) {
	// int
	case XMLRPC_TYPE_INT:
	    xmlrpc_read_int(local_env, callresult,
			    &result->intval);
	    break;
	// double
	case XMLRPC_TYPE_DOUBLE:
	    xmlrpc_read_double(local_env, callresult,
			       &result->doubleval);
	    break;
	// string
	case XMLRPC_TYPE_STRING:
	    xmlrpc_read_string(local_env, callresult,
			       &result->stringval);
	    break;
	// byte stream
	case XMLRPC_TYPE_BASE64:
	    xmlrpc_read_base64(local_env, callresult,
			       &bytesize, &result->byteval);
	    result->intval = (int)bytesize;
	    break;
    }

    return 0;
}

// call a remote Fldigi method
// result has to be passed uninitalized and then freed by the caller
// alternatively, a NULL result can be passed for a void method
// returns: 0
//      on error: -1
static int fldigi_xmlrpc_query(xmlrpc_res *result, char *methodname,
			       char *format, ...) {

    static unsigned int connerrcnt = 0;

    pthread_mutex_lock(&xmlrpc_mutex);

    if (!initialized) {
	pthread_mutex_unlock(&xmlrpc_mutex);
	return -1;
    }
    /*
    if connerr had been set up to 1, that means an error
    occurred at last xmlrpc_call() method
    if that true, then we count the number of calling this
    function (xmlrpc()), if counter reaches 10, then clear
    it, and try again
    this handles the xmlrpc_call() errors, eg. Fldigi is
    unreachable, but it will check again and again, not
    need to restart Tlf, or type ":FLDIGI" to turn on again
    */
    if (connerr && use_fldigi) {
	if (connerrcnt == 10) {
	    use_fldigi = false;
	    TLF_SHOW_WARN("Fldigi: lost connection!");
	} else {
	    connerrcnt++;
	}
    } else {
	connerrcnt = 0;
    }

    if (!use_fldigi || connerr) {
	pthread_mutex_unlock(&xmlrpc_mutex);
	return -1;
    }

    if (result != NULL) {
	xmlrpc_res_init(result);
    }

    xmlrpc_env local_env;
    xmlrpc_env_init(&local_env);

    va_list argptr;
    va_start(argptr, format);
    xmlrpc_value *pcall_array = build_param_array(&local_env, format, argptr);
    va_end(argptr);

    if (pcall_array == NULL) {
	connerr = true;
	xmlrpc_env_clean(&local_env);
	pthread_mutex_unlock(&xmlrpc_mutex);
	return -1;
    }

    xmlrpc_value *callresult = NULL;
    xmlrpc_client_call2(&local_env, clientP, serverInfoP,
			methodname, pcall_array, &callresult);

    if (local_env.fault_occurred) {
	connerr = true;
	if (callresult != NULL) {
	    xmlrpc_DECREF(callresult);
	}
	xmlrpc_DECREF(pcall_array);
	xmlrpc_env_clean(&local_env);
	pthread_mutex_unlock(&xmlrpc_mutex);
	return -1;
    }

    int rc = parse_call_result(&local_env, callresult, result);

    xmlrpc_DECREF(callresult);
    xmlrpc_DECREF(pcall_array);
    xmlrpc_env_clean(&local_env);

    pthread_mutex_unlock(&xmlrpc_mutex);
    return rc;
}
#endif

/* command fldigi to RX now */
void fldigi_to_rx() {
#ifdef HAVE_LIBXMLRPC
    fldigi_xmlrpc_query(NULL, "main.rx", "");
    fldigi_xmlrpc_query(NULL, "text.clear_tx", "");
#endif
}

#ifdef HAVE_LIBXMLRPC
static int cancel_tx() {
    xmlrpc_res result;

    // check the RX/TX status
    int rc = fldigi_xmlrpc_query(&result, "main.get_trx_state", "");
    if (rc != 0) {
	return -1;
    }

    // if state is TX, stop it
    // if the RX success, clear the previous message from TX text window
    if (strcmp(result.stringval, "TX") == 0) {
	fldigi_to_rx();
	sleep(2);
    }

    xmlrpc_res_free(&result);
    return 0;
}
#endif

/* send message to Fldigi TX window, transmit it */
int fldigi_send_text(char *line) {
    int rc = 0;

#ifdef HAVE_LIBXMLRPC
    const int len = strlen(line);
    if (len == 0) {
	return 0;   // nothing to send
    }

    char *message = g_malloc0(len + 1 + 2);  // worst case: "^r" added, +2 chars

    // scan line for Ctrl-T/Ctrl-R, remove them and set flags accordingly
    // start_tx = Ctrl-T found, switch_to_rx = Ctrl-R found
    bool start_tx = false;
    bool switch_to_rx = false;
    char *dst = message;
    for (char *src = line; *src; ++src) {
	switch (*src) {
	    case CTRL_R:
		switch_to_rx = true;
		break;
	    case CTRL_T:
		start_tx = true;
		break;
	    default:
		*dst++ = *src;
	}
    }

    if (!keyboard_mode) {   // normal mode, override flags
	start_tx = true;
	switch_to_rx = true;
	rc = cancel_tx();
	if (rc != 0) {
	    g_free(message);
	    return -1;
	}
    }

    if (switch_to_rx) {
	strcat(message, "^r");  // append buffered switch to RX
    }

    // add message to TX window
    rc = fldigi_xmlrpc_query(NULL, "text.add_tx", "s", message);
    g_free(message);
    if (rc != 0) {
	return -1;
    }

    if (start_tx) {
	/* switch to TX (immediate) */
	rc = fldigi_xmlrpc_query(NULL, "main.tx", "");
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
    static int lastpos = 0;
    static double last_poll_time = 0.0;
    int retval = 0;

    pthread_mutex_lock(&xmlrpc_get_rx_mutex);
    double now = get_current_seconds();
    if (now < last_poll_time + 0.1) {
	pthread_mutex_unlock(&xmlrpc_get_rx_mutex);
	return 0;   // last poll was within 100 ms, skip this query
    }
    last_poll_time = now;

    rc = fldigi_xmlrpc_query(&result, "text.get_rx_length", "");
    if (rc != 0) {
	pthread_mutex_unlock(&xmlrpc_get_rx_mutex);
	return -1;
    }

    int textlen = result.intval;
    xmlrpc_res_free(&result);

    if (lastpos == 0) {
	lastpos = textlen;
    }

    int available = textlen - lastpos;
    if (available > len - 1) {
	available = len - 1;
    }

    if (available > 0) {
	rc = fldigi_xmlrpc_query(&result, "text.get_rx", "ii",
				 lastpos, available);
	if (rc != 0) {
	    pthread_mutex_unlock(&xmlrpc_get_rx_mutex);
	    return -1;
	}

	if (result.intval > 0 && result.byteval != NULL) {
	    int linelen = result.intval;
	    if (linelen > len - 1) {    // sanity check, just in case
		linelen = len - 1;
	    }
	    memcpy(line, result.byteval, linelen);
	    line[linelen] = '\0';
	    retval = linelen;
	}

	xmlrpc_res_free(&result);
    }

    lastpos = textlen;
    // note: we move lastpos even buffer was too small to fetch all new text

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
    char fldigi_mode[6] = "";

    rc = fldigi_xmlrpc_query(&result, "modem.get_carrier", "");
    if (rc != 0) {
	return -1;
    }

    fldigi_var_carrier = (int)result.intval;
    xmlrpc_res_free(&result);

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
		rc = fldigi_xmlrpc_query(&result,
					 "modem.set_carrier", "i",
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
    rc = fldigi_xmlrpc_query(&result, "modem.get_name", "");
    if (rc != 0) {
	return -1;
    }

    if (strcmp(result.stringval, "RTTY") == 0) {
	modeshift = 170 / 2;
    }
    xmlrpc_res_free(&result);

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

    /* set the mode in Fldigi if needed */
    rc = fldigi_xmlrpc_query(&result, "rig.get_mode", "");
    if (rc != 0) {
	return -1;
    }

    bool mode_is_different = (strcmp(result.stringval, fldigi_mode) != 0);
    xmlrpc_res_free(&result);

    if (mode_is_different) {
	rc = fldigi_xmlrpc_query(NULL, "rig.set_mode", "s", fldigi_mode);
	if (rc != 0) {
	    return -1;
	}
    }

    /* also set the freq value in Fldigi FREQ block if it is different */
    fldigi_var_carrier = signum * fldigi_var_carrier + modeshift;
    /* round to Hz precision */
    xmlrpc_double freq_target = round(freq - fldigi_var_carrier);

    rc = fldigi_xmlrpc_query(&result, "rig.get_frequency", "");
    if (rc != 0) {
	return -1;
    }

    xmlrpc_double freq_current = round(result.doubleval);
    xmlrpc_res_free(&result);

    if (freq_target != freq_current) {
	rc = fldigi_xmlrpc_query(NULL, "rig.set_frequency", "d", freq_target);
	if (rc != 0) {
	    return -1;
	}
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

static char *clean_input(const char *input, int max_length) {
    char *result = g_malloc0(max_length + 1);
    const int input_length = (input != NULL ? strlen(input) : 0);
    int j = 0;
    // accept only alphanumeric chars and '/'
    // (in case of QRM there could be lot of extra garbage)
    for (int i = 0; i < input_length && j < max_length && input[i] != 0; i++) {
	char c = g_ascii_toupper(input[i]);
	if (valid_call_char(c)) {
	    result[j++] = c;
	}
    }
    return result;
}

/* read callsign field from Fldigi and set the CALL in Tlf */
int fldigi_get_log_call() {
#ifdef HAVE_LIBXMLRPC
    xmlrpc_res result;

    if (current_qso.call[0] != 0) {
	// call is already filled, not checking further
	return 0;
    }

    // if we set the call earlier but the OP cleared the callsign field
    // then clear it in Fldigi too
    if (call_set) {
	call_set = false;
	return fldigi_xmlrpc_query(NULL, "log.set_call", "s", "");
    }

    // otherwise, fetch the callsign field from Fldigi
    int rc = fldigi_xmlrpc_query(&result, "log.get_call", "");
    if (rc != 0) {
	return -1;
    }

    char *tempstr = clean_input(result.stringval, MAX_CALL_LENGTH);

    xmlrpc_res_free(&result);

    if (strlen(tempstr) >= 3) {
	strcpy(current_qso.call, tempstr);
	call_set = true;
	fldigi_set_callfield = true;
    }

    g_free(tempstr);
#endif
    return 0;
}

/* read exchange field from Fldigi and set it in Tlf */
int fldigi_get_log_serial_number() {
#ifdef HAVE_LIBXMLRPC
    xmlrpc_res result;

    if (current_qso.comment[0] != 0) {
	// comment (exchange) is already filled, not checking further
	return 0;
    }

    // if we set the comment earlier but the OP cleared the comment field
    // then clear it in Fldigi too
    if (comment_set) {
	comment_set = false;
	return fldigi_xmlrpc_query(NULL, "log.set_exchange", "s", "");
    }

    // otherwise, fetch the exchange field from Fldigi
    int rc = fldigi_xmlrpc_query(&result, "log.get_exchange", "");
    if (rc != 0) {
	return -1;
    }

    char *tempstr = clean_input(result.stringval, contest->exchange_width);

    xmlrpc_res_free(&result);

    if (strlen(tempstr) > 0) {
	strcpy(current_qso.comment, tempstr);
	comment_set = true;
	refresh_comment();  // NOTE: UI operation
    }

    g_free(tempstr);
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
