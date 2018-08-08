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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef DXCC_H
#define DXCC_H

typedef struct {
    char *pfx;
    short cq;
    short itu;
    short dxcc_index;
    float lat;
    float lon;
    char *continent;
    float timezone;
    char exact;
} prefix_data;

typedef struct {
    char *countryname;
    short cq;
    short itu;
    char *continent;
    float lat;
    float lon;
    float timezone;
    char *pfx;
    char starred;
} dxcc_data;

extern char have_exact_matches;

void prefix_init(void);

unsigned int prefix_count(void);

prefix_data *prefix_by_index(unsigned int index);

void prefix_add(char *pfxstr);

void dxcc_init(void);

unsigned int dxcc_count(void);

dxcc_data *dxcc_by_index(unsigned int index);

void dxcc_add(char *dxcc_line);

int load_ctydata(char *filename);
#endif 	/* DXCC_H */
