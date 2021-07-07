/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2011 Thomas Beierlein <tb@forth-ev.de>
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


#ifndef DXCC_H
#define DXCC_H

#include <stdbool.h>
#include <glib.h>

typedef struct {
    char *pfx;
    unsigned short cq;
    unsigned short itu;
    short dxcc_index;
    float lat;
    float lon;
    char *continent;
    float timezone;
    bool exact;
} prefix_data;

typedef struct {
    char *countryname;
    unsigned short cq;
    unsigned short itu;
    char *continent;
    float lat;
    float lon;
    float timezone;
    char *pfx;
    bool starred;
} dxcc_data;

extern bool have_exact_matches;
extern char cty_dat_version[12];

void prefix_init(void);

unsigned int prefix_count(void);

prefix_data *prefix_by_index(unsigned int index);

int find_full_match(const char *call);

int find_best_match(const char *call);

void prefix_add(char *pfxstr);

void dxcc_init(void);

unsigned int dxcc_count(void);

dxcc_data *dxcc_by_index(unsigned int index);

void dxcc_add(char *dxcc_line);

int load_ctydata(char *filename);
#endif 	/* DXCC_H */
