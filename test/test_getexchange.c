#include "test.h"

#include "../src/getexchange.h"
#include "../src/globalvars.h"
#include "../src/tlf.h"
#include "../src/setcontest.h"
#include "../src/addmult.h"
#include "../src/ui_utils.h"

// OBJECT ../src/getexchange.o
// OBJECT ../src/addmult.o
// OBJECT ../src/addpfx.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/focm.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/score.o
// OBJECT ../src/bands.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/ui_utils.o
// OBJECT ../src/utils.o

extern char callupdate[];

bool lan_active = false;

/* dummies */
void refresh_comment(void) {}
void time_update(void) {}
void show_rtty(void) {}
void OnLowerSearchPanel(int x, char *str) {}
void sendmessage(const char *msg) {}
void send_standard_message(int msg) {}
void addspot(void) {}
void keyer(void) {}
void qtc_main_panel(int direction) {}
void rst_recv_up() {}
void rst_recv_down() {}
void stoptx() {}
void speedup() {}
void speeddown() {}
void play_file() {}
int recall_exchange(void) { return 0; }
int GetCWSpeed(void) { return 0; }
int send_lan_message(int opcode, char *message) { return 0; }
int check_qra(char *qra) { return 0; }
void clusterinfo(void) {}
void clear_display(void) {}
void refresh_splitlayout() {}


int setup_default(void **state) {
    ssexchange[0] = 0;
    section[0] = 0;
    callupdate[0] = 0;
    return 0;
}

typedef struct {
    char *input;
    char *expected_ssexchange;
    char *expected_section;
    char *expected_callupdate;
} getex_arrlss_t;

static getex_arrlss_t getex_arrlss[] = {
    // exchange format:
    // <serial> <precedent> <call> <check> <section>
    {
	"",     // empty
	"          ", "", ""
    },
    {
	"123",  // only serial
	" 123      ", "", ""  // leading space added
    },
    {
	" 8",    // short serial + leading space
	"   8      ", "", "" // leading space added
    },
    {
	"123 X",    // junk after serial
	" 123      ", "", ""  // junk dropped
    },
    {
	"123 Q",   // valid precedent
	" 123 Q    ", "", ""
    },
    {
	"123 M N1XYZ",  // valid precedent + simple call
	" 123 M    ", "", "N1XYZ"  // call dropped
    },
    {
	"123 M N1XYZ/3",    // valid precedent + complex call
	" 123 M    ", "", "N1XYZ/3"
    },
    {
	"123 M NA1XYZ 7",   // partial check
	" 123 M    ", "", "NA1XYZ"  // partial check not added
    },
    {
	"123 M N1XYZ 73",   // valid check
	" 123 M 73 ", "", "N1XYZ"
    },
    {
	"123 M N1XYZ 73 A",   // partial section
	" 123 M 73 ", "", "N1XYZ"
    },
    {
	"123 M N1XYZ 73 AK",   // full exchange
	" 123 M 73 AK", "AK", "N1XYZ"
    },
    {
	"123 M 73 A",   // partial section w/o call
	" 123 M 73 ", "", ""
    },
    {
	"123 M 73 AS",   // invalid section
	" 123 M 73 ", "", ""    // section not considered
    },
    {
	"123 M 73 AZ",   // valid section
	" 123 M 73 AZ", "AZ", ""
    },
    {
	"123  M AR",    // no check
	" 123 M    AR", "AR", ""
    },
    {
	"M AR",         // no serial
	"     M    AR", "AR", ""
    },
};


void test_getexchange_arrlss(void **state) {
    contest = lookup_contest("ARRL_SS");
    strcpy(multsfile, TOP_SRCDIR "/share/arrlsections");
    init_and_load_multipliers();

    char *input;

    for (int i = 0; i < sizeof(getex_arrlss) / sizeof(getex_arrlss_t); ++i) {
	input = g_strdup_printf("%-20s", getex_arrlss[i].input);

	strcpy(comment, input); //FIXME should not be needed
	callupdate[0] = 0;

	checkexchange(input);

	assert_string_equal(ssexchange, getex_arrlss[i].expected_ssexchange);
	assert_string_equal(section, getex_arrlss[i].expected_section);
	assert_string_equal(callupdate, getex_arrlss[i].expected_callupdate);

	g_free(input);
    }
}


