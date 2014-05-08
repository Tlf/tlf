/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2005 Rein Couperus <pa0r@amsat.org>
 *               2012      Thomas Beierlein <tb@forth-ev.de>
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
 	*        Sendspcall  sends (de) call if callarea empty
 	*
 	*--------------------------------------------------------------*/

#include "sendspcall.h" 	
 	
int play_file(char *audiofile);
	
void sendspcall(void){

    extern int demode;
    extern char buffer[];
    extern char call[];
    extern int trxmode;
    extern int keyerport;
    extern char ph_message[14][80];
    extern char hiscall[];


    if (trxmode == CWMODE) {

	if (demode ==  SEND_DE )
	    strcat(buffer, "DE ");

	strcat(buffer, call);
	sendbuf();

    } else if (trxmode == DIGIMODE) {

	if (keyerport == MFJ1278_KEYER) {
	    strcat (buffer, "{ ");	/* => ctrl-t */
	    if (demode ==  SEND_DE) {
	        strcat(buffer, hiscall);
		strcat(buffer, " DE ");
	    }
	    strcat (buffer, call);
	    strcat (buffer, "}");	/* => ctrl-r */
	}
	else {
	    if (demode ==  SEND_DE ) {
	        strcat(buffer, hiscall);
		strcat(buffer, " DE ");
	    }
	    strcat(buffer, call);
	}

	sendbuf();

    } else

	play_file(ph_message[5]);
}
