#include "test.h"

#include "../src/addcall.h"
#include "../src/bands.h"
#include "../src/dxcc.h"
#include "../src/getctydata.h"
#include "../src/globalvars.h"
#include "../src/log_utils.h"
#include "../src/searchcallarray.h"
#include "../src/score.h"
#include "../src/setcontest.h"

// OBJECT ../src/addcall.o
// OBJECT ../src/addpfx.o
// OBJECT ../src/bands.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/focm.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/get_time.o
// OBJECT ../src/log_utils.o
// OBJECT ../src/plugin.o
// OBJECT ../src/qrb.o
// OBJECT ../src/score.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/setcontest.o
// OBJECT ../src/utils.o
// OBJECT ../src/zone_nr.o

void addmult(struct qso_t qso) {}
void addmult_lan() {}
void checkexchange(struct qso_t *qso, bool interactive) {}
int check_mult(struct qso_t *qso) { return -1; }
void clear_display() {}
int pacc_pa(void) {
    return 0;
}

extern int excl_add_veto2;


/* setups */
int setup_default(void **state) {
    static char filename[] =  TOP_SRCDIR "/share/cty.dat";
    assert_int_equal(load_ctydata(filename), 0);

    init_worked();
    bandinx = BANDINDEX_10;

    setcontest("CQWW");

    pfxmult = false;
    dupe = 0;

    /* it may be a bug that addcall does not initialize addcallarea */
    addcallarea = 0;

    strcpy(countrylist[0], "DL");
    strcpy(countrylist[1], "CE");
    strcpy(countrylist[2], "");

    countrylist_only = false;

    strcpy(continent_multiplier_list[0], "EU");
    strcpy(continent_multiplier_list[1], "NA");
    strcpy(continent_multiplier_list[2], "");

    continentlist_only = false;

    exclude_multilist_type = EXCLUDE_NONE;

    pfxnummultinr = 0;
    memset(pfxnummulti, 0, sizeof(pfxnummulti));

    memset(countries, 0, sizeof(countries));
    memset(countryscore, 0, sizeof(countryscore));

    memset(zones, 0, sizeof(zones));
    memset(zonescore, 0, sizeof(zonescore));

    current_qso.call = g_malloc0(CALL_SIZE);
    current_qso.comment = g_malloc0(COMMENT_SIZE);

    return 0;
}

int setup_addcall_pfxnum_inList(void **state) {
    int countrynr = getctynr("LZ1AB");

    setup_default(state);

    pfxnummultinr = 2;
    pfxnummulti[0].countrynr = countrynr + 3;
    pfxnummulti[1].countrynr = countrynr;

    return 0;
}

int setup_addcall_pfxnum_notinList(void **state) {
    return setup_addcall_pfxnum_inList(state);
}

/* collect_qso_data */
void test_collect(void **state) {
    struct qso_t *qso;
    strcpy(current_qso.call, "LZ1AB");
    strcpy(current_qso.comment, "Hi");
    time_t now = time(NULL);
    bandinx = BANDINDEX_80;
    trxmode = CWMODE;

    qso = collect_qso_data();
    assert_non_null(qso);

    assert_string_equal(qso->call, current_qso.call);
    assert_string_equal(qso->comment, current_qso.comment);
    assert_int_equal(qso->bandindex, bandinx);
    assert_int_equal(qso->mode, CWMODE);
    assert_in_range(qso->timestamp, now, now + 1);

    free_qso(qso);
}

/* test check_veto() */
void test_veto_exclude_none(void **state) {
    assert_int_equal(check_veto(getctynr("HB9ABC")), false);
}

void test_veto_exclude_country(void **state) {
    exclude_multilist_type = EXCLUDE_COUNTRY;
    assert_int_equal(check_veto(getctynr("HB9ABC")), false);
    assert_int_equal(check_veto(getctynr("DL1AAA")), true);
}

void test_veto_exclude_continent_contlist_only(void **state) {
    continentlist_only = true;
    exclude_multilist_type = EXCLUDE_CONTINENT;
    assert_int_equal(check_veto(getctynr("DL1AAA")), false);
    assert_int_equal(check_veto(getctynr("3B8AA")), false);
}

void test_veto_exclude_continent(void **state) {
    exclude_multilist_type = EXCLUDE_CONTINENT;
    assert_int_equal(check_veto(getctynr("DL1AAA")), true);
    assert_int_equal(check_veto(getctynr("3B8AA")), false);
}


/* addcall */
void test_add_to_worked(void **state) {
    strcpy(current_qso.call, "LZ1AB");
    bandinx = BANDINDEX_10;
    strcpy(current_qso.comment, "Hi");
    time_t now = time(NULL);

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 1);
    assert_string_equal(worked[0].exchange, "Hi");
    assert_int_equal(worked[0].band & inxes[BANDINDEX_10], inxes[BANDINDEX_10]);
    assert_in_range(worked[0].qsotime[trxmode][BANDINDEX_10], now, now + 1);
    assert_int_equal(worked[0].ctyinfo->dxcc_ctynr, getctynr("LZ1AB"));
}

void test_add_to_worked_continentlistonly(void **state) {
    continentlist_only = true;

    strcpy(current_qso.call, "LZ1AB");
    bandinx = BANDINDEX_10;
    strcpy(current_qso.comment, "Hi");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    strcpy(current_qso.call, "PY2BBB");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 2);
    assert_string_equal(worked[0].call, "LZ1AB");
    assert_string_equal(worked[1].call, "PY2BBB");
}

void test_addcall_nopfxnum(void **state) {
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(addcallarea, 0);
}


void test_addcall_pfxnum_inList(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");
    time_t now = time(NULL);

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(addcallarea, 1);
    assert_int_equal(pfxnummulti[1].qsos[1], BAND10);
    assert_int_equal(countryscore[BANDINDEX_10], 1);
    assert_int_equal(zonescore[BANDINDEX_10], 1);

    assert_int_equal(nr_worked, 1);
    assert_in_range(worked[0].qsotime[trxmode][BANDINDEX_10], now, now + 1);
}


void test_addcall_pfxnum_notinList(void **state) {
    strcpy(current_qso.call, "HA2BNL");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(addcallarea, 0);
}

void test_addcall_continentlistonly(void **state) {
    continentlist_only = true;
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 1);
    assert_int_equal(countryscore[qso->bandindex], 1);

    strcpy(current_qso.call, "PY2BBB");  // SA is not in the list

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 2);
    assert_int_equal(countryscore[qso->bandindex], 1);  // not counted
}

void test_addcall_exclude_continent(void **state) {
    exclude_multilist_type = EXCLUDE_CONTINENT;
    strcpy(current_qso.call, "LZ1AB");       // EU is excluded

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 1);
    assert_int_equal(countryscore[qso->bandindex], 0);  // not counted

    strcpy(current_qso.call, "PY2BBB");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 2);
    assert_int_equal(countryscore[qso->bandindex], 1);
}

void test_addcall_exclude_country(void **state) {
    exclude_multilist_type = EXCLUDE_COUNTRY;
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 1);
    assert_int_equal(countryscore[qso->bandindex], 1);

    strcpy(current_qso.call, "DL1AAA");  // DL is excluded

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(nr_worked, 2);
    assert_int_equal(countryscore[qso->bandindex], 1);  // not counted
}


/* addcall2 */
char logline[] =
    "160CW  08-Feb-11 17:06 0025  LZ1AB          599  599  20            LZ  20   1  ";

char logline_HA[] =
    "160CW  08-Aug-11 17:06 0025  HA1AB          599  599  19            LZ  20   1  ";

char logline_PY[] =
    "160CW  08-Feb-11 17:06 0025  PY1AB          599  599  19            PY  20   1  ";

char logline_DL[] =
    "160CW  08-Feb-11 17:06 0025  DL1AB          599  599  19            PY  20   1  ";

void test_addcall2_nopfxnum(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(addcallarea, 0);

    assert_int_equal(nr_worked, 1);
    // 2011-02-08 17:06 UTC
    assert_int_equal(worked[0].qsotime[trxmode][BANDINDEX_160], 1297184760);
}

int setup_addcall2_pfxnum_inList(void **state) {
    return setup_addcall_pfxnum_inList(state);
}


int setup_addcall2_pfxnum_notinList(void **state) {
    return setup_addcall_pfxnum_inList(state);
}



void test_addcall2_pfxnum_inList(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(addcallarea, 1);
    assert_int_equal(pfxnummulti[1].qsos[1], BAND160);
    assert_int_equal(countryscore[BANDINDEX_160], 1);
    assert_int_equal(zonescore[BANDINDEX_160], 1);
}

void test_addcall2_pfxnum_notinList(void **state) {
    strcpy(lan_logline, logline_HA);
    addcall2();
    assert_int_equal(addcallarea, 0);

    assert_int_equal(nr_worked, 1);
    // 2011-08-08 17:06 UTC
    assert_int_equal(worked[0].qsotime[trxmode][BANDINDEX_160], 1312823160);
}


/* check country handling in addcall() */
void test_add_unknown_country(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "12345");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(countryscore[bandinx], 0);
    assert_int_equal(countries[getctynr("LZ1AB")], 0);
}

void test_add_country(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(countryscore[bandinx], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10);
}

void test_add_country_2_band(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    bandinx = BANDINDEX_15;
    strcpy(current_qso.call, "LZ1AB");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(countryscore[BANDINDEX_10], 1);
    assert_int_equal(countryscore[BANDINDEX_15], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10 | BAND15);
}

void test_add_country_2_stations(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    strcpy(current_qso.call, "LZ3CD");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(countryscore[bandinx], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10);
}

void test_add_2_countries(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    strcpy(current_qso.call, "DL1YZ");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(countryscore[BANDINDEX_10], 2);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10);
    assert_int_equal(countries[getctynr("DL0ABC")], BAND10);
}

/* check zone handling in addcall() */
void test_add_unknown_zone(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "12345");
    strcpy(current_qso.comment, "0");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(zonescore[bandinx], 0);
    assert_int_equal(zones[15], 0);
}

void test_add_zone(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");
    strcpy(current_qso.comment, "15");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(zonescore[bandinx], 1);
    assert_int_equal(zones[15], BAND10);
}

void test_add_zone_2_band(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");
    strcpy(current_qso.comment, "15");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    bandinx = BANDINDEX_15;
    strcpy(current_qso.call, "LZ1AB");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(zonescore[BANDINDEX_10], 1);
    assert_int_equal(zonescore[BANDINDEX_15], 1);
    assert_int_equal(zones[15], BAND10 | BAND15);
}

void test_add_zone_2_stations(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");
    strcpy(current_qso.comment, "15");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    strcpy(current_qso.call, "LZ3CD");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(zonescore[bandinx], 1);
    assert_int_equal(zones[15], BAND10);
}

void test_add_2_zones(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(current_qso.call, "LZ1AB");
    strcpy(current_qso.comment, "15");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    strcpy(current_qso.call, "DL1YZ");
    strcpy(current_qso.comment, "14");

    qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(zonescore[BANDINDEX_10], 2);
    assert_int_equal(zones[14], BAND10);
    assert_int_equal(zones[15], BAND10);
}


/* check country handling in addcall2() */
void test_add2_unknown_country(void **state) {
    strcpy(lan_logline, logline);
    lan_logline[29] = '1';
    lan_logline[30] = '2';
    addcall2();
    assert_int_equal(countryscore[bandinx], 0);
    assert_int_equal(countries[getctynr("LZ1AB")], 0);
}

void test_add2_country(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(countryscore[BANDINDEX_160], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND160);
}

void test_add2_country_2_band(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    memcpy(lan_logline, " 15", 3);		/* patch to 15m */
    addcall2();
    assert_int_equal(countryscore[BANDINDEX_160], 1);
    assert_int_equal(countryscore[BANDINDEX_15], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND160 | BAND15);
}

void test_add2_country_2_stations(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    lan_logline[31] = '3';		/* modify to LZ3AB */
    addcall2();
    assert_int_equal(countryscore[BANDINDEX_160], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND160);
}

void test_add2_2_countries(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    strcpy(lan_logline, logline_HA);
    addcall2();
    assert_int_equal(countryscore[BANDINDEX_160], 2);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND160);
    assert_int_equal(countries[getctynr("HA0ABC")], BAND160);
}


/* check zone handling in addcall2() */
void test_add2_unknown_zone(void **state) {
    strcpy(lan_logline, logline);
    lan_logline[29] = '1';
    lan_logline[30] = '2';
    lan_logline[54] = '0';		/* Zone 0 */
    addcall2();
    assert_int_equal(zonescore[BANDINDEX_160], 0);
    assert_int_equal(zones[15], 0);
}

void test_add2_zone(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(zonescore[BANDINDEX_160], 1);
    assert_int_equal(zones[20], BAND160);
}

void test_add2_zone_2_band(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    memcpy(lan_logline, " 15", 3);
    addcall2();
    assert_int_equal(zonescore[BANDINDEX_160], 1);
    assert_int_equal(zonescore[BANDINDEX_15], 1);
    assert_int_equal(zones[20], BAND160 | BAND15);
}

void test_add2_zone_2_stations(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    lan_logline[31] = '3';		/* modify to LZ3AB */
    addcall2();
    assert_int_equal(zonescore[BANDINDEX_160], 1);
    assert_int_equal(zones[20], BAND160);
}

void test_add2_2_zones(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    strcpy(lan_logline, logline_HA);
    addcall2();
    assert_int_equal(zonescore[BANDINDEX_160], 2);
    assert_int_equal(zones[19], BAND160);
    assert_int_equal(zones[20], BAND160);
}

/* special test for WARC bands */
void test_add_warc(void **state) {
    bandinx = BANDINDEX_12;
    strcpy(current_qso.call, "LZ1AB");
    strcpy(current_qso.comment, "15");

    struct qso_t *qso = collect_qso_data();
    addcall(qso);

    assert_int_equal(countries[getctynr("LZ0AA")], BAND12);
    assert_int_equal(new_cty, getctynr("LZ1AB"));
    assert_int_equal(zones[15], BAND12);
    assert_int_equal(new_zone, 15);
}

void test_add2_warc(void **state) {
    strcpy(lan_logline, logline);
    memcpy(lan_logline, " 30", 3);
    addcall2();
    assert_int_equal(countries[getctynr("LZ0AA")], BAND30);
    assert_int_equal(zones[20], BAND30);
}


void test_addcall2_continentlistonly(void **state) {
    continentlist_only = true;
    strcpy(lan_logline, logline_PY);
    addcall2();
    assert_int_equal(excl_add_veto2, true);
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(excl_add_veto2, false);
}

void test_addcall2_exclude_continent(void **state) {
    exclude_multilist_type = EXCLUDE_CONTINENT;
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(excl_add_veto2, true);
    strcpy(lan_logline, logline_PY);
    addcall2();
    assert_int_equal(excl_add_veto2, false);
}

void test_addcall2_exclude_country(void **state) {
    exclude_multilist_type = EXCLUDE_COUNTRY;
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(excl_add_veto2, false);
    strcpy(lan_logline, logline_DL);
    addcall2();
    assert_int_equal(excl_add_veto2, true);
}

