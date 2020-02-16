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


#ifndef GETWWV_H
#define GETWWV_H

#include <time.h>

extern double ssn_r;        // sunspot number for MUF calculation
extern char lastwwv[];      // processed WWV message
extern char lastwwv_raw[];  // raw WWV message
extern time_t lastwwv_time;

void wwv_add(char *s);
void wwv_set_r(double r);
void wwv_set_sfi(double sfi);
void wwv_show_footer();

#endif /* GETWWV_H */
