#include "test.h"

#include "../src/globalvars.h"

#include "../src/tlf_curses.h"
#include "../src/tlf_panel.h"
#include "../src/searchlog.h"
#include "../src/dxcc.h"

// OBJECT ../src/searchlog.o
// OBJECT ../src/zone_nr.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/nicebox.o
// OBJECT ../src/qtcutil.o
// OBJECT ../src/printcall.o

extern WINDOW *search_win;
extern PANEL *search_panel;
extern int nr_bands;
extern int searchflg;
extern int use_part;
extern int partials;
extern int cqww;

extern char zone_export[];
extern char zone_fix[];

extern char searchresult[MAX_CALLS][82];
extern char result[MAX_CALLS][82];

extern int found_zone;

void handlePartials(void);
void filterLog();
int bandstr2line(char *buffer);

/*********************/
// mocks

// stoptx.c
int stoptx() {
    return 0;
}

// ui_utils.c
int modify_attr(int attr) {
    return attr;
}

// get_time.c
void get_time(void) {
}

// getpx.c
void getpx(char *checkcall) {
}

// dxcc.c
static dxcc_data dummy_dxcc = {
    "Noland",
    1,
    2,
    "NO",
    34,
    56,
    7,
    "QQ",
    false
};

dxcc_data *dxcc_by_index(unsigned int index) {
    return &dummy_dxcc;
}

// getctydata.c
int getctydata(char *checkcallptr) {
    return 0;
}

/*********************/
#define QSO1 " 40SSB 12-Jan-18 16:34 0006  SP9ABC         599  599  15                     1         "
#define QSO2 " 40CW  12-Jan-18 11:42 0127  K4DEF          599  599  05                     3   7026.1"
#define QSO3 " 40CW  12-Jan-18 16:34 0007  OE3UAI         599  599  15            OE       1         "
#define QSO4 " 80SSB 12-Jan-18 16:34 0008  UA3JK          599  599  16            UA  16   1         "
#define QSO5 " 80CW  12-Jan-18 16:34 0009  UA9LM          599  599  17            UA9 17   3         "
#define QSO6 " 80CW  12-Jan-18 16:36 0010  AA3BP          599  599  05            K   05   3         "

static void write_qsos() {
    int i;
    for (i = 0; i < MAX_QSOS; i++) {
	strcpy (qsos[i], "");
    }
    strcpy (qsos[0], QSO1);
    strcpy (qsos[1], QSO2);
    strcpy (qsos[2], QSO3);
    strcpy (qsos[3], QSO4);
    strcpy (qsos[4], QSO5);
    strcpy (qsos[5], QSO6);
}

int setup_default(void **state) {
    showmsg_spy = showstring_spy1 = showstring_spy2 = STRING_NOT_SET;
    arrlss = 0;
    dxped = 0;
    contest = 0;
    search_win = NULL;
    searchflg = SEARCHWINDOW;
    trxmode = CWMODE;

    partials = 1;
    use_part = 0;

    strcpy(zone_export, "");
    strcpy(zone_fix, "");

    found_zone = 0;

    clear_mvprintw_history();

    write_qsos();
    return 0;
}


static void check_mvprintw_output(int index, int y, int x, const char *text) {
    char buffer[7];
    sprintf(buffer, "%02d|%02d|", y, x);
    assert_memory_equal(mvprintw_history[index], buffer, 6);
    assert_string_equal(mvprintw_history[index] + 6, text);
}


// callmaster is checked first in current directory,
// create it there
static void write_callmaster(const char *content) {
    FILE *f = fopen("callmaster", "w");
    assert_non_null(f);
    fputs(content, f);
    fclose(f);
}

static void remove_callmaster() {
    unlink("callmaster");   // no need to check if file exists
}

int teardown_default(void **state) {
    remove_callmaster();
    return 0;
}

//void test_callmaster_no_file(void **state) {
//    remove_callmaster();
//    int n = load_callmaster();
//    assert_int_equal(n, 0);
//    assert_string_equal(showmsg_spy, STRING_NOT_SET);
//    assert_string_equal(showstring_spy1, STRING_NOT_SET);
//    assert_string_equal(showstring_spy2, STRING_NOT_SET);
//}

void test_callmaster_ok(void **state) {
    write_callmaster("# data\nA1AA\nA2BB\n\n");
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "A2BB");
}

void test_callmaster_ok_dos(void **state) {
    write_callmaster("# data\r\nA1AA\r\nA2BB\r\n\r\n");
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "A2BB");
}

void test_callmaster_ok_spaces(void **state) {
    write_callmaster(" # data \n A1AA \n A2BB \n\n");
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "A2BB");
}

void test_callmaster_ok_arrlss(void **state) {
    write_callmaster("# data\nA1AA\nG0CC\nN2BB\n\n");
    arrlss = 1;
    int n = load_callmaster();
    assert_int_equal(n, 2);
    assert_string_equal(CALLMASTERARRAY(0), "A1AA");
    assert_string_equal(CALLMASTERARRAY(1), "N2BB");
}

void test_init_search_panel_no_contest(void **state) {
    InitSearchPanel();
    assert_int_equal(nr_bands, 9);
}

void test_init_search_panel_contest(void **state) {
    contest = 1;
    InitSearchPanel();
    assert_int_equal(nr_bands, 6);
}

void test_init_search_panel_dxped(void **state) {
    dxped = 1;
    InitSearchPanel();
    assert_int_equal(nr_bands, 9);
}


/* testing searchlog for refactoring */
void test_searchlog_pickup_call(void **state) {
    strcpy (hiscall, "UA");
    filterLog("");
    assert_int_equal (strncmp(searchresult[0], QSO3, 80), 0);
    assert_int_equal (strncmp(searchresult[1], QSO4, 80), 0);
    assert_int_equal (strncmp(searchresult[2], QSO5, 80), 0);
}

void test_searchlog_extract_data(void **state) {
    strcpy (hiscall, "UA");
    filterLog("");
    assert_string_equal (result[0], " 40CW  0007 OE3UAI       15            ");
    assert_string_equal (result[1], " 80SSB 0008 UA3JK        16            ");
}

void test_bandstr2line(void **state) {
    assert_int_equal( bandstr2line( " 10"), 1);
    assert_int_equal( bandstr2line( " 15"), 2);
    assert_int_equal( bandstr2line( " 20"), 3);
    assert_int_equal( bandstr2line( " 40"), 4);
    assert_int_equal( bandstr2line( " 80"), 5);
    assert_int_equal( bandstr2line( "160"), 6);
    assert_int_equal( bandstr2line( " 12"), 7);
    assert_int_equal( bandstr2line( " 17"), 8);
    assert_int_equal( bandstr2line( " 30"), 9);
}

/* testing pickup call suggestion for USEPARTIAL */
void test_UsePartialFromLog (void **state) {
    use_part = 1;
    strcpy(hiscall, "K4DE");
    filterLog();
    handlePartials();
    assert_string_equal( hiscall, "K4DEF");
}

void test_UsePartialFromLogNotUnique (void **state) {
    use_part = 1;
    strcpy(hiscall, "UA");
    filterLog();
    handlePartials();
    assert_string_equal( hiscall, "UA");
}

void test_UsePartialFromCallmaster(void **state) {
    write_callmaster("# data\nA1AA\nA2BB\n\n");
    load_callmaster();
    use_part = 1;
    strcpy(hiscall, "A1");
    filterLog();
    handlePartials();
    assert_string_equal( hiscall, "A1AA");
}

void test_UsePartialNotUnique(void **state) {
    write_callmaster("# data\nA1AA\nA2BB\nA3BB\n");
    load_callmaster();
    use_part = 1;
    strcpy(hiscall, "A3");
    filterLog();
    handlePartials();
    assert_string_equal( hiscall, "A3");
}


/* test lookup of zone - will be used for display if already worked
 * - normally determined from countryinformation
 * - can be picked up from previous qso if we have full match
 * - or overwritten in exchange field */
void test_ZoneFromCountry(void **state) {
    cqww = 1;
    strcpy(zone_export, "15");
    strcpy( hiscall, "OH2");
    searchlog(hiscall);
    assert_int_equal (found_zone, 15);
}

void test_ZoneFromExchange(void **state) {
    cqww = 1;
    strcpy(zone_fix, "14");
    strcpy(zone_export, "15");
    strcpy( hiscall, "OH2");
    searchlog(hiscall);
    assert_int_equal (found_zone, 14);
}

void test_ZoneFromLog(void **state) {
    cqww = 1;
    strcpy(zone_export, "14");
    strcpy( hiscall, "SP9");
    searchlog(hiscall);
    assert_int_equal (found_zone, 15);
}

/* test position of output on lower border of search window */
void test_OnLowerSearchPanel_contest(void **state) {
    contest = 1;
    OnLowerSearchPanel(4, "test");
    check_mvprintw_output(0, 7, 4, "test");
}

void test_OnLowerSearchPanel_AllBand(void **state) {
    dxped = 1;
    OnLowerSearchPanel(4, "test");
    check_mvprintw_output(0, 10, 4, "test");
}
