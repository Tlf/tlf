/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013-2014      Thomas Beierlein <tb@forth-ev.de>
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


#ifndef _SCORE_H
#define _SCORE_H
#include <stdbool.h>
#include "tlf.h"

int score_wpx(struct qso_t * qso);
int score_cqww(struct qso_t * qso);
int score_arrlfd(struct qso_t * qso);
int score_arrldx_usa(struct qso_t * qso);
int score_stewperry(struct qso_t * qso);
int score(struct qso_t *qso);
void score_qso(struct qso_t *qso);
int score2(char *line);
bool country_found(char prefix[]);
bool is_in_countrylist(int countrynr);
bool is_in_continentlist(char *continent);

#endif /* end of include guard: _SCORE_H */
