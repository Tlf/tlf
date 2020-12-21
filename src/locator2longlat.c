/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
 *
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

/* returns true if 'qra' is a valid QRA locator
 * note: only the first 4 characters are tested
 */
int check_qra(char *qra) {

    return strlen(qra) >= 4
	   && qra[0] >= 'A' && qra[0] <= 'R'
	   && qra[1] >= 'A' && qra[1] <= 'R'
	   && qra[2] >= '0' && qra[2] <= '9'
	   && qra[3] >= '0' && qra[3] <= '9';

}

