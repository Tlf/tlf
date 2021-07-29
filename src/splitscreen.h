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

#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

#include <stdbool.h>

extern bool lanspotflg;
extern pthread_mutex_t spot_ptr_mutex;

int init_packet(void) ;
void cleanup_telnet(void);
int packet(void);
void send_cluster(void);
void addtext(char *s);
int receive_packet(void);
void refresh_splitlayout();


#ifdef SPLITSCREEN_H_PRIVATE

#define ENTRYROWS 2
#define BUFFERSIZE 256

#define SCROLLSIZE (LINES/4*3+1)
#define DEFAULTTLN_LOGLINES 300

#define NORMAL_ATTR 0
#define MINE_ATTR 1
#define ENTRY_ATTR 2

#define STATE_EDITING 0
#define STATE_VIEWING 1


void addlog(char *s);
int logattr(void);
char *firstlog(void);
char *lastlog(void);
char *nextlog(void);
char *prevlog(void);
void start_editing(void);
void delete_prev_char(void);
void right_arrow(void);
void left_arrow(void);
void move_eol(void);
void gather_input(char *s);
int walkup(void);
int walkdn(void);
int pageup(int lines);
int pagedn(int lines);
void viewbottom(void);
void viewtop(void);
void resume_editing(void);
void viewlog(void);
int edit_line(int c);
void sanitize(char *s);

#endif /* SPLITSCREEN_H_PRIVATE */

#endif /* end of include guard: SPLITSCREEN_H */
