/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
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
#include <curses.h>
#include <stdio.h>
#include "setcontest.h"
#include "lancode.h"

#ifndef PARSE_LOGCFG_H
#define PARSE_LOGCFG_H

enum{ 
    PARSE_OK,
    PARSE_ERROR,
    PARSE_CONFIRM
};

int read_logcfg(void);
int parse_logcfg(char *inputbuffer);

#endif // PARSE_LOGCFG_H
