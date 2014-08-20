/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014           Ervin Hegedüs - HA2OS <airween@gmail.com>
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

#include <glib.h>
#include "tlf.h"

//int create_store();
int qtc_inc(char callsign[15], int direction);
int qtc_dec(char callsign[15], int direction);
struct t_qtc_store_obj * qtc_get(char callsign[15]);

int parse_qtcline(char * line, char callsign[15], int direction);

