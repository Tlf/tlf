#include "test.h"

#include "../src/addcall.h"
#include "../src/globalvars.h"
#include "../src/dxcc.h"
#include "../src/getctydata.h"

// OBJECT ../src/addcall.o
// OBJECT ../src/addmult.o
// OBJECT ../src/searchcallarray.o
// OBJECT ../src/getctydata.o
// OBJECT ../src/getpx.o
// OBJECT ../src/dxcc.o
// OBJECT ../src/get_time.o
// OBJECT ../src/zone_nr.o


/* these are missing from globalvars */
extern int dupe;
extern t_pfxnummulti pfxnummulti[MAXPFXNUMMULT];
extern int pfxnummultinr;
extern char continent_multiplier_list[7][3];
extern char countrylist[][6];

int add_pfx(char *call) {
    return 0;
}

int pacc_pa(void) {
    return 0;
}

long timecorr = 0;


/* setups */
int setup_default(void **state) {
    char filename[100];

    bandinx = BANDINDEX_10;
    cqww = 1;   /* trigger zone evaluation */
    wpx = 0;
    pfxmult = 0;
    dupe = 0;
    arrldx_usa = 0;

    /* it may be a bug that addcall does not initialize addcallarea */
    addcallarea = 0;

    strcpy(countrylist[0], "");

    strcpy(continent_multiplier_list[0], "");

    pfxnummultinr = 0;
    memset(pfxnummulti, 0, sizeof(pfxnummulti));

    memset(countries, 0, sizeof(countries));
    memset(countryscore, 0, sizeof(countryscore));

    memset(zones, 0, sizeof(zones));
    memset(zonescore, 0, sizeof(zonescore));

    strcpy(filename, TOP_SRCDIR);
    strcat(filename, "/share/cty.dat");
    assert_int_equal(load_ctydata(filename), 0);

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

void test_addcall_nopfxnum(void **state) {
    strcpy(hiscall, "LZ1AB");
    addcall();
    assert_int_equal(addcallarea, 0);
}


void test_addcall_pfxnum_inList(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");

    addcall();
    assert_int_equal(addcallarea, 1);
    assert_int_equal(pfxnummulti[1].qsos[1], BAND10);
    assert_int_equal(countryscore[BANDINDEX_10], 1);
    assert_int_equal(zonescore[BANDINDEX_10], 1);
}


void test_addcall_pfxnum_notinList(void **state) {
    strcpy(hiscall, "HA2BNL");
    addcall();
    assert_int_equal(addcallarea, 0);
}

char logline[] =
    "160CW  08-Feb-11 17:06 0025  LZ1AB          599  599  20            LZ  20   1  ";

char logline_2[] =
    "160CW  08-Feb-11 17:06 0025  HA1AB          599  599  19            LZ  20   1  ";

void test_addcall2_nopfxnum(void **state) {
    strcpy(lan_logline, logline);
    addcall2();
    assert_int_equal(addcallarea, 0);
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
    strcpy(lan_logline, logline_2);
    addcall2();
    assert_int_equal(addcallarea, 0);
}


/* check country handling in addcall() */
void test_add_unknown_country(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "12345");
    addcall();
    assert_int_equal(countryscore[bandinx], 0);
    assert_int_equal(countries[getctynr("LZ1AB")], 0);
}

void test_add_country(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    addcall();
    assert_int_equal(countryscore[bandinx], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10);
}

void test_add_country_2_band(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    addcall();
    bandinx = BANDINDEX_15;
    strcpy(hiscall, "LZ1AB");
    addcall();
    assert_int_equal(countryscore[BANDINDEX_10], 1);
    assert_int_equal(countryscore[BANDINDEX_15], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10 | BAND15);
}

void test_add_country_2_stations(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    addcall();
    strcpy(hiscall, "LZ3CD");
    addcall();
    assert_int_equal(countryscore[bandinx], 1);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10);
}

void test_add_2_countries(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    addcall();
    strcpy(hiscall, "DL1YZ");
    addcall();
    assert_int_equal(countryscore[BANDINDEX_10], 2);
    assert_int_equal(countries[getctynr("LZ0AA")], BAND10);
    assert_int_equal(countries[getctynr("DL0ABC")], BAND10);
}

/* check zone handling in addcall() */
void test_add_unknown_zone(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "12345");
    strcpy(comment, "0");
    addcall();
    assert_int_equal(zonescore[bandinx], 0);
    assert_int_equal(zones[15], 0);
}

void test_add_zone(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    strcpy(comment, "15");
    addcall();
    assert_int_equal(zonescore[bandinx], 1);
    assert_int_equal(zones[15], BAND10);
}

void test_add_zone_2_band(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    strcpy(comment, "15");
    addcall();
    bandinx = BANDINDEX_15;
    strcpy(hiscall, "LZ1AB");
    addcall();
    assert_int_equal(zonescore[BANDINDEX_10], 1);
    assert_int_equal(zonescore[BANDINDEX_15], 1);
    assert_int_equal(zones[15], BAND10 | BAND15);
}

void test_add_zone_2_stations(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    strcpy(comment, "15");
    addcall();
    strcpy(hiscall, "LZ3CD");
    addcall();
    assert_int_equal(zonescore[bandinx], 1);
    assert_int_equal(zones[15], BAND10);
}

void test_add_2_zones(void **state) {
    bandinx = BANDINDEX_10;
    strcpy(hiscall, "LZ1AB");
    strcpy(comment, "15");
    addcall();
    strcpy(hiscall, "DL1YZ");
    strcpy(comment, "14");
    addcall();
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
    strcpy(lan_logline, logline_2);
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
    strcpy(lan_logline, logline_2);
    addcall2();
    assert_int_equal(zonescore[BANDINDEX_160], 2);
    assert_int_equal(zones[19], BAND160);
    assert_int_equal(zones[20], BAND160);
}

/* special test for WARC bands */
void test_add_warc(void **state) {
    bandinx = BANDINDEX_12;
    strcpy(hiscall, "LZ1AB");
    strcpy(comment, "15");
    addcall();
    assert_int_equal(countries[getctynr("LZ0AA")], BAND12);
    assert_int_equal(addcty, getctynr("LZ1AB"));
    assert_int_equal(zones[15], BAND12);
    assert_int_equal(addzone, 15);
}

void test_add2_warc(void **state) {
    strcpy(lan_logline, logline);
    memcpy(lan_logline, " 30", 3);
    addcall2();
    assert_int_equal(countries[getctynr("LZ0AA")], BAND30);
    assert_int_equal(zones[20], BAND30);
}

