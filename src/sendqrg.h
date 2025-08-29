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


#ifndef SENDQRG_H
#define SENDQRG_H

#include <hamlib/rig.h>
#include <hamlib/rotator.h>
#include <stdbool.h>

bool rig_has_send_morse();
bool rig_has_stop_morse();

int init_tlf_rig(void);
void close_tlf_rig(RIG *my_rig);
void rig_debug_init();

bool sendqrg(void);

int init_tlf_rot(void);
double get_rotator_bearing();
void rotate_to_qrb(bool long_path);
void stop_rotator();
void close_tlf_rot(ROT *my_rot);

#endif /* end of include guard: SENDQRG_H */
