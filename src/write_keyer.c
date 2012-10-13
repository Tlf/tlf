/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004-2005 Rein Couperus <pa0r@amsat.org>
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
#include "write_keyer.h"
#include <assert.h>
#include "clear_display.h"
#include "cwkeyer.h"
#include <curses.h>
#include "netkeyer.h"

int write_keyer(void)
{

    extern char wkeyerbuffer[];
    extern int trxmode;
    extern int keyerport;
    extern int data_ready;
    extern char controllerport[];
    extern int native_rig_fd;
    extern char speedstr[];
    extern int speed;
    extern char rttyoutput[];

    FILE *bfp = NULL;
    int i, rc;
    char send_orion[3];
    char buff[8];
    int realspeed = 32;
    char outstring[120] = "";

    if (trxmode != CWMODE && trxmode != DIGIMODE)
	return (1);

    if (data_ready == 1) {

	if (keyerport == NET_KEYER) {
	    netkeyer(K_MESSAGE, wkeyerbuffer);
	    wkeyerbuffer[0] = '\0';
	    data_ready = 0;

	} else if (keyerport == MFJ1278_KEYER) {
	    if ((bfp = fopen(controllerport, "a")) == NULL) {
		mvprintw(24, 0, "1278 not active. Switching to SSB mode.");
		sleep(1);
		trxmode = SSBMODE;
		clear_display();
	    } else {
		fputs(wkeyerbuffer, bfp);
		wkeyerbuffer[0] = '\0';
		data_ready = 0;

		fclose(bfp);
	    }

	} else if (keyerport == GMFSK) {
	    if (strlen(rttyoutput) < 2) {
		mvprintw(24, 0, "No modem file specified!");
	    }
	    sprintf(outstring, "echo -n \"\n%s\" >> %s", 
		    wkeyerbuffer, rttyoutput);
	    rc = system(outstring);

	    wkeyerbuffer[0] = '\0';
	    data_ready = 0;

	} else if (keyerport == ORION_KEYER && strlen(wkeyerbuffer) > 0) {
	    if (native_rig_fd == 0) {
		mvprintw(24, 0, "Orion keyer not open.");
		sleep(1);
		clear_display();
	    } else {
		strncpy(buff, (speedstr + (speed * 2)), 2);
		buff[2] = '\0';
		realspeed = atoi(buff);

		for (i = 0; i < strlen(wkeyerbuffer); i++) {

		    if (strlen(wkeyerbuffer) == 0)
			break;
		    if (wkeyerbuffer[i] != ' ') {
			send_orion[0] = '/';
			send_orion[1] = wkeyerbuffer[i];
			send_orion[2] = '\015';
			rc = write(native_rig_fd, send_orion, 3);

			usleep(cw_char_length(send_orion + 1) *
			       (int) (1200000.0 / realspeed));
		    } else
			usleep(6 * (int) (1200000.0 / realspeed));
		}

		wkeyerbuffer[0] = '\0';
		data_ready = 0;

	    }

	}
    }

    return (0);

}
