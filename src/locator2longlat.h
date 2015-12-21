/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2013           Ervin Hegedus <airween@gmail.com>
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

/*
 * original code in HAMLIB
 *
 * src/locator.c header file
 *
 * author Stephane Fillod and the Hamlib Group
 * date 2000-2010
 *
 *  Hamlib Interface - locator, bearing, and conversion calls
 *
 *
 *  Hamlib Interface - locator and bearing conversion calls
 *  Copyright (c) 2001-2010 by Stephane Fillod
 *  Copyright (c) 2003 by Nate Bargmann
 *  Copyright (c) 2003 by Dave Hines
 *
 *
 *  Code to determine bearing and range was taken from the Great Circle,
 *  by S. R. Sampson, N5OWK.
 *  Ref: "Air Navigation", Air Force Manual 51-40, 1 February 1987
 *  Ref: "ARRL Satellite Experimenters Handbook", August 1990
 *
 *  Code to calculate distance and azimuth between two Maidenhead locators,
 *  taken from wwl, by IK0ZSN Mirko Caserta.
 *
 *  New bearing code added by N0NB was found at:
 *  http://williams.best.vwh.net/avform.htm#Crs
 */


#ifndef LOCATOR2LONGLAT_H
#define LOCATOR2LONGLAT_H


int locator2longlat(double *longitude, double *latitude, const char *locator);
int check_qra(char *qra);

#endif /* end of include guard: LOCATOR2LONGLAT_H */
