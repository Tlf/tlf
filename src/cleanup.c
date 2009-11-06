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
 	*        Cleanup call input field
 	*
 	*--------------------------------------------------------------*/

#include "cleanup.h"

 int cleanup(void)
 {

 extern int use_rxvt;
 extern char hiscall[];
 extern char comment[];
 extern int defer_store;
 extern char wkeyerbuffer[];
 
 int k = 0;

  	if (use_rxvt == 0) attron(COLOR_PAIR(NORMCOLOR) | A_BOLD  );
	else  attron(COLOR_PAIR(NORMCOLOR) );

	mvprintw(12, 29, "            ");
	mvprintw(12, 54, "                        ");
	mvprintw(12, 29, "");

	attron(COLOR_PAIR(COLOR_WHITE  | A_STANDOUT));

	for (k = 1; k <= 5; k++) {
		mvprintw(k, 0, "%s", "                                        ");	
	}
	
	refresh();
	hiscall[0] = '\0';
	comment[0]='\0';
	defer_store = 0;
	wkeyerbuffer[0] = '\0';		// stop keyer ?? 
	
  return(0);
 }

