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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
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
#define CTRL_U    21
#define CTRL_V    22
#define CTRL_W    23

/* Keyboard characters */
#define ESCAPE    27
#define SPACE     32
#define BACKSLASH 92
#define DELETE    127

#define ALT_A     225
#define ALT_B     226
#define ALT_C     227
#define ALT_E     229
#define ALT_G     231
#define ALT_H     232
#define ALT_I     233
#define ALT_J     234
#define ALT_K     235
#define ALT_M     237
#define ALT_N     238
#define ALT_P     240
#define ALT_Q     241
#define ALT_R     242
#define ALT_S     243
#define ALT_T     244
#define ALT_V     246
#define ALT_W     247
#define ALT_X     248
#define ALT_Z     250

/* Common name macros. */
#define TAB       CTRL_I
#define LINEFEED  CTRL_J
#define RETURN    CTRL_M

#define SHIFT_F(n)  (KEY_F(n) + 12)
