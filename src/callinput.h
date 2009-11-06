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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "tlf.h"
#include "clear_display.h"
#include "onechar.h"
#include "stoptx.h"
#include "speedup.h"
#include "speeddown.h"
#include "sendbuf.h"
#include "scroll_log.h"
#include "addcall.h"
#include "makelogline.h"
#include "store_qso.h"
#include "qsonr_to_str.h"
#include "writeparas.h"
#include "printcall.h"
#include "time_update.h"
#include "cleanup.h"
#include "autocq.h"
#include "sendspcall.h"
#include "edit_last.h"
#include "changepars.h"
#include "deleteqso.h"
#include "note.h"
#include "prevqso.h"
#include "getctydata.h"
#include "showinfo.h"
#include "searchlog.h"
#include "calledit.h"
#include "muf.h"
#include "clusterinfo.h"
#include "grabspot.h"
#include "splitscreen.h"
#include "showpxmap.h"
#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif
#include "lancode.h"
#include "rtty.h"


int callinput(void);
int play_file(char *audiofile);

