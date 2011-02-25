/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2011 Thomas Beierlein <tb@forth-ev.de>
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

#ifndef _BANDMAP_H
#define _BANDMAP_H

/** add a new spot to bandmap data 
 * \param call  	the call to add
 * \param frequ 	on which frequency heard
 * \param mode		actual mode
 * \param reason	- new cluster spot
 * 			- local announcement (Ctrl-A)
 * 			- cluster announcement (Ctrl-B)
 * 			- just worked in S&P
 */
bandmap_addspot( call, frequ, mode, reason);
/*
 * - if call already on that band and mode replace old entry with new one and
 *   set age to 0 otherwise add it to collection
 * - if other call on same frequency (with some tolerance) replace it and set
 *   age to 0
 * - round all frequencies from cluster to 100 Hz, remember all other exactly
 *   but display only rounded to 100 Hz - sort exact
 */

bandmap_age();
/*
 * - go through all entries
 *   + increment age
 *   + set state to new, normal, aged or aged_out
 *   + if aged_out drop it from collection
 */

bandmap_show();
/*
 * display depending on filter state
 * - all bands on/off
 * - all mode  on/off
 * - dupes     on/off
 *
 * If more entries to show than place in window, show around current frequency
 *
 * mark entries according to age, source and worked state. Mark new multis
 * - new 	brigth blue
 * - normal	blue
 * - aged	black
 * - worked	small caps
 * - new multi	underlined
 * - self announced stations
 *   		small preceeding letter for repoting station
 *
 * maybe show own frequency as dashline in other color 
 * (maybee green highlighted)
 * - highligth actual spot if near its frequency 
 *
 * Allow selection of one of the spots
 * - Ctrl-G as known
 * - '.' and cursor plus 'Enter'
 *
 * '.' goes into map, shows help line above and supports
 * - cursormovement
 * - 'ESC' leaves mode
 * - 'Enter' selects spot
 * - 'B', 'D', 'M' switches filtering for band, dupes and mode on or off.
 */


#endif
