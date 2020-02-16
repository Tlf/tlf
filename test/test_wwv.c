#include "test.h"

#include "../src/getwwv.h"

// OBJECT ../src/getwwv.o

int setup_default(void **state) {

    ssn_r = 10.0;
    lastwwv[0] = 0;

    return 0;
}

void test_wwv_check_not_wwv(void **state) {
    char *s = "DX de DL3DTH-#:   1816.5  ON7PQ        CW 30 dB 23 WPM CQ             2201Z";
    wwv_add(s);
    assert_int_equal(0, strlen(lastwwv));
    assert_in_range(ssn_r, 9.99, 10.01);    // unchanged
}

void test_wwv_check(void **state) {
    char *s = "WWV de VA6AAA <18>:   SFI=74, A=5, K=0, No Storms -> No Storms";
    wwv_add(s);
    assert_in_range(ssn_r, 4.43, 4.45);     // 74 --> 4.44
    g_strchomp(lastwwv);
    assert_string_equal(lastwwv, "Condx: 18 GMT                       SFI=74");
}

void test_wwv_check_wcy(void **state) {
    char *s = "WCY de DK0WCY-1 <07> : K=3 expK=4 A=10 R=0 SFI=71 SA=qui GMF=qui Au=no";
    wwv_add(s);
    assert_in_range(ssn_r, 1.10, 1.12);     // 71 --> 1.11
    g_strchomp(lastwwv);
    assert_string_equal(lastwwv, "Condx: 07 GMT              R=0      SFI=71");
}

void test_wwv_check_wcy_sa(void **state) {
    char *s = "WCY de DK0WCY-1 <17> : K=3 expK=4 A=10 R=11 SFI=71 SA=eru GMF=qui Au=no";
    wwv_add(s);
    assert_in_range(ssn_r, 1.10, 1.12);     // 71 --> 1.11
    g_strchomp(lastwwv);
    assert_string_equal(lastwwv,
			"Condx: 17 GMT              R=11     SFI=71     eruptive");
}

void test_wwv_check_wcy_sa_gmf(void **state) {
    char *s = "WCY de DK0WCY-1 <17> : K=3 expK=4 A=10 R=11 SFI=71 SA=eru GMF=act Au=no";
    wwv_add(s);
    assert_in_range(ssn_r, 1.10, 1.12);     // 71 --> 1.11
    g_strchomp(lastwwv);
    assert_string_equal(lastwwv,
			"Condx: 17 GMT              R=11     SFI=71     eruptive  act");
}

void test_wwv_check_wcy_sa_gmf_aur(void **state) {
    char *s = "WCY de DK0WCY-1 <17> : K=3 expK=4 A=10 R=0 SFI=71 SA=eru GMF=act Au=au";
    wwv_add(s);
    assert_in_range(ssn_r, 1.10, 1.12);     // 71 --> 1.11
    g_strchomp(lastwwv);
    assert_int_equal(70, strlen(lastwwv));
    assert_string_equal(lastwwv,
			"Condx: 17 GMT              R=0      SFI=71     eruptive  act   AURORA!");
// original:            "Condx: 17 GMT              R=0 S    SFI=71      eruptive   act     AURORA!"
}

void test_wwv_check_wcy_sa_aur(void **state) {
    char *s = "WCY de DK0WCY-1 <17> : K=3 expK=4 A=10 R=11 SFI=71 SA=eru GMF=qui Au=au";
    wwv_add(s);
    assert_in_range(ssn_r, 1.10, 1.12);     // 71 --> 1.11
    g_strchomp(lastwwv);
    assert_int_equal(70, strlen(lastwwv));
    assert_string_equal(lastwwv,
			"Condx: 17 GMT              R=11     SFI=71     eruptive        AURORA!");
}

void test_wwv_check_wcy_no_gmt(void **state) {
    char *s = "WCY de DK0WCY-1 #### : K=3 expK=4 A=10 R=0 SFI=71 SA=qui GMF=qui Au=no";
    wwv_add(s);
    assert_in_range(ssn_r, 1.10, 1.12);     // 71 --> 1.11
    g_strchomp(lastwwv);
    assert_string_equal(lastwwv, "Condx:                     R=0      SFI=71");
}


