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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curses.h>
#include "startmsg.h"

#include "tlf.h"
#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif


#define SERIAL_PORT "/dev/ttyS0"

#define RIG_BUFFERSIZE 8000

#define N_RIGMODE_USB 0
#define N_RIGMODE_LSB 1
#define N_RIGMODE_CW 3

int sendqrg(void);
#ifdef HAVE_LIBHAMLIB
int init_tlf_rig (void);
int close_tlf_rig (RIG *my_rig);
#endif
int init_native_rig(void);
int close_native_rig(void);
float native_rig_get_freq (int  my_rig);
int  native_rig_set_mode(int rignumber,  int mode);
int  native_rig_set_freq(int rignumber,  int outfreq);
int  native_rig_reset_rit(int rignumber);

