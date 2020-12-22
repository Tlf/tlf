/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2014 Thomas Beierlein <tb@forth-ev.de>
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


#ifndef _FOC_H
#define _FOC_H

#include "tlf.h"

extern contest_config_t config_focm;

void foc_init(void);
int foc_score(char *call);
int foc_total_score();
void foc_show_scoring(int start_colmn);
void foc_show_cty();

#endif /* end of include guard: _FOC_H */
