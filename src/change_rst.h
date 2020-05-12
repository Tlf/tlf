/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2020 Thomas Beierlein <dl1jbe@darc.de>
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

#ifndef CHANGE_RST_H
#define CHANGE_RST_H

#include <stdbool.h>

extern bool change_rst;

void rst_init(char *init_string) ;
void rst_reset(void);
void rst_set_strings();
void rst_recv_up();
void rst_recv_down();
void rst_sent_up();
void rst_sent_down();

#endif
