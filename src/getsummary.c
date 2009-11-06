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
 	*   write summary  file
 	*
 	*--------------------------------------------------------------*/
	
#include "getsummary.h" 	
 	
 	extern int cluster;
 	extern int showscore_flag;
 	extern char call[];
 	extern int wpx;
 	extern int total;
 	extern int nr_of_px;
 	extern int cqww;
 	extern int arrldx_usa;
 	extern int totalmults;
 	extern char exchange[];
	extern int multcount;
	extern int arrlss;
	extern int serial_section_mult;
 	
 	int getsummary(void)
 	{
	char  buffer[80]; 	
 	FILE *fp;
 	
 	cluster =  NOCLUSTER;
 	showscore_flag  = 0;
	
	if  ( (fp = fopen("./header","w"))  == NULL){
		fprintf(stdout,  "Error opening header file.\n");
		return(1);
	}

	fputs("START-OF-LOG: 2.0\n", fp);

		attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Your exchange (e.g. State, province, age etc... (# if serial number)): ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
		echo();
		getnstr(buffer, 78);
		noecho();
		strncpy(exchange, buffer, 10);
		
		attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "ARRL-SECTION:");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
	
		echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
		if (strncmp(buffer, "none", 4) != 0) {
			fputs("ARRL-SECTION: ", fp);
			fputs(buffer, fp);
		}
		fputs("CALLSIGN: ", fp);
		fputs(call, fp);
		fputs("CATEGORY: ",  fp);
	
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Cat.:(SINGLE-OP, SINGLE-OP-ASSISTED,MULTI-ONE,MULTI-TWO,MULTI-MULTI,CHECKLOG)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, " ");
	fputs(buffer, fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Bands:(ALL,160M,80M,40M,20M,15M,10M)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat (buffer,  " ");
	fputs(buffer,  fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "POWER: (HIGH,LOW,QRP)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer,  " ");
	fputs(buffer, fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Mode: (CW,SSB,MIXED)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs(buffer, fp);
	fputs("CATEGORY-OVERLAY: ", fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Overlay: (ROOKIE,BAND-LIMITED,TB-WIRES,OVER-50,HQ)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer,  "\n");
	fputs(buffer, fp);

	if (wpx == 1)
		sprintf(buffer, "%d\n",total  * (nr_of_px ));
	if (cqww == 1)
	    sprintf(buffer, "%d\n",total  * totalmults);
	if (arrldx_usa == 1)
	    sprintf(buffer, "%d\n",total  * totalmults);
	if (arrlss == 1)
		sprintf(buffer, "%d\n", multcount * total);
	if (serial_section_mult == 1)
		sprintf(buffer, "%d\n", totalmults * total);

	fputs("CLAIMED-SCORE: ", fp);
	fputs( buffer, fp);
	fputs("CLUB: ", fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Club: ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs(buffer, fp);
	fputs("CONTEST: ", fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Contest: (CQ-WW-CW/SSB, CQ-WPX-CW/SSB, ARRL-DX-CW/SSB)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
		fputs(buffer, fp);
// LU4HKN mod.
		strcpy(buffer, "CREATED-BY: tlf-");
		strcat(buffer, VERSION);
		strcat(buffer, "\n");
		fputs(buffer, fp);
 // end
		attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Name: ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");

     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer,  "\n");
	fputs("NAME: ", fp);
	fputs(buffer,  fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "ADDRESS: ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs("ADDRESS: ",  fp);
	fputs(buffer, fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "ADDRESS(2): ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs("ADDRESS: ",  fp);
	fputs(buffer, fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "ADDRESS(3): ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs("ADDRESS: ", fp);
	fputs(buffer, fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "Operators: ");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs("OPERATORS: ",fp);
	fputs(buffer, fp);
        attron(A_STANDOUT);
        mvprintw(15,1, "                                                                                    ");
        nicebox(14,0, 1, 78, "SOAPBOX: (use any text editor to include more lines)");
        attron(A_STANDOUT);
        mvprintw(15,1,"");
     	
     	echo();
		getnstr(buffer, 78);
		noecho();
		strcat(buffer, "\n");
	fputs("SOAPBOX: ", fp);
	fputs(buffer, fp);
	
 	 fclose(fp);
 	 return(0);
 	}

 	
 	
