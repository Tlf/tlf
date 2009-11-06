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
 	*     get the prefix
 	*
 	*--------------------------------------------------------------*/
 	
#include "globalvars.h" 	
#include "getpx.h" 	
#include <curses.h>

 	
void getpx(char *checkcall)
{
 	char pxbuffer[16] = "";
 	int i;
 	char j = 0;
 	
 	
	if  (strlen(checkcall)  >=  2){
 	
 		if ((checkcall[strlen(checkcall)-2] == '/') && (checkcall[strlen(checkcall)-1] <= '9') && (checkcall[strlen(checkcall)-1] >= '0'))      /*  portable /3 */
 	 		j = checkcall[strlen(checkcall)-1];
 	}
 	
 	for  (i = 0  ; i < strlen(checkcall) ; i++ ){
 	
 	 	if (((checkcall[i] <= 'Z')  && (checkcall[i]  >= 'A'))  || (i == 0))
 	   		pxbuffer[i]  = checkcall[i]; 	  	
 	  	else 		
 	  		break;
 	}
 	pxbuffer[i]  = checkcall[i];
 	
 	i++;
 	
 	if (isdigit(checkcall[i])){
 	 	pxbuffer[i]  = checkcall[i];
 	 	pxbuffer[i+1] = '\0';
 	} else  {
 		if (j != 0) {
 	 		pxbuffer[i-1] = j;
 	 	}	
 	 	pxbuffer[i] = '\0';
 	}
	 		 	
 	strcpy(pxstr, pxbuffer);
 	
 	if (pxstr[strlen(pxstr)-1] == '/') {
 		pxstr[strlen(pxstr)-1] = '0';	
 	}
 	else if ((pxstr[ strlen(pxstr) - 1] <= 'Z') &&  (pxstr[ strlen(pxstr) - 1] >= 'A') )
 		strcat(pxstr, "0");

}

