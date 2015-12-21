/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011, 2014     Thomas Beierlein <tb@forth-ev.de>
 *               2014           Ervin Hegedus <airween@gmail.com>
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


#include "startmsg.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LIBXMLRPC
# include <xmlrpc-c/base.h>
# include <xmlrpc-c/client.h>
#endif

#define NAME "Tlf"
#define XMLRPCVERSION "1.0"


int fldigi_var_carrier = 0;

int fldigi_xmlrpc_get_carrier() {

#ifndef HAVE_LIBXMLRPC
    return 0;
#else

    xmlrpc_env env;
    xmlrpc_value * result;
    xmlrpc_int32 sum;
    xmlrpc_env_init(&env);

    static int errflg;
    static int trycnt;

    /* if some call timed outs/breaks, we will suspend the call */
    if (errflg > 0 && trycnt < 1000) {
        trycnt++;
	return -1;
    }

    errflg = 0;
    trycnt = 0;
    const char * const serverUrl = "http://localhost:7362/RPC2";
    const char * const methodName = "modem.get_carrier";

    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, XMLRPCVERSION, NULL, 0);
    if (env.fault_occurred) {
	fldigi_var_carrier = 0;
	errflg = 1;
        return -1;
    }

    result = xmlrpc_client_call(&env, serverUrl, methodName,
                                 "(ii)", (xmlrpc_int32) 5, (xmlrpc_int32) 7);
    if (env.fault_occurred) {
	fldigi_var_carrier = 0;
	errflg = 1;
        return -1;
    }

    xmlrpc_read_int(&env, result, &sum);
    if (env.fault_occurred) {
	fldigi_var_carrier = 0;
	errflg = 1;
	return -1;
    }
    fldigi_var_carrier = (int)sum;

    xmlrpc_DECREF(result);
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return 0;
#endif

}

int fldigi_get_carrier() {
#ifdef HAVE_LIBXMLRPC
        return fldigi_var_carrier;
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
