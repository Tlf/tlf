#include "tlf.h"

extern char qsos[MAX_QSOS][82];		// array of log lines of QSOs so far; note that not every log line needs to be a QSO, it could also be a comment, starting with a semicolon
extern int nr_qsos;			// number of lines in qsos[]

extern char mults[MAX_MULTS][12];	// array of multipliers worked so far
extern int multarray_nr;		// number of entries in mults[]
extern int mult_bands[];		// bitfield indicating on which band(s) the multiplier has been worked; bits assigned according to BAND160..BAND10 defined in tlf.h
extern int multscore[NBANDS];		// number of multipliers worked per band; index is BANDINDEX_160 ... BANDINDEX_10
					// note: until 200409111, index was 0...5 for only the non-warc bands!

extern char callarray[MAX_CALLS][20];	// list of all calls worked so far
extern int callarray_nr;		// number of entries in callarray[]
extern int call_band[MAX_CALLS];	// for each of them, a bitfield indicating bands
extern int call_country[MAX_CALLS];	// for each call, the country

extern int countries[MAX_DATALINES];	// for every country, a bitfield indicating bands on which it has been worked

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
extern char call_exchange[MAX_CALLS][12];
extern int wpx;
extern char prefixes_worked[MAX_CALLS][6];
extern int arrlss;
extern int serial_section_mult;
extern int serial_grid4_mult;
extern int sectn_mult;
extern int dx_arrlsections;
extern int wysiwyg_multi;
extern int wysiwyg_once;
extern int wysiwygmults;
extern int multcount;
extern char pxstr[];
extern int nr_of_px;
extern int zonescore[6];
extern int countryscore[6];
extern int qsonum;
extern int countrynr;
extern int w_cty;
extern int ve_cty;
extern int pfxmult;
extern int minute_timer;


// extern char logline0[81];
// extern char logline1[81];
// extern char logline2[81];
// extern char logline3[81];
// extern char logline4[];
extern char logline_edit[5][82];
#define logline0 logline_edit[0]
#define logline1 logline_edit[1]
#define logline2 logline_edit[2]
#define logline3 logline_edit[3]
#define logline4 logline_edit[4]

extern int stop_backgrnd_process;
extern char band[9][4];
extern struct tm *time_ptr;

extern int inxes[9]; //  = {BAND160,BAND80,BAND40,0,BAND20,0,BAND15,0,BAND10} ;	// from addmult.c

extern char prefixlines[MAX_DBLINES][17];
extern char cqzone[];
extern char ituzone[];
extern char zonearray[MAX_DBLINES][3];
extern char ituarray[MAX_DBLINES][3];
extern int dataindex[MAX_DBLINES];
extern char continent[];
extern char datalines[MAX_DATALINES][81];
extern char zone_export[];
extern int itumult;

extern int max_multipliers;
extern char mults_possible[MAX_MULTS][12];
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
extern char lastcomment[];
extern int addcallarea;
extern int addcty;
extern char zone_fix[];
extern int universal;
extern int arrl_fd;
extern int one_point;
extern int two_point;
extern int three_point;
extern int dxped;
extern char pointstring[];
extern int addzone;




