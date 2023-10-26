/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2023	Thomas Beierlein <dl1jbe@darc.de>
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


#ifndef MATH_UTILS_H
#define MATH_UTILS_H

/* Make sure math.h is included before the following definitions */
#ifndef MATH_H
#include <math.h>
#endif

/* provide a fallback for M_PI_2 as C standard does not guarantee the definition
of these constant */
#ifndef M_PI_2
# define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif

#define RADIAN  (90.0 / M_PI_2)

#endif
