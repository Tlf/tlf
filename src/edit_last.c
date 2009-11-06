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
 	*   edit last qso
 	*
 	*--------------------------------------------------------------*/

#include "globalvars.h"
#include "edit_last.h"
#include "clear_display.h"

char logline_edit[5][82];

int edit_last(void)
{
 	
  	int j=0, b, k, ii=0;
	int lfile;
	struct stat statbuf;
	int editline = 4;

	stop_backgrnd_process = 1;         //(no qso add during edit process)

	attron(A_STANDOUT);

	attron(COLOR_PAIR(COLOR_GREEN));
	mvprintw(7 + editline,0, logline_edit[editline]);



	b =  29;
	mvprintw(7 + editline, b, "");

	while ((j != 27) && (j != '\n')){

		j = onechar();
		
		if(j == 1) {					// ctrl A, beginning of line
				b = 1;
				attron(COLOR_PAIR(COLOR_GREEN)  | A_STANDOUT);
				mvprintw(7 + editline,0, logline_edit[editline]);
				mvprintw(7 + editline, b, "");
				refresh();
		}else if(j == 5) {					// ctrl E, end of line
				b = 77;
				attron(COLOR_PAIR(COLOR_GREEN)  | A_STANDOUT);
				mvprintw(7 + editline,0, logline_edit[editline]);
				mvprintw(7 + editline, b, "");
				refresh();
		}else if(j == 9) {					// TAB, next field
				if (b == 1) b = 17;
				else if (b == 17) b = 29;
				else if (b == 29) b = 54;
				else if (b == 54) b = 68;
				else if (b == 68) b = 77;
				else if (b == 77) b = 1;

				attron(COLOR_PAIR(COLOR_GREEN)  | A_STANDOUT);
				mvprintw(7 + editline,0, logline_edit[editline]);
				mvprintw(7 + editline, b, "");
				refresh();

		}else if(j == 152) {  			// up
			if (editline > (6-nr_qsos) && (editline > 0)) {
				attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);
 				mvprintw(7 + editline,0, logline_edit[editline]);
				editline--;
				attron(COLOR_PAIR(COLOR_GREEN)  | A_STANDOUT);
				mvprintw(7 + editline,0, logline_edit[editline]);
				mvprintw(7 + editline, b, "");

				refresh();
			} else {
				logview();
				j = 27;
			}

		}else if(j == 153) {  			// down

			if (editline < 4) {
				attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);
 				mvprintw(7 + editline,0, logline_edit[editline]);
				editline++;
				attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
				mvprintw(7 + editline,0, logline_edit[editline]);
				mvprintw(7 + editline, b, "");

				refresh();
			} else j = 27;                              /* escape */


		}else if (j == 155){

			if (b >= 1)
				b--;

			mvprintw(7 + editline, 0, logline_edit[editline]);
			mvprintw(7 + editline,  b, "");
			refresh();

		}else if (j == 154){
		 	if (b < 79){
		 		b++;
		 	}
		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();

		} else if ((j == 160) && (b >= 0 ) && ( b< 28 ) ) {                            // insert

			for (k = 28; k >b; k--)
				logline_edit[editline][k] = logline_edit[editline][k-1];
			logline_edit[editline][b] = ' ';
 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();

		} else if ((j == 160) && (b >= 29 ) && ( b< 39 ) ) {                            // insert  call
				for (k = 39; k >b; k--)
					logline_edit[editline][k] = logline_edit[editline][k-1];
				logline_edit[editline][b] = ' ';
 		 		mvprintw(7 + editline,0, logline_edit[editline]);
		 		mvprintw(7 + editline, b, "");
		 		refresh();
			
		} else if ((j == 160) && (b >= 54 ) && ( b< 64 ) ) {                            // insert

			for (k = 64; k >b; k--)
				logline_edit[editline][k] = logline_edit[editline][k-1];
			logline_edit[editline][b] = ' ';
 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();

		} else if ((j == 160) && (b >= 68 ) && ( b< 76 ) ) {                            // insert

			for (k = 76; k >b; k--)
				logline_edit[editline][k] = logline_edit[editline][k-1];
			logline_edit[editline][b] = ' ';
 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();

		} else if ((j == 161) && (b >= 1 ) && ( b< 28 )) {                            // delete

			for (k = b; k < 28; k++)
				logline_edit[editline][k] = logline_edit[editline][k+1];

 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();


		} else if ((j == 161) && (b >= 29 ) && ( b< 39 )) {                            // delete

			for (k = b; k < 39; k++)
				logline_edit[editline][k] = logline_edit[editline][k+1];

 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();

		} else if ((j == 161) && (b >= 68 ) && ( b< 76 )) {                            // delete

			for (k = b; k < 76; k++)
				logline_edit[editline][k] = logline_edit[editline][k+1];

 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();

		} else if ((j == 161) && (b >= 54 ) && ( b< 64 )) {                            // delete

			for (k = b; k < 64; k++)
				logline_edit[editline][k] = logline_edit[editline][k+1];

 		 	mvprintw(7 + editline,0, logline_edit[editline]);
		 	mvprintw(7 + editline, b, "");
		 	refresh();


		}else if (j != 27){

			if ((j >= 97) && (j <= 122))
				j = j - 32;
			if ((j >= 32) && (j<97)){
				logline_edit[editline][b] = j;
		 		mvprintw(7 + editline,0, logline_edit[editline]);
				if ((b < strlen(logline_edit[editline])-2) && (b < 80))
					b++;
				mvprintw(7 + editline, b, "");
			}
		}

	}



	attron(COLOR_PAIR(COLOR_WHITE) | A_STANDOUT);

 	mvprintw(7 + editline,0, logline_edit[editline]);
	refresh();

	if ((lfile = open(logfile, O_RDWR)) < 0){

		mvprintw(24,0, "I can not find the logfile...");
		refresh();
		sleep(2);
	}  else {


		for (ii = 4; ii >=0; ii--) {

			fstat(lfile, &statbuf);

			if(statbuf.st_size >  80) {
				ftruncate(lfile, statbuf.st_size -  81);
				nr_qsos--;
				qsos[nr_qsos][0]='\0';
				fsync(lfile);

			} else break;

		}


	}

	close(lfile);

	ii++;

	while (ii < 5) {
		store_qso(logline_edit[ii]);
		ii++;
	}


	scroll_log();

	stop_backgrnd_process = 0;

	return(0);
}


