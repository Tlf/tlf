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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#ifndef SETCONTEST_H
#define SETCONTEST_H

#include "bandmap.h"
#include "globalvars.h"

#define CONTEST_IS(cid) (contest->id == cid)
#define IS_ALL_BAND (CONTEST_IS(QSO) || !iscontest)

extern contest_config_t config_qso;

bool general_ismulti(spot *data);
contest_config_t *lookup_contest(char *name);
void list_contests();
void setcontest(char *name);
bool no_multi(spot *data);

#endif /* SETCONTEST_H */
