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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define	MACRO_QSONUMBER		'#'
#define MACRO_CALL		'@'
#define	MACRO_RST		'~'
#define MACRO_SPEED_UP		'+'
#define MACRO_SPEED_DOWN	'-'

#define CWKEYER_IOC_MAGIC  'k'
#define CWKEYER_IOCRESET    _IO(CWKEYER_IOC_MAGIC, 0)


/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

#define CWKEYER_IOCSSPEED   	_IOW(CWKEYER_IOC_MAGIC,  1, int)
#define CWKEYER_IOCSFREQ    	_IOW(CWKEYER_IOC_MAGIC,  2, int)
#define CWKEYER_IOCSMONI	_IOW(CWKEYER_IOC_MAGIC,  3, int)
#define CWKEYER_IOCSWEIGTH	_IOW(CWKEYER_IOC_MAGIC,  4, int)
#define CWKEYER_IOCSTXDELAY	_IOW(CWKEYER_IOC_MAGIC,  5, int)
#define CWKEYER_IOCSFLUSH	_IOW(CWKEYER_IOC_MAGIC,  6, void *)
#define CWKEYER_IOCSCALL	_IOW(CWKEYER_IOC_MAGIC,  7, char[16])
#define CWKEYER_IOCSQSONUMBER	_IOW(CWKEYER_IOC_MAGIC,  8, char[16])
#define CWKEYER_IOCSRST		_IOW(CWKEYER_IOC_MAGIC,  9, char[16])
#define CWKEYER_IOCSPOSNEGKEY	_IOW(CWKEYER_IOC_MAGIC, 10, int)
#define CWKEYER_IOC_MAXNR 9

/*
 * Serial port keyer
 */

#define CWTONE  0xCC01
#define CWSPEED 0xCC02
#define CWMUTE  0xCC03
