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

#include "tlf.h"
#include "clear_display.h"
#include "printcall.h"
#include "callinput.h"
#include "getexchange.h"
#include "sendbuf.h"
#include "sendspcall.h"
#include "log_to_disk.h"
#include "keyer.h"
#include "recall_exchange.h"
#include "sendqrg.h"
#include "lancode.h"


int logit(void);
void refresh_comment(void);
