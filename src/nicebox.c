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
 	*          Nicebox draws  a  box with  a header
 	*
 	*--------------------------------------------------------------*/

#include "nicebox.h"

int nicebox( int Nb_y,  int  Nb_x, int Nb_height,  int Nb_width, char *Nb_boxname)
{
 extern int use_rxvt;

 int y, x,  height, width;
 char boxname[21];

 y = Nb_y;
 x = Nb_x;
 height  =  Nb_height + 1;
 width = Nb_width  +  1;
 *boxname =  *Nb_boxname;

	attroff(A_STANDOUT);
	if (use_rxvt == 0) attron(COLOR_PAIR(COLOR_YELLOW) | A_BOLD );
	else  attron(COLOR_PAIR(COLOR_YELLOW) );

	mvaddch(y,x, ACS_ULCORNER);
	hline(ACS_HLINE, width);
	mvaddch(y + height ,x, ACS_LLCORNER);
	hline(ACS_HLINE, width);
	mvaddch(y, x + width,  ACS_URCORNER);
	mvvline(y +  1, x + width, ACS_VLINE, height);
	mvaddch(y  + height, x  + width,  ACS_LRCORNER);
				
	mvvline(y + 1, x,  ACS_VLINE, height - 1);
	mvprintw ( y, x+2,  Nb_boxname);

  return(0);
}

