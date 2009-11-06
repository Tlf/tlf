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

#include "sendqrg.h"
#include "tlf.h"

int send_bandswitch(int trxqrg);

int sendqrg (void) {

extern char hiscall[];
extern int trx_control;
#ifdef HAVE_LIBHAMLIB
//extern RIG *my_rig;
extern freq_t outfreq;
#else
//extern int my_rig;
extern int outfreq;
#endif

float trxqrg;

if (trx_control != 1)
	return(0);                  /* nothing to do here */

trxqrg = atof(hiscall);

switch ((int)trxqrg) {

	case 0 ... 1799 : {
		trxqrg = 0;
		break;
	}
	case 1800 ... 2000:
	case 3500 ... 4000:
	case 7000 ... 7300:
	case 10100 ... 10150:
	case 14000 ... 14350:
	case 18068 ... 18168:
	case 21000 ... 21450:
	case 24890 ... 24990:
	case 28000 ... 29700:
	{
#ifdef HAVE_LIBHAMLIB
		outfreq = (freq_t)(trxqrg*1000);
#else
		outfreq = (int)(trxqrg*1000);
#endif

	send_bandswitch((int) trxqrg);

		break;
	}
	default:
		trxqrg = 0;

}

 return(trxqrg);
}

/********************************************************************************************/
#ifdef HAVE_LIBHAMLIB                    //code for Hamlib interface

int init_tlf_rig (void)
{
	extern RIG *my_rig;
	extern rig_model_t myrig_model;
	extern freq_t rigfreq;		/* frequency  */
	extern freq_t outfreq;
	extern char rigconf[];
	extern int serial_rate;
	extern int rig_port;
	extern char rigportname[];
	extern int verbose;
	extern int debugflag;

	int retcode;		/* generic return code from functions */

	const char  *ptt_file=NULL, *dcd_file=NULL;
	ptt_type_t ptt_type = RIG_PTT_NONE;
	dcd_type_t dcd_type = RIG_DCD_NONE;

	char *cnfparm, *cnfval;
	int rigconf_len, i;


 	/*
	 * allocate memory, setup & open port
	 */
			rig_set_debug(RIG_DEBUG_NONE);

			my_rig = rig_init(myrig_model);

			if (!my_rig) {
				shownr("Unknown rig num %d", (int)myrig_model);
				 sleep(2);
				return(-1);
			}
			else {
				if (strlen(rigportname) > 1) {
						rigportname[strlen(rigportname)-1] = '\0'; // remove '\n'
						strncpy(my_rig->state.rigport.pathname, rigportname, FILPATHLEN);
				}
				else {
					if (rig_port == 0)
						strncpy(my_rig->state.rigport.pathname, "/dev/ttyS0", FILPATHLEN);
					else
						strncpy(my_rig->state.rigport.pathname, "/dev/ttyS1", FILPATHLEN);
				}

			}

			if (ptt_type != RIG_PTT_NONE)
				my_rig->state.pttport.type.ptt = ptt_type;
			if (dcd_type != RIG_DCD_NONE)
				my_rig->state.dcdport.type.dcd = dcd_type;
			if (ptt_file)
				strncpy(my_rig->state.pttport.pathname, ptt_file, FILPATHLEN);
			if (dcd_file)
				strncpy(my_rig->state.dcdport.pathname, dcd_file, FILPATHLEN);

			my_rig->state.rigport.parm.serial.rate = serial_rate ;

			cnfparm = cnfval = rigconf;
			rigconf_len = strlen(rigconf);
			for (i=0; i<rigconf_len; i++) {
				/* FIXME: left hand value of = cannot be null */
				if (rigconf[i] == '=' && cnfval == cnfparm) {
					cnfval = rigconf+i+1;
					rigconf[i] = '\0';
					continue;
				}
				if (rigconf[i] == ',' || i+1 == rigconf_len) {
					if (cnfval <= cnfparm) {
			  			showstring("Missing parm value in RIGCONF: ", rigconf);
						refresh();
						sleep(2);
						return(-1);
					}
					if (rigconf[i] == ',')
						rigconf[i] = '\0';
					retcode = rig_set_conf(my_rig, rig_token_lookup(my_rig, cnfparm), cnfval);
					if (retcode != RIG_OK) {
			  			showmsg("rig_set_conf: error  ");
						refresh();
						sleep(5);
						return(-1);
					}

					cnfparm = cnfval = rigconf+i+1;
					continue;
				}
			}

			retcode = rig_open(my_rig);

			if (retcode != RIG_OK) {
	  			showmsg("rig_open: error ");
		//		mvprintw(7,0,"rig_open: error = %s \n", rigerror(retcode));
				refresh();
				sleep(2);
				return(-1);
			}

			 retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

			if (retcode != RIG_OK)  {
				showmsg("Problem with rig link!");
				sleep(1);
			} else {
				shownr("freq =", (int) rigfreq);

				if (verbose == 1) sleep(1);
			 }
			if (debugflag == 1){			// debug routines
			 sleep(10);
			 
			 retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);

			if (retcode != RIG_OK)  {
				showmsg("Problem with rig get freq!");
				sleep(1);
			} else {
				shownr("freq =", (int) rigfreq);

				if (verbose == 1) sleep(1);
			 }
			 sleep(10);


			 outfreq = 14000000;	//test set frequency

			 retcode = rig_set_freq(my_rig, RIG_VFO_CURR, outfreq);

			if (retcode != RIG_OK)  {
				showmsg("Problem with rig set freq!");
				sleep(1);
			}else {
				showmsg("Rig set freq ok!");
				sleep(1);

			}

			outfreq = 0;
			 
			 retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &rigfreq);	// read qrg

			if (retcode != RIG_OK)  {
				showmsg("Problem with rig get freq!");
				sleep(1);
			} else {
				shownr("freq =", (int)rigfreq);
				if (verbose == 1) sleep(1);
			 }
			 sleep(10);


} // end debug

 if (verbose == 1 || debugflag ==1) sleep(1);

	return(0);
}

int close_tlf_rig(RIG *my_rig) {

	rig_close(my_rig); /* close port */
	rig_cleanup(my_rig); /* if you care about memory */

	printf("port %s closed ok \n",SERIAL_PORT);

return(0);
}

#endif         // end code for Hamlib interface


/* ------------------------------------------ native get mode ----------------------------- */

int native_rig_get_mode(rignumber) {

extern int native_rig_fd;
extern int trxmode;

int i;
char line[20] = "";
char inputline[80] = "";
const char eom[2] = {'\015' , '\0'};

	strcpy (line, "?RMM");
	strcat (line, eom);

	if (native_rig_fd > 0)
		write (native_rig_fd, line, strlen(line));

	usleep(30000);

	inputline[0] = '\0';

	if(native_rig_fd > 0) {

			i = read (native_rig_fd, inputline, sizeof(inputline)-1);

			if (strncmp(inputline, "Z!",2) != 0) {
				if (strncmp(inputline, "@RMM", 4 )== 0) {
					if (inputline[4] == '0')
						trxmode = SSBMODE;
					else if (inputline[4] == '1')
						trxmode = SSBMODE;
					else
						trxmode = CWMODE;
					mvprintw(23, 30, "%s", inputline);
					refresh();
					sleep(1);
				}
			}
			else {
				mvprintw(24,0, "Rig communication error");
				refresh();
				sleep(2);
			}
	}


return(0);
}
/*--------------------------------- native rig init------------------------------------------------*/

int init_native_rig (void)
{
extern char rigportname[];
extern int serial_rate;
extern int native_rig_fd;
extern int rignumber;

int i;
struct termios termattribs;
char line[20] = "";
char inputline[80] = "";
const char eom[2] = {'\015' , '\0'};

		if (rigportname[strlen(rigportname) - 1] == '\n')
				rigportname[strlen(rigportname) -1 ] = '\0'; // remove \n

   		if ((native_rig_fd = open(rigportname, O_RDWR | O_NONBLOCK)) < 0) {
                	mvprintw(5,0,  "open of %s failed!!!", rigportname);
			refresh();
			sleep(2);
			return(-1);
    		}

	termattribs.c_iflag = IGNBRK | IGNPAR | IMAXBEL | IXOFF;
    	termattribs.c_oflag = 0;
   	termattribs.c_cflag = CS8 | CSTOPB | CREAD | CLOCAL;
 //	termattribs.c_cflag = CS8 | CREAD | CLOCAL;


    	termattribs.c_lflag = 0;		    /* Set some term flags */

    	/*  The ensure there are no read timeouts (possibly writes?) */
    	termattribs.c_cc[VMIN] = 1;
    	termattribs.c_cc[VTIME] = 0;

	switch(serial_rate) {

 			case   1200 : {
				cfsetispeed(&termattribs,B1200);	    /* Set input speed */
    				cfsetospeed(&termattribs,B1200);	    /* Set output speed */
			 break;
			}

			case   2400 : {
				cfsetispeed(&termattribs,B2400);	    /* Set input speed */
    				cfsetospeed(&termattribs,B2400);	    /* Set output speed */
			 break;
			 }

			case   4800 : {
				cfsetispeed(&termattribs,B4800);	    /* Set input speed */
    				cfsetospeed(&termattribs,B4800);	    /* Set output speed */
			 break;
			 }

			case   9600 : {
				cfsetispeed(&termattribs,B9600);	    /* Set input speed */
    				cfsetospeed(&termattribs,B9600);	    /* Set output speed */
			 break;
			 }
			case   57600 : {
				cfsetispeed(&termattribs,B57600);	    /* Set input speed */
    				cfsetospeed(&termattribs,B57600);	    /* Set output speed */
			 break;
			 }

			default:   {

				cfsetispeed(&termattribs,B9600);	    /* Set input speed */
    				cfsetospeed(&termattribs,B9600);	    /* Set output speed */
			}
	}

	tcsetattr(native_rig_fd,TCSANOW,&termattribs);  /* Set the serial port */


	strcpy (line, "XX");
	strcat (line, eom);

	if (native_rig_fd > 0)
		write (native_rig_fd, line, strlen(line));

	usleep(30000);


	if(native_rig_fd > 0) {

//			i = read (native_rig_fd, inputline, RIG_BUFFERSIZE-1);	### bug fix
			i = read (native_rig_fd, inputline, sizeof(inputline));

			if (strncmp(inputline, "Z!",2) != 0) {
				rignumber = 2000;				// ORION
				mvprintw(23,0, "%s",inputline);
				refresh();
				sleep(1);
			}
			else {
				mvprintw(23,0, "Rig communication not initialized");
				refresh();
				sleep(2);
			}
	}

	native_rig_get_mode(rignumber);


	return(0);
}
/* ----------------------------------------------- close native rig ----------------------------*/

int close_native_rig(void) {

extern int native_rig_fd;

	if (native_rig_fd > 0)
		close (native_rig_fd);

return(0);
}

/* ------------------------------------------ native rig get freqency --------------------------*/


float native_rig_get_freq (int rignumber) {  // ORION only

extern int native_rig_fd;
extern int rig_comm_error;
extern int rig_comm_success;

char line[20] = "";
char inputline[20] = "";
const char eom[2] = {'\015' , '\0'};		// ORION
int i, rigfreq;

	strcpy (line, "?AF");			// ORION
	strcat (line, eom);

	if (native_rig_fd > 0)
		write (native_rig_fd, line, strlen(line));

	usleep(100000);


	if(native_rig_fd > 0) {

			i = read (native_rig_fd, inputline, sizeof(inputline)-1);

			if (strncmp(inputline, "Z!",2) != 0) {
				if (strncmp(inputline, "@AF", 3) == 0 && strlen(inputline) == 12) {
					rigfreq = atof (inputline + 3);
					rig_comm_success++;
					return(rigfreq);
				}
				else {
					rig_comm_error++;
				}
			}
			else {
				mvprintw(24,0, "Rig communication error");
				refresh();
			}
			inputline[0]='\0';
	}

return (0.0);
}
/* ------------------------------------------ native set mode ----------------------------- */

int  native_rig_set_mode(int rignumber,  int mode){

extern int native_rig_fd;

char line[20] = "";
const char eom[2] = {'\015' , '\0'};		// ORION

switch (mode) {
	case N_RIGMODE_USB :
		strcpy (line, "*RMM0");			// ORION
		break;
	case N_RIGMODE_LSB :
		strcpy (line, "*RMM1");
		break;
	default :
		strcpy (line, "*RMM3");
}
	strcat (line, eom);

	if (native_rig_fd > 0)
		write (native_rig_fd, line, strlen(line));


return(0);
}



/* ------------------------------------------ native set frequency----------------------------- */

int  native_rig_set_freq(int rignumber,  int outfreq){

extern int native_rig_fd;

char line[20] = "";
const char eom[2] = {'\015' , '\0'};		// ORION

	sprintf(line, "%s%d", "*AF", outfreq);
	strcat (line, eom);

	if (native_rig_fd > 0)
		write (native_rig_fd, line, strlen(line));

	outfreq = 0;

return(0);
}
/* ------------------------------------------ native reset rit----------------------------- */

int  native_rig_reset_rit(int rignumber){

extern int native_rig_fd;
#ifdef HAVE_LIBHAMLIB                    //code for Hamlib interface
extern freq_t outfreq;
#else
extern int outfreq;
#endif

char line[20] = "*RMR0";
const char eom[2] = {'\015' , '\0'};		// ORION

	strcat (line, eom);

	if (native_rig_fd > 0)
		write (native_rig_fd, line, strlen(line));

	outfreq = 0;

return(0);
}

/* -------------------------------------- set cw speed --------------------------------- */

int orion_set_cw_speed(int speed) {

extern int native_rig_fd;

char buffer[10];

//sprintf(buffer, "%s%d%c%", "*CS", speed, '\015');	### bug fix
sprintf(buffer, "%s%d%c", "*CS", speed, '\015');

	if (native_rig_fd > 0)
		write (native_rig_fd, buffer, strlen(buffer));

return (0);
}

