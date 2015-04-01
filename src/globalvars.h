#include "tlf.h"
#include <glib.h>

extern char qsos[MAX_QSOS][LOGLINELEN+1];// array of log lines of QSOs so far;
					// note that not every log line needs
					// to be a QSO, it could also be a
					// comment, starting with a semicolon
extern int nr_qsos;			// number of lines in qsos[]

extern char mults[MAX_MULTS][12];	// array of multipliers worked so far
extern int multarray_nr;		// number of entries in mults[]
extern int mult_bands[];		// bitfield indicating on which band(s)
					// the multiplier has been worked;
					// bits assigned according to
					// BAND160..BAND10 defined in tlf.h
extern int multscore[NBANDS];		// number of multipliers worked per
					// band; index is
					// BANDINDEX_160 ... BANDINDEX_10
					// note: until 200409111,
					// index was 0...5 for only
					// the non-warc bands!

extern int nr_worked;			// number of worked station
					// entries in worked[]
extern struct worked_t worked[MAX_CALLS]; // worked stations

extern int countries[MAX_DATALINES];	// for every country, a bitfield
					// indicating bands on which it has
					// been worked

extern int bandinx;			// band we're currently working on

extern char logfile[];
extern int contest;
extern int cqww;
extern int arrldx_usa;
extern int pacc_pa_flg;
extern int country_mult;
extern int other_flg;
extern char hiscall[20];
extern int total;
extern int band_score[NBANDS];
extern int zones[41];
extern int wpx;
extern char prefixes_worked[MAX_CALLS][6];
extern int arrlss;
extern int serial_section_mult;
extern int serial_grid4_mult;
extern int sectn_mult;
extern int dx_arrlsections;
extern int wysiwyg_multi;
extern int wysiwyg_once;
extern char pxstr[];
extern int nr_of_px;
extern int nr_of_px_ab;
extern int pfxs_per_band[NBANDS];
extern int zonescore[6];
extern int countryscore[6];
extern int qsonum;
extern int countrynr;
extern int w_cty;
extern int ve_cty;
extern int pfxmult;
extern int pfxmultab;
extern int minute_timer;
extern int stewperry_flg;
extern char myqra[7];

extern char logline_edit[5][LOGLINELEN+1];
#define logline0 logline_edit[0]
#define logline1 logline_edit[1]
#define logline2 logline_edit[2]
#define logline3 logline_edit[3]
#define logline4 logline_edit[4]

extern int stop_backgrnd_process;
extern char band[9][4];
extern struct tm *time_ptr;

extern int inxes[NBANDS]; //  = {BAND160,BAND80,BAND40,0,BAND20,0,BAND15,0,BAND10} ;
		    // from addmult.c

extern char cqzone[];
extern char ituzone[];
extern char continent[];
extern char zone_export[];
extern int itumult;

extern GPtrArray *mults_possible;	/* growing array of possible mutlipliers */
extern char ssexchange[];
extern int shownewmult;
extern char comment[];

extern char  lan_logline[];
extern char logfile[];
extern char qsonrstr[];
extern int lan_mutex;
extern int lan_active;
extern int exchange_serial;
extern int highqsonr;


extern int trxmode;
extern char lastqsonr[];
extern int cqwwm2;
extern char thisnode;
extern char lastcall[];
extern char my_rst[];
extern char his_rst[];
extern char section[];
extern int wazmult;
extern int addcallarea;
extern int addcty;
extern char zone_fix[];
extern int universal;
extern int arrl_fd;
extern int one_point;
extern int two_point;
extern int three_point;
extern int dxped;
extern int addzone;

extern int qsoflags_for_qtc[MAX_QSOS];	// array of flag to log lines of QSOs for QTC's;
					// this is an array of flags, which marks when a QSO sent as QTC
extern int nr_qsosflags_for_qtc;	// number of lines in qsoflags_for_qtc[]
extern int next_qtc_qso;		// the next non-sent QSO, which can be send next as QTC
extern t_qtclist qtclist;		// the QTC list to send
extern int nr_qtcsent;
extern t_qtcreclist qtcreclist;		// the QTC list which received

extern int qtcdirection;				// QTC direction - 1: RECV, 2: SEND
extern t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];	// array of PFX_NUM_MULTIS parameter
extern int pfxnummultinr;				// length of array of PFX_NUM_MULTIS parameter
extern int continentlist_only;				// CONTINENT_LIST_ONLY parameter
extern t_qtc_ry_line qtc_ry_lines[QTC_RY_LINE_NR];	// when QTC is set, and mode is RTTY, then the modem lines stored this array
extern int qtc_ry_currline;				// current line of QTC RTTY modem
extern int qtc_ry_capture;				// enable or disable QTC RTTY capturing
extern int qtc_ry_copied;				// stores the number of copied lines from QTC RTTY terminal to QTC window
