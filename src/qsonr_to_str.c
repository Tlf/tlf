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
 	*        i-to-a function for qso number (4  chars)
 	*
 	*--------------------------------------------------------------*/

#include "qsonr_to_str.h"

int  qsonr_to_str(void)
{
 extern int qsonum;
 extern char qsonrstr[5];

static int x;
static int thousands;
static int hundreds;
static int tens;
static char buffer[5];

 x  =  qsonum;
 thousands =  (x /  1000);
 x  =  x  - (thousands  *  1000);
 hundreds  = (x  /  100);
 x  = x - (hundreds * 100);
 tens  = (x  /  10);
 x =  x - (tens * 10);

 buffer[0] =  thousands  + 48 ;
 buffer[1] =  hundreds  +  48  ;
 buffer[2] = tens +  48  ;
 buffer[3]  = x + 48  ;
 buffer[4]  =  '\0'  ;
 strncpy (qsonrstr, buffer, 4);

 return (0);
}

