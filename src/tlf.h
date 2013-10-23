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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>

#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#ifndef TLF_H
#define TLF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#define NO_KEYER 0
#define LPT_KEYER 1 	/* deprecated */
#define COM1_KEYER 2	/* deprecated */
#define NET_KEYER 3
#define MFJ1278_KEYER 4
#define ORION_KEYER 5
#define K2_KEYER 6
#define GMFSK 7

#define SINGLE 0        /* single op */
#define MULTI 1         /* multi op / single tx */

#define TELNET_INTERFACE 1
#define TNC_INTERFACE 2
#define NETWORK_INTERFACE 3
#define FIFO_INTERFACE 4

#define NOCLUSTER 0     /*  no cluster info  */
#define MAP 1           /*  show spots */
#define CLUSTER 3 		/*  full cluster info  */
#define FREQWINDOW 4    /* M/M frequencies */

#define SHORTCW 1       /*  short  cw characters in  exchange  (e.g. 0 = T,  9 = N) */
#define LONGCW 0

#define SEARCHWINDOW 1  /* searchflg on */

#define CQ 1			/* cqmode   on */
#define S_P 0			/* S&P mode  on  */

#define SEND_DE 1		/* de_mode on */
#define CONTEST 1		/* contest mode on */

#define FILTER_ANN 1	/*  filter announcements */
#define FILTER_DX 3
#define FILTER_ALL 0
#define FILTER_TALK 2

#define CWMODE 0
#define SSBMODE 1
#define DIGIMODE 2

#define  BAND160 32
#define  BAND80 16
#define  BAND40 8
#define  BAND20 4
#define  BAND15 2
#define  BAND10 1

enum {
    BANDINDEX_160 = 0,
    BANDINDEX_80,
    BANDINDEX_40,
    BANDINDEX_30,
    BANDINDEX_20,
    BANDINDEX_17,
    BANDINDEX_15,
    BANDINDEX_12,
    BANDINDEX_10,
    NBANDS 		// not yet used everywhere, 
};			// many places have hardcode 9 
			// (or 8, being the maximum band index)
			//
extern int inxes[NBANDS];  /*< conversion from BANDINDEX to BAND-mask,
			see addmult.c */


/* display color sets */
enum {
    C_HEADER = 2,
    C_BORDER,
    C_INPUT,
    C_DUPE,
    C_WINDOW,
    C_LOG 
};

#define NORMCOLOR C_INPUT
#define ISDUPE 1
#define NODUPE 0

#define  MAX_QSOS 20000       /* internal qso array */
#define  MAX_DATALINES 1000   /* from ctydb.dat  */
#define  MAX_CALLS 5000      /*  max nr of calls in dupe array */
#define  MAX_MULTS 1000        /* max nr of mults in mults array */
#define	MAX_SPOTS 200		/* max nr. of spots in spotarray */
#define MAX_CALLMASTER 50000 /* max number of calls in callmaster array */

#define EDITOR_JOE 0
#define EDITOR_VI 1
#define EDITOR_E3 2
#define EDITOR_MC 3

#define  CW_SPEEDS	"06121416182022242628303234363840424446485060"

#define LOGLINELEN (88)		/* Length of logline in logfile
				   (including linefeed) */
#if defined (TLN_LOGLINE)
/* do nothing, already defined */
#else
#define TLN_LOGLINE
struct tln_logline {
    struct tln_logline *next;
    struct tln_logline *prev;
    char *text;
    int attr;
} ;
#endif

typedef struct {
  int qsoline;	// qsos[INDEX]
  int flag;	// flag to mark for send
  int saved;	// indicates QTC has saved
  int sent;	// indicates QTC has sent at least once
  char qtc[25];
} t_qtcline;

typedef struct {
  int serial;	// qtc serial
  int count;	// nr of qtc line in block
  int marked;	// nr of marked to send
  int totalsent; // nr of sent qtc's
  char callsign[15];  // current callsign; helps to detect if QSO has dropped
  t_qtcline qtclines[10];
} t_qtclist;

#define QTC_SENT_LOG	"QTC_sent.log"
#define QTC_RECV_LOG	"QTC_recv.log"

void refreshp();

#endif /* TLF_H */

