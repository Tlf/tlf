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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

   	/* ------------------------------------------------------------
 	*       delete  last qso
 	*
 	*--------------------------------------------------------------*/

#include "globalvars.h"
#include "deleteqso.h"

int delete_qso(void)
{

int x, isnote;
int lfile;
struct stat statbuf;


	mvprintw(13,29,  "OK to delete last qso (y/n)?");
	x =  onechar();
	
    if ((x == 'y') || (x  == 'Y'))
    {

		if ((lfile = open(logfile, O_RDWR)) < 0){

			mvprintw(24,0, "I can not find the logfile...");
			refresh();
			sleep(2);
		}  else {

			nr_qsos--;
			qsos[nr_qsos][0]='\0';

			fstat(lfile, &statbuf);

			if(statbuf.st_size > 80)
				ftruncate(lfile, statbuf.st_size - 81);

			fsync(lfile);
			close(lfile);

		}

		if (logline4[0] == ';') isnote = 1;
		else isnote = 0;

		if (isnote == 0) {
			band_score[bandinx]--;
			qsonum--;
			qsonr_to_str();
		}

		scroll_log();

	}


		attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);
		mvprintw(13,29,  "                            ");

		printcall();

		clear_display();

return(0);
}


