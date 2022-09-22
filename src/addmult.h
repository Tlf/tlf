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


#ifndef ADDMULT_H
#define ADDMULT_H

#include <glib.h>

#include "tlf.h"

/** possible multi
 *
 * name of possible multiplier and
 * list of belonging aliases */
typedef struct {
    char *name;
    GSList *aliases;
} possible_mult_t;


int addmult(struct qso_t *qso);
int check_mult(struct qso_t *qso);
void addmult_lan(void);
char *get_mult(int n);
int get_mult_count(void);
unsigned int get_matching_length(char *str, unsigned int n);
int get_exact_mult_index(char *str);
int init_and_load_multipliers(void);
int remember_multi(char *multiplier, int band, int mult_mode, bool check_only);
void init_mults();

#endif /* ADDMULT_H */
