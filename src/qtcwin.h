/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013 Ervin Hegedüs - HA2OS <airween@gmail.com>
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
#include "tlf.h"


typedef struct {
  t_qtcreclist qtcreclist;	// received QTC list
  int active;			// current field index
} t_qtcfieldset;


int qtc_main_panel(int direction);
int showfield(int fidx);
int modify_field(int pressed);
int delete_from_field(int dir);
int shift_right(char * fieldval);
int shift_left(char * fieldval, int shift);
int show_status(int idx);
int number_fields();
int readqtcfromfile();
int clear_help_block();
int show_help_msg();
int put_qtc();
//int move_cursor(int dir);

