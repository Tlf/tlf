/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2019      Nate Bargmann <n0nb@n0nb.us>
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

/* ------------------------------------------------------------
 *        Keystroke magic numbers and names as symbolic
 *        constants.  Values are decimal ordinals provided
 *        by the ncurses library.  See doc/keynames.txt.
 *
 *--------------------------------------------------------------*/

#define CTRL_A    1
#define CTRL_B    2
#define CTRL_E    5
#define CTRL_F    6
#define CTRL_G    7             /* Bell */
#define CTRL_H    8             /* Backspace */
#define CTRL_I    9             /* Tab */
#define CTRL_J    10            /* Newline/Linefeed */
#define CTRL_K    11
#define CTRL_L    12
#define CTRL_M    13            /* Return */
#define CTRL_N    14
#define CTRL_P    16
#define CTRL_Q    17
#define CTRL_R    18
#define CTRL_S    19
#define CTRL_T    20
#define BACKSLASH 92
#define ALT_N     238

/* Common name macros. */
#define TAB       CTRL_I
#define LINEFEED  CTRL_J
#define RETURN    CTRL_M
