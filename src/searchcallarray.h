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


#ifndef SEARCHCALLARRAY_H
#define SEARCHCALLARRAY_H

#include <stdbool.h>

#include "tlf.h"

void init_worked(void);
int lookup_worked(char *call);
int lookup_or_add_worked(char *call);
bool worked_in_current_minitest_period(int found);
bool is_dupe(char *call, int bandindex, int mode);
void update_worked(int station, struct qso_t *qso);

#endif /* SEARCHCALLARRAY_H */
