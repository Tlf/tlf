#include "test.h"

#include "../src/tlf.h"
#include "../src/addmult.h"
#include "../src/globalvars.h"
#include "../src/bands.h"
#include <stdio.h>
#include <unistd.h>

// OBJECT ../src/addmult.o
// OBJECT ../src/bands.o

extern mults_t multis[MAX_MULTS];
extern int nr_multis;

extern char multsfile[];	/* name of file with a list of allowed
				   multipliers */

char *testfile = "mults";

/* export internal definition */
GSList *get_aliases(int n);

/* helpers for writing a temporary multsfile for testing */
void write_testfile(char *name, char *content) {
    FILE *fp;
    fp = fopen(name, "w");
    if (fp != NULL) {
	fputs(content, fp);
	fclose(fp);
    }
}

void setup_multis(char *multstring) {
    write_testfile(testfile, multstring);
    strcpy(multsfile, testfile);
    init_and_load_multipliers();
}


int setup_default(void **state) {
    bandinx = BANDINDEX_80;

    arrlss = 0;
    shownewmult = -1;
    wysiwyg_once = 0;
    wysiwyg_multi = 0;
    arrlss = 0;
    serial_section_mult = 0;
    sectn_mult = 0;
    serial_grid4_mult = 0;
    dx_arrlsections = 0;
    ve_cty = 77;	/* random numbers just for test */
    w_cty = 78;

    strcpy(comment, "");
    strcpy(section, "");

    strcpy(multsfile, "");

    init_mults();

    unlink(testfile);

    return 0;
}

/* tewst initialization of 'multis' */
void test_init_mults(void **state) {
    nr_multis = 2;
    strcpy(multis[0].name, "abc");
    strcpy(multis[1].name, "abd");
    multis[0].band = inxes[BANDINDEX_160];
    multis[1].band = inxes[BANDINDEX_80];
    init_mults();
    assert_int_equal(nr_multis, 0);

    assert_string_equal(multis[0].name, "");
    assert_int_equal(multis[0].band, 0);
}


/* tests for remember_multi */
void test_remember_mult_empty(void **state) {
    assert_int_equal(remember_multi("", BANDINDEX_80, 0), -1);
}

void test_remember_mult_one(void **state) {
    assert_int_equal(remember_multi("abc", BANDINDEX_80, 0), 0);
    assert_int_equal(nr_multis, 1);
    assert_string_equal(multis[0].name, "abc");
    assert_int_equal(multis[0].band, inxes[BANDINDEX_80]);
}

void test_remember_mult_two(void **state) {
    assert_int_equal(remember_multi("abc", BANDINDEX_80, 0), 0);
    assert_int_equal(remember_multi("def", BANDINDEX_80, 0), 1);
    assert_int_equal(nr_multis, 2);
    assert_string_equal(multis[0].name, "abc");
    assert_string_equal(multis[1].name, "def");
    assert_int_equal(multis[0].band, inxes[BANDINDEX_80]);
    assert_int_equal(multis[1].band, inxes[BANDINDEX_80]);
}

void test_remember_mult_same_2x(void **state) {
    assert_int_equal(remember_multi("abc", BANDINDEX_80, 0), 0);
    assert_int_equal(remember_multi("abc", BANDINDEX_160, 0), -1);
    assert_int_equal(nr_multis, 1);
    assert_string_equal(multis[0].name, "abc");
    assert_int_equal(multis[0].band, inxes[BANDINDEX_80] | inxes[BANDINDEX_160]);
}

void test_remember_mult_same_2x_newband(void **state) {
    assert_int_equal(remember_multi("abc", BANDINDEX_80, 0), 0);
    assert_int_equal(remember_multi("abc", BANDINDEX_160, 1), 0);
    assert_int_equal(nr_multis, 1);
    assert_string_equal(multis[0].name, "abc");
    assert_int_equal(multis[0].band, inxes[BANDINDEX_80] | inxes[BANDINDEX_160]);
}


/* helpers for check of loading of possible mults */
void test_write_mult(void **state) {
    FILE *fp;
    char buffer[100];

    buffer[0] = '\0';
    write_testfile(testfile, "Hallo");
    fp = fopen(testfile, "r");
    assert_non_null(fp);
    char *p = fgets(buffer, sizeof(buffer), fp);
    assert_ptr_equal(p, buffer);
    assert_string_equal(buffer, "Hallo");
}


/* helper for checking content of mults_possible array */
void check_multi(int pos, char *str) {
    assert_string_equal(get_mult(pos), str);
}

/* tests for load_multipliers */
void test_load_multi_no_file(void **state) {
    assert_int_equal(init_and_load_multipliers(), 0);
    assert_int_equal(get_mult_count(), 0);
}

/** \todo better would be to return -1 if multsfile could not be found */
void test_load_multi_file_not_found(void **state) {
    strcpy(multsfile, "nonsense");
    assert_int_equal(init_and_load_multipliers(), 0);
}

void test_load_multi(void **state) {
    setup_multis("AB\n#LZ is not active\nKL\n 	\nZH\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
}

void test_load_multi_dos(void **state) {
    setup_multis("AB\r\n#LZ is not active\r\nKL\r\n 	\r\nZH\r\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
}

// leading space both on comment and data lines
void test_load_multi_leading_space(void **state) {
    setup_multis(" AB\n   #LZ is not active\nKL\n 	\nZH\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
}

void test_load_multi_sorted(void **state) {
    setup_multis("AB\n#LZ is not active\nZH\n 	\nKL\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
}

void test_load_multi_redefined(void **state) {
    setup_multis("AB\nKL\nZH\nKL\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
}


void test_load_multi_with_emptyalias(void **state) {
    setup_multis("AB\nZH :\nZH\nKL\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
    assert_int_equal(g_slist_length(get_aliases(2)), 0);
}

void test_load_multi_with_alias(void **state) {
    setup_multis("AB\nZH: NH, ZD,AA\nZH\nKL\n");
    assert_int_equal(get_mult_count(), 3);
    check_multi(0, "AB");
    check_multi(2, "ZH");
    assert_int_equal(g_slist_length(get_aliases(2)), 3);
}

/* test matching of mults and aliases */
void test_match_length_no_match(void **state) {
    setup_multis("ZH:NHA,ZDL,AA,AAC\n");
    assert_int_equal(get_matching_length("ABC", 0), 0);
}

void test_match_length_match_mult(void **state) {
    setup_multis("ZH:NHA,ZDL,AA,AAC\n");
    assert_int_equal(get_matching_length("12aZHXc", 0), 2);
}

void test_match_length_match_alias(void **state) {
    setup_multis("ZH:NHA,ZDL,AA,AAC\n");
    assert_int_equal(get_matching_length("12aAAX2", 0), 2);
}

void test_match_length_match_alias2(void **state) {
    setup_multis("ZH:NHA,ZDL,AA,AAC\n");
    assert_int_equal(get_matching_length("12aAAC2", 0), 3);
}


/* addmult tests */
void test_wysiwyg_once(void **state) {
    wysiwyg_once = 1;
    strcpy(comment, "WAC   ");
    assert_int_equal(addmult(), 0);
    assert_string_equal(multis[0].name, "WAC");
    assert_int_equal(shownewmult, 0);
}

void test_wysiwyg_multi(void **state) {
    wysiwyg_multi = 1;
    strcpy(comment, "WAC   ");
    assert_int_equal(addmult(), 0);
    assert_string_equal(multis[0].name, "WAC");
    assert_int_equal(shownewmult, 0);
}

void test_wysiwyg_multi_empty(void **state) {
    wysiwyg_multi = 1;
    strcpy(comment, "   ");
    addmult();
    assert_int_equal(shownewmult, -1);
}

void test_serial_grid4(void **state) {
    serial_grid4_mult = 1;
    strcpy(section, "JO60LX");
    assert_int_equal(addmult(), 0);
    assert_string_equal(multis[0].name, "JO60");
    assert_int_equal(shownewmult, 0);
}


void test_serial_grid4_empty(void **state) {
    serial_grid4_mult = 1;
    addmult();
    assert_int_equal(shownewmult, -1);
}

void test_arrlss(void **state) {
    arrlss = 1;
    setup_multis("SC\nSCV\n");
    strcpy(ssexchange, "SCV");
    addmult();
    strcpy(ssexchange, "97A23SCV");
    addmult();
    strcpy(ssexchange, "KL");
    addmult();
    strcpy(ssexchange, "SC");
    addmult();
    assert_int_equal(nr_multis, 2);
    assert_string_equal(multis[0].name, "SCV");
    assert_string_equal(multis[1].name, "SC");
}

void test_serial_section_mult(void **state) {
    serial_section_mult = 1;
    setup_multis("NE\nONE\n");
    strcpy(ssexchange, "ONE");
    addmult();
    strcpy(ssexchange, "023");
    addmult();
    strcpy(ssexchange, "NE");
    addmult();
    strcpy(ssexchange, "SC");
    addmult();
    assert_int_equal(nr_multis, 2);
}

void test_dx_arrlsections(void **state) {
    dx_arrlsections = 1;
    countrynr = w_cty;
    setup_multis("NE\nONE\n");
    strcpy(ssexchange, "ONE");
    addmult();
    strcpy(ssexchange, "97A23SCV");
    addmult();
    strcpy(ssexchange, "NE");
    addmult();
    strcpy(ssexchange, "SC");
    addmult();
    assert_int_equal(nr_multis, 2);
}


/* addmult2 tests */
char logline[] =
    " 20CW  08-Feb-11 17:06 0025  W3ND           599  599   95 A 12 SCV            2 ";

char logline_2[] =
    " 20CW  08-Feb-11 17:06 0025  W3ND           599  599  WAC                     2 ";

void test_arrlss_2(void **state) {
    arrlss = 1;
    setup_multis("SC\nSCV\n");
    strcpy(lan_logline, logline);
    addmult2();
    memcpy(lan_logline + 54, " 97 A 23 SCV", 12);
    addmult2();
    memcpy(lan_logline + 63, "KL ", 3);
    addmult2();
    memcpy(lan_logline + 63, "SC ", 3);
    addmult2();
    assert_int_equal(nr_multis, 2);
    assert_string_equal(multis[0].name, "SCV");
    assert_string_equal(multis[1].name, "SC");
}

void test_wysiwyg_once_2(void **state) {
    wysiwyg_once = 1;
    strcpy(lan_logline, logline_2);
    assert_int_equal(addmult2(), 0);
    assert_string_equal(multis[0].name, "WAC");
    assert_int_equal(shownewmult, 0);
}


void test_wysiwyg_multi_2(void **state) {
    wysiwyg_multi = 1;
    strcpy(lan_logline, logline_2);
    assert_int_equal(addmult2(), 0);
    assert_string_equal(multis[0].name, "WAC");
    assert_int_equal(shownewmult, 0);
}


