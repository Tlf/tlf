/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2015 Nate Bargmann <n0nb@n0nb.us>
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


/* Collate the macro test boilerplate into this file and then
 * include this file into the Tlf source files that need panel.h
 * functions.
 *
 * For ncurses including panel.h also includes curses.h.
 */

#ifndef TLF_PANEL_H
#define TLF_PANEL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined HAVE_NCURSESW_PANEL_H
# include <ncursesw/panel.h>
#elif defined HAVE_NCURSES_PANEL_H
# include <ncurses/panel.h>
#elif defined HAVE_PANEL_H
# include <panel.h>
#else
# error "SysV-compatible Curses Panel header file required"
#endif

#endif
