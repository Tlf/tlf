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
// OBJECT ../src/printcall.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/focm.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/score.o
// OBJECT ../src/bands.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/ui_utils.o
// OBJECT ../src/utils.o

bool lan_active = false;

/* dummies */
void refresh_comment(void) {}
void time_update(void) {}
void show_rtty(void) {}
void OnLowerSearchPanel(int x, char *str) {}
void sendmessage(const char *msg) {}
void send_standard_message(int msg) {}
void add_local_spot(void) {}
void keyer(void) {}
void qtc_main_panel(int direction) {}
void rst_recv_up() {}
void rst_recv_down() {}
void stoptx() {}
void speedup() {}
void speeddown() {}
void vk_play_file() {}
int recall_exchange(void) { return 0; }
int GetCWSpeed(void) { return 0; }
int send_lan_message(int opcode, char *message) { return 0; }
void clusterinfo(void) {}
void clear_display(void) {}
void refresh_splitlayout() {}

int setup_default(void **state) {
    serial_section_mult = false;
    sectn_mult = false;
    current_qso.callupdate = g_malloc0(MAX_CALL_LENGTH + 1);
    current_qso.normalized_comment = g_malloc0(MAX_CALL_LENGTH + 1);
    current_qso.section = g_malloc0(MAX_SECTION_LENGTH + 1);
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

	assert_string_equal(current_qso.normalized_comment,
			    getex_arrlss[i].expected_normalized_comment);
	assert_string_equal(mult1_value, getex_arrlss[i].expected_mult1_value);
        assert_string_equal(current_qso.callupdate, getex_arrlss[i].expected_callupdate);

	g_free(input);
    }
}


typedef struct {
    char *input;
    char *expected_normalized_comment;
    char *expected_callupdate;
} getex_cqww_t;

static getex_cqww_t getex_cqww[] = {
    // exchange format:
    // <zone> <call_fix> <zone_fix>
    {
	"",     // empty
	"00", ""
    },
    {
	"12",     // plain
	"12", ""
    },
    {
	"  12",     // leading space
	"12", ""
    },
    {
	"  5",     // single digit
	"05", ""
    },
    {
	"12 34",     // corrected
	"34", ""
    },
    {
	"12 K1AB",     // with call
	"12", "K1AB"
    },
    {
	"12 F1AB 3",     // with call and correction
	"03", "F1AB"
    },
#if 0
    {
	"12 7 SK1AB",     // with correction and call
	"07", "SK1AB"
    },
#endif
    {
	"12 K1AB/4",     // call with region
	"12", "K1AB/4"
    },
    {
	"12 G/K1AB/QRP",    // complexer call
	"12", "G/K1AB/QRP"
    },
};

void test_getexchange_cqww(void **state) {
    contest = lookup_contest("CQWW");

    char *input;

    for (int i = 0; i < LEN(getex_cqww); ++i) {
	input = g_strdup(getex_cqww[i].input);

	checkexchange(input, false);

	assert_string_equal(current_qso.normalized_comment,
			    getex_cqww[i].expected_normalized_comment);
	assert_string_equal(current_qso.callupdate, getex_cqww[i].expected_callupdate);

	g_free(input);
    }
}

static getex_arrlss_t getex_serial_section[] = {
    // exchange format:
    // <serial> <section> [call_fix]
    {
	"",     // empty
	"", "", ""
    },
    {
	"123",  // only serial
	"", "", ""
    },
    {
	" 12 N",    // serial, leading space, partial section
	"", "", ""
    },
    {
	" 12  NA",  // serial, leading space, valid section
	"  12 NA", "NA", ""
    },
    {
	"12NA",     // valid section without delimiting space
	"  12 NA", "NA", ""
    },
    {
	"123  NA EA0XYZ",   // with call update
	" 123 NA", "NA", "EA0XYZ"
    },
    {
	"123S EA0XYZ",  // single letter section, no space, with call update
	" 123 S", "S", "EA0XYZ"
    },
    {
	"1234 K89",     // section with digits
	"1234 K89", "K89", ""
    },
    {
	"1234K89",      // section with digits, no space
	"1234 K89", "K89", ""
    },
    {
	"123 4K89",     // section starting with digit
	" 123 4K89", "4K89", ""
    },
#if 0   // section too long
    {
	"8 67K89 DL/EA0XYZ",    // Sonder-DOK with call update
	"   8 67K89", "67K89", "DL/EA0XYZ"
    },
#endif
};

void test_getexchange_serial_section(void **state) {
    contest = lookup_contest("Unknown");
    serial_section_mult = true;
    strcpy(multsfile, TOP_SRCDIR "/share/ea_sections");
    init_and_load_multipliers();

    // add some extra sections (DOKs)
    extern void add_mult_line(char *line);
    add_mult_line("K89");
    add_mult_line("4K89");
    add_mult_line("67K89");
    add_mult_line("50ABCD");

    char *input;

    for (int i = 0; i < LEN(getex_serial_section); ++i) {
	input = g_strdup(getex_serial_section[i].input);

	checkexchange(input, false);

	assert_string_equal(current_qso.normalized_comment,
			    getex_serial_section[i].expected_normalized_comment);
	assert_string_equal(mult1_value, getex_serial_section[i].expected_mult1_value);
	assert_string_equal(current_qso.callupdate, getex_serial_section[i].expected_callupdate);

	g_free(input);
    }
}

static getex_arrlss_t getex_sectn_mult[] = {
    // exchange format:
    // <section> [call_fix]
    {
	"",     // empty
	"", "", ""
    },
    {
	" N",    // leading space, partial section
	"", "", ""
    },
    {
	"   NA",  // leading space, valid section
	"NA", "NA", ""
    },
    {
	"NA",     // valid section without space
	"NA", "NA", ""
    },
    {
	"NA EA0XYZ",   // with call update
	"NA", "NA", "EA0XYZ"
    },
    {
	"S EA0XYZ",  // single letter section, no space, with call update
	"S", "S", "EA0XYZ"
    },
    {
	"K89",     // section with digits
	"K89", "K89", ""
    },
    {
	"4K89",     // section starting with digit
	"4K89", "4K89", ""
    },
#if 0   // section too long
    {
	" 67K89 DL/EA0XYZ",    // Sonder-DOK with call update
	"67K89", "67K89", "DL/EA0XYZ"
    },
#endif
};

void test_getexchange_sectn_mult(void **state) {
    contest = lookup_contest("Unknown");
    sectn_mult = true;
    strcpy(multsfile, TOP_SRCDIR "/share/ea_sections");
    init_and_load_multipliers();

    // add some extra sections (DOKs)
    extern void add_mult_line(char *line);
    add_mult_line("K89");
    add_mult_line("4K89");
    add_mult_line("67K89");
    add_mult_line("50ABCD");

    char *input;

    for (int i = 0; i < LEN(getex_sectn_mult); ++i) {
	input = g_strdup(getex_sectn_mult[i].input);

	checkexchange(input, false);

	assert_string_equal(current_qso.normalized_comment,
			    getex_sectn_mult[i].expected_normalized_comment);
	assert_string_equal(mult1_value, getex_sectn_mult[i].expected_mult1_value);
	assert_string_equal(current_qso.callupdate, getex_sectn_mult[i].expected_callupdate);

	g_free(input);
    }
}

void test_getexchange_serial_grid4(void **state) {
    contest = lookup_contest("Unknown");
    serial_grid4_mult = true;

    char *input;

    input = g_strdup("012 JN97AB");

    checkexchange(input, false);

    assert_string_equal(current_qso.normalized_comment, "  12 JN97");
    assert_string_equal(mult1_value, "JN97");
    assert_string_equal(current_qso.callupdate, "");

    g_free(input);
}
