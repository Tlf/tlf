/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2020 Zoltan Csahok <ha5cqz@freemail.hu>
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

#ifndef TRX_MEMORY_H
#define TRX_MEMORY_H

#include "time_update.h"
#include "tlf.h"

extern freq_t memory_get_freq();        // returns 0 if memory is empty
extern cqmode_t memory_get_cqmode();    // returns NONE if memory is empty

extern void memory_store_or_pop();
extern void memory_store();
extern void memory_pop();
extern void memory_swap();

#endif

