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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


/* Collate the macro test boilerplate into this file and then
 * include this file into the Tlf source files that need [n]curses.h
 * functions.
 */

#ifndef TLF_CURSES_H
#define TLF_CURSES_H

#include <config.h>

#if defined HAVE_NCURSESW_CURSES_H
# include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
# include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
# include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
# include <ncurses.h>
#elif defined HAVE_CURSES_H
# include <curses.h>
#else
# error "SysV or X/Open-compatible Curses header file required"
#endif


/* Additional key codes reported by Gnome Terminal and Xterm
 * and apparently in the terminfo database but not in Ncurses header files.
 *
 * The following keys are defined in the xterm terminfo database.
 * Key names with a suffix of '3' are Alt-key combinations.
 * Key names with a suffix of '5' are Ctl-key combinations.
 *
 * Keys are defined in decimal ordinal order.
 */
#define kDC5	519	/* Ctrl-Delete */
#define kDN3	523	/* Alt-Down arrow */
#define kDN5	525	/* Ctrl-Down arrow */
#define kEND3	528	/* Alt-End */
#define kEND5	530	/* Ctrl-End */
#define kHOM3	533	/* Alt-Home */
#define kHOM5	535	/* Ctrl-Home */
#define kIC5	540	/* Ctrl-Insert */
#define kLFT3	543	/* Alt-Left arrow */
#define kLFT5	545	/* Ctrl-Left arrow */
#define kNXT3	548	/* Alt-Page Down */
#define kNXT5	550	/* Ctrl-Page Down */
#define kPRV3	553	/* Alt-Page Up */
#define kPRV5	555	/* Ctrl-Page Up */
#define kRIT3	558	/* Alt-Right arrow */
#define kRIT5	560	/* Ctrl-Right arrow */
#define kUP3	564	/* Alt-Up arrow */
#define kUP5	566	/* Ctrl-Up arrow */

#endif
