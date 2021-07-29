/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2015 Ervin Hegedus <airween@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#ifndef QTCVARS_H
#define QTCVARS_H

#include "globalvars.h"

#define QTC_SENT_LOG	"QTC_sent.log"
#define QTC_RECV_LOG	"QTC_recv.log"
#define QTC_META_LOG	"QTC_meta.log"

#define QTC_LINES 10
#define QTC_RY_LINE_NR 12

#define RECV 1		// QTC RECV direction
#define SEND 2		// QTC SEND direction
#define QTC_CAP 4	// QTC CAPABLE station
#define QTC_LATER 8	// QTC LATER
#define QTC_NO 16	// NO QTC for/from this station

#define QTC_CALL_SIZE 15

typedef struct {
    int qsoline;	// qsos[INDEX]
    int flag;	// flag to mark for send
    int saved;	// indicates QTC has saved
    int sent;	// indicates QTC has sent at least once
    char qtc[25]; // QTC line by concatenated fields
    char senttime[16];  // sent time: YY-Mon-dd HH:MM\0
} t_qtcline;

typedef struct {
    int serial;	// qtc serial
    int count;	// nr of qtc line in block
    int marked;	// nr of marked to send
    int totalsent; // nr of sent qtc's
    char callsign[QTC_CALL_SIZE];  // current callsign; helps to detect if QSO has dropped
    t_qtcline qtclines[QTC_LINES];
} t_qtclist;

typedef struct {
    int status;	// received, failed, nothing
    char time[5];	// time of qso
    char callsign[QTC_CALL_SIZE]; // callsign
    char serial[5]; // qso serial
    int confirmed; // qtc had confirmed
    char receivedtime[16]; // received time: YY-Mon-dd HH:MM\0
} t_qtcrecline;

typedef struct {
    int serial;
    int count;
    int confirmed;
    int sentcfmall;
    char callsign[QTC_CALL_SIZE];
    t_qtcrecline qtclines[QTC_LINES];
} t_qtcreclist;

typedef struct {
    char content[50];
    int attr;		// meta attr: 0 => not nopied, 1 => copied
} t_qtc_ry_line;

extern int next_qtc_qso;		// the next non-sent QSO, which can
					// be send next as QTC
extern int qsoflags_for_qtc[MAX_QSOS];	// array of flag to log lines of QSOs
					// for QTC's;  this is an array of
					// flags, which marks when a QSO
					// sent as QTC
extern int qtcdirection;		// 1: RECV, 2: SEND, 3: BOTH
extern t_qtclist qtclist;		// the QTC list to send
extern t_qtcreclist qtcreclist;		// the QTC list which received
extern int nr_qtcsent;			// number of sent QTC blocks; this used
					// at the QTC header, eg. 23/10 - here the 23
					// is the value of this variable
extern t_qtc_ry_line qtc_ry_lines[QTC_RY_LINE_NR];	// when QTC is set,
					// and mode is RTTY, then the modem
					// lines stored this array
extern int qtc_ry_currline;		// current line of QTC RTTY modem
extern int qtc_ry_capture;		// enable or disable QTC RTTY capturing
extern int qtc_ry_copied;		// stores the number of copied lines i
					// from QTC RTTY terminal to QTC window

extern int qtcrec_record;		// do we record the received QTCs
extern char qtcrec_record_command[2][50]; 	// command to start recording
extern char qtcrec_record_command_shutdown[50]; // coomand to stop recording

/* arras of CW/Digimode messages for QTC receive and send */
extern char qtc_recv_msgs[12][80];
extern char qtc_send_msgs[12][80];
/* arras of SSB fiel names for QTC receive and send */
extern char qtc_phrecv_message[14][80];
extern char qtc_phsend_message[14][80];

extern char qtc_cap_calls[40];
extern int qtc_auto_filltime;		// set QTC auto filltime; when DX sent first
					// QTC line, the first two digits will be copied to others
extern int qtc_recv_lazy;		// ignore check of received QTC lines
#endif /* end of include guard: QTCVARS_H */
