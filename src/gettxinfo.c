/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
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

	/* ------------------------------------------------------------
 	*      get trx info  
 	*
 	*--------------------------------------------------------------*/

#include "gettxinfo.h"
#include <curses.h>
#include "sendqrg.h"

  int gettxinfo(void)
  {

#ifdef HAVE_LIBHAMLIB
  extern RIG *my_rig;
  extern freq_t rigfreq;
  extern freq_t outfreq;
   extern int cw_bandwidth;
   extern int nobandchange;
#else
 extern float rigfreq;
 extern int outfreq;
#endif
  extern float freq;
  extern int bandinx;
  extern float bandfrequency[];

  extern int rignumber;
  extern int trx_control;
  extern int trxmode;

	int retval = 0;
	int qrg = 0;
	char qrg_string[8];
	static int oldbandinx;

void send_bandswitch(int freq);

  if (trx_control != 1)
  	return(0);



 if (outfreq == 0) {

#ifdef HAVE_LIBHAMLIB              // Code for Hamlib interface
	if (rignumber < 2000)
		retval =  rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);
	else
		rigfreq = native_rig_get_freq(rignumber);		//ORION

#else
		rigfreq = (float) native_rig_get_freq(rignumber);		//ORION
#endif

	if (rigfreq > 1800.0) {
    		freq = rigfreq/1000.0;
    		qrg = (int) rigfreq/1000;
    	}

    qrg_string[7] = '\0';

	switch (qrg)
	{
		case 1800 ... 1900 :{
	  		bandinx = 0;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	  	case 3500 ... 4000 :{
	  		bandinx = 1;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	   	case 7000 ... 7200 : {
	  		bandinx = 2;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	    	case 10100 ... 10150 : {
	  		bandinx = 3;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	    case 14000 ... 14350 : {
	  		bandinx = 4;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	    case 18068 ... 18168 : {
	  		bandinx = 5;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	  	case 21000 ... 21450 : {
	  		bandinx = 6;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	    case 24890 ... 24990 : {
	  		bandinx = 7;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}
	    	case 28000 ... 29700 : {
	  		bandinx = 8;
	  		bandfrequency[bandinx] = freq;
	  		break;
	 	}


	}
	if (bandinx != oldbandinx)  // band change on trx
	{
		oldbandinx = bandinx;
		send_bandswitch((int) freq);

		if (rignumber < 2000) {

#ifdef HAVE_LIBHAMLIB              // Code for Hamlib interface

			if (trxmode == SSBMODE) {
				if (freq < 14000.0 )
					retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,  RIG_PASSBAND_NORMAL);
				else
					retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_USB,  RIG_PASSBAND_NORMAL);

				if (retval != RIG_OK)  {
					mvprintw(24,0,"Problem with rig link: set mode!\n");
					refresh();
					sleep(1);
				}
			}else if (trxmode == DIGIMODE) {
					retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,  RIG_PASSBAND_NORMAL);

				if (retval != RIG_OK)  {
					mvprintw(24,0,"Problem with rig link: set mode!\n");
					refresh();
					sleep(1);
				}

			}else {
//					retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  RIG_PASSBAND_NORMAL);
					if (cw_bandwidth == 0) {
						if (nobandchange != 1) {
							retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  RIG_PASSBAND_NORMAL);
						}
					}
					else {
						if (nobandchange != 1) {
							retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  cw_bandwidth);
						}
					}

				if (retval != RIG_OK)  {
					mvprintw(24,0,"Problem with rig link: set mode!\n");
					refresh();
					sleep(1);
				}

			}
#endif
		} else {			// native rig driver
			if (trxmode == SSBMODE) {
				if (freq < 14000.0 )
					retval =  native_rig_set_mode(rignumber, N_RIGMODE_LSB);
				else
					retval =  native_rig_set_mode(rignumber, N_RIGMODE_USB);

				if (retval != 0)  {
					mvprintw(24,0,"Problem with rig link: set mode!\n");
					refresh();
					sleep(1);
				}
			}else if (trxmode == DIGIMODE) {
					retval =  native_rig_set_mode(rignumber, N_RIGMODE_LSB);

				if (retval != 0)  {
					mvprintw(24,0,"Problem with rig link: set mode!\n");
					refresh();
					sleep(1);
				}

			}else {
					retval =  native_rig_set_mode(rignumber, N_RIGMODE_CW);

				if (retval != 0)  {
					mvprintw(24,0,"Problem with rig link: set mode!\n");
					refresh();
					sleep(1);
				}

			}



		}

	}

   } else if (outfreq == SETCWMODE)  {

	if (rignumber < 2000) {
#ifdef HAVE_LIBHAMLIB              // Code for Hamlib interface
		if (cw_bandwidth == 0) {
			if (nobandchange != 1) {
				retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  RIG_PASSBAND_NORMAL);
			}
		}
		else {
			if (nobandchange != 1) {
				retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_CW,  cw_bandwidth);
			}
		}
#endif
	}else {
		retval =  native_rig_set_mode(rignumber, N_RIGMODE_CW);
	}
			if (retval != 0)  {
				mvprintw(24,0,"Problem with rig link!\n");
				refresh();
				sleep(1);
			}

		outfreq = 0;


   } else if (outfreq == SETSSBMODE)  {
	if (rignumber < 2000) {
#ifdef HAVE_LIBHAMLIB              // Code for Hamlib interface
		if (freq > 13999.9)
			retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_USB,  RIG_PASSBAND_NORMAL);
		else
			retval =  rig_set_mode(my_rig, RIG_VFO_CURR, RIG_MODE_LSB,  RIG_PASSBAND_NORMAL);

		if (retval != RIG_OK)  {
				mvprintw(24,0,"Problem with rig link!\n");
				refresh();
				sleep(1);
		}

#endif
	}else {
		if (freq > 13999.9)
			retval =  native_rig_set_mode(rignumber,  N_RIGMODE_USB);
		else
			retval =  native_rig_set_mode(rignumber,  N_RIGMODE_LSB);

		if (retval != 0)  {
				mvprintw(24,0,"Problem with rig link!\n");
				refresh();
				sleep(1);
		}
	}

		outfreq = 0;




   }  else if (outfreq == RESETRIT){
	if (rignumber < 2000) {
#ifdef HAVE_LIBHAMLIB              // Code for Hamlib interface
		retval =  rig_set_rit(my_rig, RIG_VFO_CURR, 0);

			if (retval != RIG_OK)  {
				mvprintw(24,0,"Problem with rig link!\n");
				refresh();
				sleep(1);
			}
#endif
	} else
		native_rig_reset_rit(rignumber);

	outfreq = 0;

   } else {

		if (rignumber < 2000) {
#ifdef HAVE_LIBHAMLIB              // Code for Hamlib interface
			retval = rig_set_freq(my_rig, RIG_VFO_CURR, outfreq);

			if (retval != RIG_OK)  {
				mvprintw(24,0,"Problem with rig link: set frequency!\n");
				refresh();
				sleep(1);
			}

#endif
		} else
			retval = native_rig_set_freq( rignumber, outfreq);

			if (retval != 0)  {
				mvprintw(24,0,"Problem with rig link: set frequency!\n");
				refresh();
				sleep(1);
			}

			outfreq = 0;

   }

   return(0);
  }

