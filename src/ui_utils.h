/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2015 Thomas Beierlein <tb@forth-ev.de>
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


#ifndef UI_UTILS_H
#define UI_UTILS_H

extern int key_kNXT3;
extern int key_kPRV3;
extern int key_kNXT5;
extern int key_kPRV5;

void refreshp();
int modify_attr(int attr);
int lookup_key(char *capability);
void lookup_keys();

/** convenience macro to name alt-keys */
#define ALT(c) (c + 128)

int key_get();
int key_poll();


#endif /* UI_UTILS_H */
