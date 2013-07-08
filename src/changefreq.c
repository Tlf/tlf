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

#include "changefreq.h"

void change_freq (void) {

    extern float freq;
    extern int trx_control;
#ifdef HAVE_LIBHAMLIB
    extern freq_t outfreq;
#else
    extern int outfreq;
#endif
    int brkflg = 0;
    int x;

    if (trx_control == 0)
	return;

    curs_set(0);

    while (1) {

	freq_display();

	if (outfreq == 0) {
	    x = onechar();

	    switch (x) {

	    case 152:{		//up      
#ifdef HAVE_LIBHAMLIB
		    outfreq = (freq_t) (freq * 1000);
#else
		    outfreq = (int) (freq * 1000);
#endif
		    outfreq += 100;

		    break;
		}
	    case 153:{		//down
#ifdef HAVE_LIBHAMLIB
		    outfreq = (freq_t) (freq * 1000);
#else
		    outfreq = (int) (freq * 1000);
#endif
		    outfreq -= 100;

		    break;
		}
	    case 154:{		//right
#ifdef HAVE_LIBHAMLIB
		    outfreq = (freq_t) (freq * 1000);
#else
		    outfreq = (int) (freq * 1000);
#endif
		    outfreq += 20;

		    break;
		}
	    case 155:{		//left
#ifdef HAVE_LIBHAMLIB
		    outfreq = (freq_t) (freq * 1000);
#else
		    outfreq = (int) (freq * 1000);
#endif
		    outfreq -= 20;

		    break;
		}
	    case 156:{		//left
#ifdef HAVE_LIBHAMLIB
		    outfreq = (freq_t) (freq * 1000);
#else
		    outfreq = (int) (freq * 1000);
#endif
		    outfreq += 500;

		    break;
		}
	    case 157:{		//left
#ifdef HAVE_LIBHAMLIB
		    outfreq = (freq_t) (freq * 1000);
#else
		    outfreq = (int) (freq * 1000);
#endif
		    outfreq -= 500;

		    break;
		}

	    default:{
		    brkflg = 1;
		    break;
		}

	    }
	}

	if (brkflg == 1) {
	    brkflg = 0;
	    break;
	}

	freq_display();

	time_update();

	usleep(100000);

    }
    curs_set(1);
}
