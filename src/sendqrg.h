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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef SENDQRG_H
#define SENDQRG_H

#include <hamlib/rig.h>

#ifdef HAMLIB_FILPATHLEN
  #define TLFFILPATHLEN HAMLIB_FILPATHLEN
#else
  #ifdef FILPATHLEN
  #define TLFFILPATHLEN FILPATHLEN
  #else
  #error "FILPATHLEN macro not found"
  #endif
#endif

int init_tlf_rig(void);
void close_tlf_rig(RIG *my_rig);

int sendqrg(void);

#endif /* end of include guard: SENDQRG_H */
