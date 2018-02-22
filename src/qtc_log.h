/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013           Ervin Hegedüs - HA2OS <airween@gmail.com>
 *
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


#ifndef LOG_RECV_QTC_TO_DISK_H
#define LOG_RECV_QTC_TO_DISK_H

#include "cabrillo_utils.h"

void store_qtc(char *loglineptr, int direction, char *filename);
int log_recv_qtc_to_disk(int qtcnr);
int log_sent_qtc_to_disk(int qtcnr);
void make_qtc_logline(struct read_qtc_t qtc_line, char *fname);

#endif /* end of include guard: LOG_RECV_QTC_TO_DISK_H */
