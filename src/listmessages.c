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
 	*        List  CW messages
 	*
 	*--------------------------------------------------------------*/
 	
#include "listmessages.h"

int listmessages(void)
{
	extern char message[15][80];
	extern char backgrnd_str[];
	
	int i, j;
	char  printbuffer[160];

	nicebox(10, 0, 12, 78, "Messages");
	
	for  ( i = 0  ; i  <= 11 ; i++){
		printbuffer[0] = '\0';
		strncat (printbuffer,  message[i],  strlen(message[i]) -1);
		strcat  (printbuffer, backgrnd_str);
		printbuffer[71] = '\0';
		attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT );
		mvprintw (i + 11, 1, " %i     ",  i+1);
		mvprintw (i  + 11, 6,  ": %s",  printbuffer);
		refreshp();
	}
	attroff(A_STANDOUT);
	mvprintw(23, 30,  "Press any key");
	i = onechar();
	
		for  ( i = 2  ; i  <= 13 ; i++){
		printbuffer[0] = '\0';
		strncat (printbuffer,  message[i],  strlen(message[i]) -1);
		strcat  (printbuffer, backgrnd_str);
		printbuffer[71] = '\0';
		attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT );
		
		if (i == 12)
			mvprintw(i + 9, 1, " SPmg:" );
		else if ( i== 13)
			mvprintw(i + 9, 1, " CQmg:" );
		else
			mvprintw (i + 9, 1, " %i     ",  i+1);
		
		mvprintw (i  + 9, 6,  ": %s",  printbuffer);
		refreshp();
		
	}
	attroff(A_STANDOUT);
	mvprintw(23, 30,  "Press any key");
	i = onechar();
	
	clear_display();
	attron(COLOR_PAIR(7)  |  A_STANDOUT);

	for (j = 13 ;  j  <= 23 ; j++){
		 mvprintw(j, 0, backgrnd_str);
		}


 return(0);
}

