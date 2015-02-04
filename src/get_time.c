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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

	/* ------------------------------------------------------------------------
	*  get the local time from the  kernel  and put into global buffer (time_ptr)
	*   for use by  several routines
	---------------------------------------------------------------------------*/

#include "get_time.h"
#include <time.h>

void get_time(void)
{
    extern struct tm *time_ptr;
    extern int timeoffset;
    extern long timecorr;

    time_t now;

//time (&now);
    now = (time(0) + (timeoffset * 3600) + timecorr);

    time_ptr =  gmtime(&now);
}

