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


#ifndef SEARCHLOG_H
#define SEARCHLOG_H

#include <stdbool.h>
#include <glib.h>

extern GPtrArray *callmaster;
#define CALLMASTERARRAY(n) ((char *) g_ptr_array_index(callmaster, n))

extern char *callmaster_filename;
extern char callmaster_version[12];

int load_callmaster(void);

/* search 'hiscall' in the log */
void searchlog(void);

void InitSearchPanel(void);
void ShowSearchPanel(void);
void HideSearchPanel(void);
void OnLowerSearchPanel(int x, char *str);

#endif /* SEARCHLOG_H */
