/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012           Thomas Beierlein <tb@forth-ev.de>
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

	/* ------------------------------------------------------------
	 *   write cabrillo header
	 *
	 *--------------------------------------------------------------*/

#include "getsummary.h"
#include "showscore.h"
#include <glib.h>

extern char call[];

void ask(char *buffer, char *what)
{

    attron(A_STANDOUT);
    mvprintw(15, 1,
	     "                                                                              ");
    nicebox(14, 0, 1, 78, what);
    attron(A_STANDOUT);
    mvprintw(15, 1, "");

    echo();
    getnstr(buffer, 78);
    noecho();
    g_strstrip(buffer);
}


int getsummary(FILE *fp)
{
    char buffer[80];

    fprintf(fp, "START-OF-LOG: 3.0\n");
    fprintf(fp, "CREATED-BY: tlf-%s\n", VERSION);

    ask(buffer, "Contest: (CQ-WW-CW/SSB, CQ-WPX-CW/SSB, ARRL-DX-CW/SSB)");
    fprintf(fp, "CONTEST: %s\n", buffer);

    fprintf(fp, "CALLSIGN: %s", call);		/* !!! trailing \n at call */

    ask(buffer, "Category-Assisted: (ASSISTED, NON-ASSISTED");
    fprintf(fp, "CATEGORY-ASSISTED: %s\n", buffer);

    ask(buffer, "Bands: (ALL,160M,80M,40M,20M,15M,10M)");
    fprintf(fp, "CATEGORY-BAND: %s\n", buffer);

    ask(buffer, "Mode: (CW,SSB,RTTY,MIXED)");
    fprintf(fp, "CATEGORY-MODE: %s\n", buffer);

    ask(buffer, "Category-Operator:(SINGLE-OP, MULTI-OP, CHECKLOG)");
    fprintf(fp, "CATEGORY-OPERATOR: %s\n", buffer);

    ask(buffer, "POWER: (HIGH,LOW,QRP)");
    fprintf(fp, "CATEGORY-POWER: %s\n", buffer);
    
    ask(buffer, "Category-Station: (FIXED, MOBILE, PORTABLE, ROVER, EXPEDITION, HQ, SCHOOL");
    if (*buffer != '\0')
	fprintf(fp, "CATEGORY-STATION: %s\n", buffer);

    ask(buffer, "Category-Time: (6-HOURS, 12-HOURS, 24-HOURS)");
    if (*buffer != '\0')
	fprintf(fp, "CATEGORY-TIME: %s\n", buffer);

    ask(buffer, "Transmitter: (ONE, TWO, LIMITED, UNLIMITED, SWL)");
    if (*buffer != '\0')
	fprintf(fp, "CATEGORY-TRANSMITTER: %s\n", buffer);

    ask(buffer, "Category-Overlay: (ROOKIE, TB-WIRES, NOVICE-TECH, OVER-50)");
    if (*buffer != '\0')
	fprintf(fp, "CATEGORY-OVERLAY: %s\n", buffer);

    fprintf(fp, "CLAIMED-SCORE: %d\n", get_nr_of_points() * get_nr_of_mults());

    ask(buffer, "Club: ");
    if (*buffer != '\0')
	fprintf(fp, "CLUB: %s\n", buffer);

    ask(buffer, "Location: (section, IOTA name, RDA, State/Province, ...)");
    if (*buffer != '\0')
	fprintf(fp, "LOCATION: %s\n", buffer);


    ask(buffer, "Operator name: ");
    fprintf(fp, "NAME: %s\n", buffer);

    ask(buffer, "ADDRESS: ");
    fprintf(fp, "ADDRESS: %s\n", buffer);

    ask(buffer, "ADDRESS(2): ");
    if (*buffer != '\0')
	fprintf(fp, "ADDRESS: %s\n", buffer);

    ask(buffer, "ADDRESS(3): (use any text editor to insert more ADDRESS lines)");
    if (*buffer != '\0')
	fprintf(fp, "ADDRESS: %s\n", buffer);

    ask(buffer, "List of Operators: (space delimited)");
    fprintf(fp, "OPERATORS: %s\n", buffer);

    ask(buffer, "OFFTIME: (yyyy-mm-dd hhmm yyyy-mm-dd hhmm)"); 
    if (*buffer != '\0')
	fprintf(fp, "OFFTIME: %s\n", buffer);

    ask(buffer, "SOAPBOX: (use any text editor to include more lines)");
    if (*buffer != '\0')
	fprintf(fp, "SOAPBOX: %s\n", buffer);

    return (0);
}
