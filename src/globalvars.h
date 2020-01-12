#ifndef TLF_H
# include "tlf.h"
#endif

#include <glib.h>

#include <hamlib/rig.h>

extern char qsos[MAX_QSOS][LOGLINELEN + 1];
					// array of log lines of QSOs so far;
					// note that not every log line needs
					// to be a QSO, it could also be a
					// comment, starting with a semicolon
extern int nr_qsos;			// number of lines in qsos[]

extern struct mults_t multis[MAX_MULTS]; // array of multipliers worked so far
extern int nr_multis;			// number of entries in mults[]
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
extern int band_score[NBANDS];		// QSO/band
extern int zones[MAX_ZONES];
extern int wpx;
extern int arrlss;
extern int serial_section_mult;
extern int serial_grid4_mult;
extern int sectn_mult;
extern int dx_arrlsections;
extern int wysiwyg_multi;
extern int wysiwyg_once;
extern char pxstr[];
extern int zonescore[NBANDS];
extern int countryscore[NBANDS];
extern int qsonum;
extern int countrynr;
extern int w_cty;
extern int ve_cty;
extern int pfxmult;
extern int pfxmultab;
extern int minute_timer;
extern int stewperry_flg;
extern char myqra[7];
extern int unique_call_multi;

extern char logline_edit[5][LOGLINELEN + 1];
#define logline0 logline_edit[0]
#define logline1 logline_edit[1]
#define logline2 logline_edit[2]
#define logline3 logline_edit[3]
#define logline4 logline_edit[4]

extern char band[NBANDS][4];
extern struct tm *time_ptr;
extern struct tm time_ptr_cabrillo;

extern char cqzone[];
extern char ituzone[];
extern char continent[];
extern char zone_export[];
extern int itumult;

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
extern rmode_t rigmode;
extern freq_t freq;
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
extern int do_cabrillo;
extern rmode_t digi_mode;

extern char message[][80];
extern char *digi_message[];
