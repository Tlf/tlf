#include "test.h"
#include <stdio.h>

#include "../src/clusterinfo.h"
#include "../src/globalvars.h"
#include "../src/bandmap.h"
#include "../src/lancode.h"
#include "../src/dxcc.h"

// OBJECT ../src/clusterinfo.o
// OBJECT ../src/get_time.o
// OBJECT ../src/err_utils.o


int LINES=25;   /* test for 25 lines */

long timecorr;
int getctynr(char *checkcall) {
    return 0;
}

extern char spot_ptr[MAX_SPOTS][82];
extern int nr_of_spots;
extern int announcefilter;
extern int xplanet;
extern int trx_control;
extern freq_t freq;

char thisnode = 'A';
freq_t node_frequencies[MAXNODES];
char call[20];
int cluster;

#include <pthread.h>
pthread_mutex_t spot_ptr_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bm_mutex = PTHREAD_MUTEX_INITIALIZER;

bm_config_t bm_config = { .livetime = 900 };

GList *allspots = NULL; // not used yet

/* mockups */
static char nicebox_boxname[100];
void nicebox(int y, int x, int height, int width, char *boxname) {
    strcpy(nicebox_boxname, boxname);
}

static int bandmap_show_called;
void bandmap_show() {
    bandmap_show_called = 1;
}

static int printcall_called;
void printcall() {
    printcall_called = 1;
}

int modify_attr(int attr) {
    return attr;
}

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

/* setup/teardown */

static void put_dx_line(char *p, int index) {
    sprintf(p,
	    "DX de QQ3QQQ:    %2d010.0  AA%dAAA      COMMENT                        12%02dZ",
	    (index % 2) == 0 ? 7 : 14,
	    index % 10,
	    index % 60);
}

static void put_anno_line(char *p, int index, char *to) {
    sprintf(p,
	    "To %s de QQ3QQQ: message                           12%02dZ",
	    to,
	    index % 60);
}

static void put_wwv_line(char *p, int index) {
    sprintf(p,
	    "WWV de Q0QQ:  SFI=68,A=12,K=3,No Storms             12%02dZ",
	    index % 60);
}

static void put_short_line(char *p, int index) {
    sprintf(p,
	    "length<20 12%02dZ",
	    index % 60);
}

int setup_default(void **state) {
    strcpy(call, "N0CALL\n"); 		// !!! do not forget trailing \n

    xplanet = 0;

    clear_mvprintw_history();

    // generate 25 various cluster spots
    nr_of_spots = 25;

    for (int i = 0; i < MAX_SPOTS; ++i) {
	char *spot = spot_ptr[i];
	if (i >= nr_of_spots) {
	    spot[0] = 0;
	    continue;
	}

	if (i == 17) {
	    put_short_line(spot, i);
	} else if (i == 19) {
	    put_anno_line(spot, i, "N0CALL");
	} else if (i == 21) {
	    put_anno_line(spot, i, "ALL");
	} else if (i == 23) {
	    put_wwv_line(spot, i);
	} else {
	    put_dx_line(spot, i);
	}

    }

    trx_control = 1;
    freq = 7123800.0;   // Hz

    nicebox_boxname[0] = 0;
    printcall_called = 0;
    bandmap_show_called = 0;

    return 0;
}

static void check_mvprintw_output(int index, int y, int x, const char *text) {
    char buffer[7];
    sprintf(buffer, "%02d|%02d|", y, x);
    assert_memory_equal(mvprintw_history[index], buffer, 6);
    assert_string_equal(mvprintw_history[index] + 6, text);
}

/* test CLUSTER mode with FILTER_DX setting */
void test_cluster_show_dx(void **state) {

    cluster = CLUSTER;
    announcefilter = FILTER_DX;

    clusterinfo();

    // check that only DX spots are shown
    check_mvprintw_output(7, 15, 1, spot_ptr[13]);
    check_mvprintw_output(6, 16, 1, spot_ptr[14]);
    check_mvprintw_output(5, 17, 1, spot_ptr[15]);
    check_mvprintw_output(4, 18, 1, spot_ptr[16]);
    // #17 missing (short)
    check_mvprintw_output(3, 19, 1, spot_ptr[18]);
    // #19 missing (talk)
    check_mvprintw_output(2, 20, 1, spot_ptr[20]);
    // #21 missing (announcement)
    check_mvprintw_output(1, 21, 1, spot_ptr[22]);
    // #23 missing (WWV)
    check_mvprintw_output(0, 22, 1, spot_ptr[24]);

    assert_string_equal(nicebox_boxname, "Cluster");

    assert_true(printcall_called);
    assert_false(bandmap_show_called);
}

/* test CLUSTER mode with FILTER_TALK setting */
void test_cluster_show_talk(void **state) {

    cluster = CLUSTER;
    announcefilter = FILTER_TALK;

    clusterinfo();

    // check that DX spots + talk message are shown
    check_mvprintw_output(7, 15, 1, spot_ptr[14]);
    check_mvprintw_output(6, 16, 1, spot_ptr[15]);
    check_mvprintw_output(5, 17, 1, spot_ptr[16]);
    // #17 missing (short)
    check_mvprintw_output(4, 18, 1, spot_ptr[18]);
    check_mvprintw_output(3, 19, 1, spot_ptr[19]);
    check_mvprintw_output(2, 20, 1, spot_ptr[20]);
    // #21 missing (announcement)
    check_mvprintw_output(1, 21, 1, spot_ptr[22]);
    // #23 missing (WWV)
    check_mvprintw_output(0, 22, 1, spot_ptr[24]);

    assert_string_equal(nicebox_boxname, "Cluster");

    assert_true(printcall_called);
}

/* test CLUSTER mode with FILTER_ANN setting */
void test_cluster_show_ann(void **state) {

    cluster = CLUSTER;
    announcefilter = FILTER_ANN;

    clusterinfo();

    // check that DX spots + announcements are shown
    check_mvprintw_output(7, 15, 1, spot_ptr[15]);
    check_mvprintw_output(6, 16, 1, spot_ptr[16]);
    // #17 missing (short)
    check_mvprintw_output(5, 17, 1, spot_ptr[18]);
    check_mvprintw_output(4, 18, 1, spot_ptr[19]);
    check_mvprintw_output(3, 19, 1, spot_ptr[20]);
    check_mvprintw_output(2, 20, 1, spot_ptr[21]);
    check_mvprintw_output(1, 21, 1, spot_ptr[22]);
    // #23 missing (WWV)
    check_mvprintw_output(0, 22, 1, spot_ptr[24]);

    assert_string_equal(nicebox_boxname, "Cluster");

    assert_true(printcall_called);
}

/* test CLUSTER mode with FILTER_ALL setting */
void test_cluster_show_all(void **state) {

    cluster = CLUSTER;
    announcefilter = FILTER_ALL;

    clusterinfo();

    // check that all spots are shown except the too short one
    check_mvprintw_output(7, 15, 1, spot_ptr[16]);
    // #17 missing (short)
    check_mvprintw_output(6, 16, 1, spot_ptr[18]);
    check_mvprintw_output(5, 17, 1, spot_ptr[19]);
    check_mvprintw_output(4, 18, 1, spot_ptr[20]);
    check_mvprintw_output(3, 19, 1, spot_ptr[21]);
    check_mvprintw_output(2, 20, 1, spot_ptr[22]);
    check_mvprintw_output(1, 21, 1, spot_ptr[23]);
    check_mvprintw_output(0, 22, 1, spot_ptr[24]);

    assert_string_equal(nicebox_boxname, "Cluster");

    assert_true(printcall_called);
}

/* test CLUSTER mode with just 6 spots */
void test_cluster_show_all_6(void **state) {

    cluster = CLUSTER;
    announcefilter = FILTER_ALL;

    nr_of_spots = 6;

    clusterinfo();

    // check that all 6 spots are shown
    check_mvprintw_output(5, 15, 1, spot_ptr[0]);
    check_mvprintw_output(4, 16, 1, spot_ptr[1]);
    check_mvprintw_output(3, 17, 1, spot_ptr[2]);
    check_mvprintw_output(2, 18, 1, spot_ptr[3]);
    check_mvprintw_output(1, 19, 1, spot_ptr[4]);
    check_mvprintw_output(0, 20, 1, spot_ptr[5]);
    // lines 21 and 22 are empty

    assert_string_equal(nicebox_boxname, "Cluster");

    assert_true(printcall_called);
}

/* test FREQWINDOW mode */
void test_freqwindow(void **state) {

    cluster = FREQWINDOW;

    clusterinfo();

    // frequency shown rounded to kHz
    check_mvprintw_output(0, 15, 4, " Stn A :  7124");

    assert_string_equal(nicebox_boxname, "Frequencies");

    assert_true(printcall_called);
    assert_false(bandmap_show_called);
}
