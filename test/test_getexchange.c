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
    normalized_comment[0] = 0;
    section[0] = 0;
    callupdate[0] = 0;
    return 0;
}

typedef struct {
    char *input;
    char *expected_normalized_comment;
    char *expected_mult1_value;
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
	"123 X",    // junk after serial (invalid precedent)
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
	"123 M 2E0XYZ 73 IL",   // non-US/CA call
	" 123 M 73 IL", "IL", ""
    },
    {
	"123 M K/G1Q 73 GA",   // non recognized call
	" 123 M    ", "", ""
    },
    {
	"456M N1XYZ 73AK",   // less spaces
	" 456 M 73 AK", "AK", "N1XYZ"
    },
    {
	"9M73AK",            // squashed
	"   9 M 73 AK", "AK", ""
    },
    {
	"123AK1ABC73OH",     // squashed with call
	" 123 A 73 OH", "OH", "K1ABC"
    },
    {
	"123AK1ABC7KS",     // squashed with call, partial check
	" 123 A    KS", "KS", "K1ABC"
    },
    {
	"123AK1ABC/873OH",  // squashed with complex call
	" 123 A 73 OH", "OH", "K1ABC/8"
    },
    {
	"123AK1XBC/83LAX",  // squashed with complex call and partial check
	" 123 A    LAX", "LAX", "K1XBC/8"
    },
    {
	"123AL1ABC73NH",    // squashed with invalid call
	" 123 A 73 NH", "NH", ""
    },
    {
	"123AK1AUT",        // squashed with call only
	" 123 A    ", "", "K1AUT"
    },
    {
	"123 M 73 A",       // partial section w/o call
	" 123 M 73 ", "", ""
    },
    {
	"123 M 73 AS",      // invalid section
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

    for (int i = 0; i < LEN(getex_arrlss); ++i) {
	input = g_strdup_printf("%-20s", getex_arrlss[i].input);

	checkexchange(input, false);

	assert_string_equal(normalized_comment,
			    getex_arrlss[i].expected_normalized_comment);
	assert_string_equal(mult1_value, getex_arrlss[i].expected_mult1_value);
	assert_string_equal(callupdate, getex_arrlss[i].expected_callupdate);

	g_free(input);
    }
}


typedef struct {
    char *input;
    char *expected_zone_fix;
    char *expected_zone_export;
    char *expected_callupdate;
} getex_cqww_t;

static getex_cqww_t getex_cqww[] = {
    // exchange format:
    // <zone> <call_fix> <zone_fix>
    {
	"",     // empty
	"", "00"/*FIXME*/, ""
    },
    {
	"12",     // plain
	"", "12", ""
    },
    {
	"  12",     // leading space
	"12", "12", ""
    },
    {
	"  5",     // single digit
	"05", "05", ""
    },
    {
	"12 34",     // corrected
	"34", "34", ""
    },
    {
	"12 K1AB",     // with call
	"", "12", "K1AB"
    },
    {
	"12 F1AB 3",     // with call and correction
	"03", "03", "F1AB"
    },
    {
	"12 7 SK1AB",     // with correction and call
	"07", "07", "SK1AB"
    },
#if 0
    {
	"12 K1AB/4",     // call with region
	"", "", "K1AB/4"
    },
    {
	"12 G/K1AB/QRP",    // complexer call
	"", "", "G/K1AB/QRP"
    },
#endif
};

void test_getexchange_cqww(void **state) {
    contest = lookup_contest("CQWW");

    char *input;

    for (int i = 0; i < LEN(getex_cqww); ++i) {
	input = g_strdup_printf("%-20s", getex_cqww[i].input);
	printf("%s{\n", input);
	strcpy(comment, input); //FIXME should not be needed
	callupdate[0] = 0;
	zone_fix[0] = 0;
	zone_export[0] = 0;

	checkexchange(input, false);
	printf("zone_fix=|%s| zone_export=|%s|\n", zone_fix, zone_export);

	assert_string_equal(zone_fix, getex_cqww[i].expected_zone_fix);
	assert_string_equal(zone_export, getex_cqww[i].expected_zone_export);
	assert_string_equal(callupdate, getex_cqww[i].expected_callupdate);

	g_free(input);
    }
}
