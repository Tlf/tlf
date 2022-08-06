
#include <glib.h>
#include <hamlib/rig.h>
#include <stdbool.h>

#include <config.h>

#include "tlf.h"
#include "tlf_curses.h"

extern const char *argp_program_version;

extern mystation_t my;			// all about my station
extern char whichcontest[];
extern contest_config_t *contest;	// contest configuration

extern GPtrArray *qso_array;		// array of parsed QSOs
					// note that not every log line needs
					// to be a QSO, it could also be a
					// comment, starting with a semicolon.
					// Than is_comment field in qso_t
					// struct gets set

extern mults_t multis[MAX_MULTS]; 	// array of multipliers worked so far
extern int nr_multis;			// number of entries in mults[]
extern int multscore[NBANDS];		// number of multipliers worked per
					// band; index is
					// BANDINDEX_160 ... BANDINDEX_10
					// note: until 200409111,
					// index was 0...5 for only
					// the non-warc bands!

extern int nr_worked;			// number of worked station
					// entries in worked[]
extern worked_t worked[MAX_CALLS]; 	// worked stations

extern int countries[MAX_DATALINES];	// for every country, a bitfield
					// indicating bands on which it has
					// been worked

extern int bandinx;			// band we're currently working on

extern struct ie_list *main_ie_list;

extern char logfile[];
extern bool iscontest;

extern bool country_mult;

extern struct qso_t current_qso;
extern char hiscall_sent[CALL_SIZE];
extern int resend_call;
extern char sentcall[CALL_SIZE];
extern int total;
extern int qso_points;
extern int qsos_per_band[NBANDS];
extern int zones[MAX_ZONES];
extern bool serial_section_mult;
extern bool serial_grid4_mult;
extern bool sectn_mult;
extern bool sectn_mult_once;
extern bool dx_arrlsections;
extern bool wysiwyg_multi;
extern bool wysiwyg_once;
extern char wpx_prefix[];
extern int zonescore[NBANDS];
extern int countryscore[NBANDS];
extern int qsonum;
extern int countrynr;
extern int w_cty;
extern int ve_cty;
extern bool pfxmult;
extern bool pfxmultab;
extern int minute_timer;
extern int unique_call_multi;
extern int generic_mult;

extern char logline_edit[5][LOGLINELEN + 1];
#define logline0 logline_edit[0]
#define logline1 logline_edit[1]
#define logline2 logline_edit[2]
#define logline3 logline_edit[3]
#define logline4 logline_edit[4]

extern char band[NBANDS][4];
extern freq_t bandfrequency[NBANDS];

extern struct tm *time_ptr;

extern char cqzone[];
extern char ituzone[];
extern char continent[];
extern bool itumult;

extern int new_mult;
extern char proposed_exchange[80];

extern char lan_logline[];
extern char logfile[];
extern char qsonrstr[];
extern int lan_mutex;
extern bool lan_active;
extern int highqsonr;


extern RIG *my_rig;
extern cqmode_t cqmode;
extern int trxmode;
extern int myrig_model;
extern rmode_t rigmode;
extern freq_t freq;
extern char lastqsonr[];
extern bool cqwwm2;
extern char lastcall[];
extern char recvd_rst[];
extern char sent_rst[];
extern char last_rst[];
extern bool wazmult;
extern int addcallarea;
extern int new_cty;
extern int new_zone;
extern bool new_pfx;
extern bool no_rst;
extern rmode_t digi_mode;
extern int minitest;    // minitest period length in seconds, 0 if not used
extern int portnum;
extern int lan_port;
extern int txdelay;
extern int weight;
extern int cw_bandwidth;
extern int cwpoints;
extern int ssbpoints;
extern int continentlist_points;
extern int dx_cont_points;
extern int my_cont_points;
extern int packetinterface;
extern int use_bandoutput;
extern int cluster;
extern int nodes;
extern int multlist;
extern int xplanet;
extern int cwkeyer;
extern int digikeyer;
extern int cwstart;
extern int early_started;
extern int zonedisplay;
extern int rigptt;
extern int k_ptt;
extern int k_pin14;
extern int tune_seconds;
extern int sending_call;
extern int exclude_multilist_type;
extern bool partials;
extern bool use_part;
extern int shortqsonr;
extern bool rit;
extern bool showscore_flag;
extern bool searchflg;
extern bool nob4;
extern bool demode;
extern bool ctcomp;
extern bool show_time;
extern bool use_rxvt;
extern bool time_master;
extern bool noautocq;
extern bool no_arrows;
extern bool exc_cont;
extern bool sc_sidetone;
extern bool logfrequency;
extern bool bmautoadd;
extern bool bmautograb;
extern bool serial_or_section;
extern bool portable_x2;
extern bool clusterlog;
extern bool sprint_mode;
extern int timeoffset;
extern bool keyer_backspace;
extern int netkeyer_port;
extern int cqdelay;
extern int serial_rate;
extern int tnc_serial_rate;
extern int countrylist_points;
extern int my_country_points;
extern bool lowband_point_mult;
extern bool landebug;
extern int dupe;
extern int block_part;
extern int miniterm;
extern int announcefilter;
extern int nr_of_spots;
extern int fdSertnc;
extern int commentfield;

extern float fixedmult;

extern int bandweight_points[NBANDS];
extern int bandweight_multis[NBANDS];

extern pfxnummulti_t pfxnummulti[MAXPFXNUMMULT];
extern int pfxnummultinr;

extern char message[][80];
extern char *digi_message[];
extern char ph_message[14][80];
extern char tncportname[];
extern char multsfile[];
extern char markerfile[];
extern char countrylist[255][6];
extern char continent_multiplier_list[7][3];
extern char controllerport[];           // port for multi-mode controller
extern char modem_mode[];
extern char sc_volume[];
extern char clusterlogin[];
extern char rigconf[];
extern char keyer_device[10];
extern char netkeyer_hostaddress[];
extern char pr_hostaddress[];
extern char synclogfile[];
extern char exchange_list[40];
extern char rttyoutput[];
extern char spot_ptr[MAX_SPOTS][82];
extern char lastmsg[];
extern char exchange[40];
#ifdef HAVE_LIBXMLRPC
extern char fldigi_url[50];
#endif

extern char *cabrillo;
extern char *editor_cmd;
extern char *rigportname;
extern char *config_file;

extern int bandindexarray[];
extern int tlfcolors[8][2];

extern char fkey_header[60];

extern SCREEN *mainscreen;

extern bool mult_side;
extern bool countrylist_only;
extern bool mixedmode;
extern bool qso_once;
extern bool noleadingzeros;
extern bool ignoredupe;
extern bool continentlist_only;
extern bool debugflag;
extern bool trx_control;
extern bool nopacket;
extern bool verbose;

extern double DEST_Lat;
extern double DEST_Long;
