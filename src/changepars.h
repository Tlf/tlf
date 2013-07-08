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

#include <curses.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "messagechange.h"
#include "listmessages.h"
#include "tlf.h"
#include "set_tone.h"
#include "editlog.h"
#include "logview.h"
#include "show_help.h"
#include "writecabrillo.h"
#include "writeparas.h"
#include "showpxmap.h"
#include "setparameters.h"
#include "muf.h"
#include "parse_logcfg.h"
#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif
#include "gettxinfo.h"
#include "netkeyer.h"
#include "splitscreen.h"
#include "audio.h"

#define RITCLEAR 1

int changepars  (void);
int networkinfo(void);
int multiplierinfo(void);
