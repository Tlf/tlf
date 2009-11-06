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
 	/* ------------------------------------------------------------
 	*        Set Cw sidetone
 	*
 	*--------------------------------------------------------------*/

#include "set_tone.h"
#include "tlf.h"
#include "cwkeyer.h"
#include "clear_display.h"
#include "write_tone.h"
#include "netkeyer.h"

 int set_tone(void)
 {

 extern char speedstr[];
 extern int speed;
 extern char tonestr[];
 extern int trxmode;

 char speedbuf[3] = "  ";

if (trxmode != CWMODE)
	return(1);

strncpy(speedbuf, speedstr  + (2 * speed)  ,2);
speedbuf[2] =  '\0';


 nicebox (4,  40, 1, 6 , "Tone");
 mvprintw(5, 42,  "");
 attron(COLOR_PAIR(7) | A_STANDOUT);
 echo();
 getnstr(tonestr,  3);
 noecho();
 tonestr[4] = '\0';

 write_tone();


  return(0);
 }

void write_tone(void)
{

	extern int trxmode;
 	extern char tonestr[];
	extern int keyerport;
	extern int cfd;

 	int mute = 1;


	if (keyerport == COM1_KEYER) {

		if (ioctl (cfd, CWTONE, atoi(tonestr))) {
	
			mvprintw(24,0, "keyer not active; switching to SSB");
			trxmode = SSBMODE;
			clear_display();
		}

	}  else {

 		if (keyerport == LPT_KEYER) {

			if (atoi(tonestr) == 0)
 				mute = 1;
 			else
 				mute = 0;

			if (mute == 0) {
		    		if	(ioctl(cfd,CWKEYER_IOCSMONI, 1 ))
				{
					printf(" Unable to switch on  sidetone\n");
				}

				if	(ioctl(cfd, CWKEYER_IOCSFREQ, atoi(tonestr)))
				{
					mvprintw(24,0, "keyer not active; switching to SSB");
					trxmode = SSBMODE;

				}
		
			}else {
		
				if	(ioctl(cfd,CWKEYER_IOCSMONI, 0 ))
				{
					mvprintw(24,0, "keyer not active; switching to SSB");
					trxmode = SSBMODE;

				}	
		
			}
		
		}  else {
			if (netkeyer(K_TONE, tonestr) < 0)   {
				mvprintw(24,0, "keyer not active; switching to SSB");
				trxmode = SSBMODE;
			} 
	
		}
	}
}	


