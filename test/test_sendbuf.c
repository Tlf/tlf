#include "test.h"
#include <stdio.h>

#include "../src/sendbuf.h"
#include "../src/sendspcall.h"
#include "../src/tlf.h"
#include "../src/globalvars.h"
#include "../src/cqww_simulator.h"

// OBJECT ../src/sendbuf.o
// OBJECT ../src/sendspcall.o

char test_msg[1024];

/* export internal function for test */
void ExpandMacro();
char *PrepareSPcall();
void replace_all(char *buf, int size, const char *what,
		 const char *rep);


/* break dependencies */
int digikeyer = NO_KEYER;
int cwkeyer = NO_KEYER;

extern char call[20];
extern char message[25][80];
extern char buffer[];
extern char wkeyerbuffer[400];
extern int demode;
char *SPcall;
extern int sending_call, data_ready, shortqsonr;

void keyer_append(const char *string) { }
int play_file(char *file) { return 0; }

bool simulator;
void set_simulator_state(simstate_t s) { }
simstate_t get_simulator_state() { return IDLE; }


/* test helpers */
void check_replace_all(char *input, const char *what, char *rep,
		       char *exp) {
    char sandbox[30 + 1];
    memset(sandbox, 'S', 30);
    sandbox[30] = 0;
    char *buf = sandbox + 10;

    strcpy(buf, input);

    replace_all(buf, 10, what, rep);
    // check the result
    sprintf(test_msg, "for input |%s|\ngot |%s|\nexp |%s|\n", input, buf,
	    exp);
    assert_string_equal(buf, exp);

    // check if the rest of sandbox is intact
    int i;
    for (i = 0; i < 30; ++i) {
	if (10 <= i && i < 20) {
	    continue;
	}
	sprintf(test_msg, "wrong char at %d: 0x%02x", i, sandbox[i]);
	assert_int_equal(sandbox[i], 'S');
    }
}

void check_ExpandMacro(const char *input, const char *exp) {
    strcpy(buffer, input);
    ExpandMacro();
    assert_string_equal(buffer, exp);
}


/* setup/teardown */
int setup_default(void **state) {
    wkeyerbuffer[0] = '\0';
    data_ready = 0;
    simulator = false;
    sending_call = 0;
    trxmode = CWMODE;
    cwkeyer = 1;
    digikeyer = 1;
    strcpy(call, "dl1jbe\n"); 		// !!! do not forget trailing \n
    strcpy(hiscall, "lz1ab");
    strcpy(his_rst, "579");
    shortqsonr = LONGCW;
    strcpy(qsonrstr, "0309");
    strcpy(comment, "Alex");
    *message[SP_CALL_MSG] = '\0';

    return 0;
}

int teardown_default(void **state) {
    if (SPcall)
	g_free(SPcall);
    return 0;
}


/* test replace_all */
void test_replace_all(void **state) {
    // basic replace tests
    // - 'what' is empty
    (check_replace_all("1noop", "", "A", "1noop"));
    // - 'repl' is empty
    (check_replace_all("2noop", "X", "", "2noop"));
    (check_replace_all("2noop", "o", "", "2np"));
    (check_replace_all("onoop", "o", "", "np"));
    (check_replace_all("0000", "0000", "", ""));
    // - 'what' not found
    (check_replace_all("3noop", "X", "Y", "3noop"));
    // - simple replacement
    (check_replace_all("012345", "2", "Y", "01Y345"));
    (check_replace_all("012322", "2", "Y", "01Y3YY"));
    (check_replace_all("012345678", "8", "Y", "01234567Y"));
    // - shorter replacement
    (check_replace_all("001200300", "00", "Y", "Y12Y3Y"));
    (check_replace_all("000000", "00", "Y", "YYY"));
    (check_replace_all("0000", "0000", "YW", "YW"));
    // - longer replacement
    (check_replace_all("012345", "0", "YW", "YW12345"));
    (check_replace_all("012305", "0", "YW", "YW123YW5"));
    (check_replace_all("0123405", "0", "YW", "YW1234YW5"));
    (check_replace_all("01234505", "0", "YW", "YW12345YW"));
    (check_replace_all("0120", "0", "YWX", "YWX12YWX"));
    (check_replace_all("01230", "0", "YWX", "YWX123YWX"));
    (check_replace_all("012340", "0", "YWX", "YWX1234YW"));
    (check_replace_all("0123450", "0", "YWX", "YWX12345Y"));
    (check_replace_all("01234560", "0", "YWX", "YWX123456"));
    (check_replace_all("01234567", "0", "YWX", "YWX123456"));
    (check_replace_all("012305", "0", "YWX", "YWX123YWX"));
    (check_replace_all("012300", "0", "YWX", "YWX123YWX"));
    (check_replace_all("01200", "0", "YWX", "YWX12YWXY"));
    (check_replace_all("1200", "0", "YWX", "12YWXYWX"));
    (check_replace_all("1234567", "7", "YWX", "123456YWX"));
    (check_replace_all("1234567", "4", "YWX", "123YWX567"));
    (check_replace_all("12345678", "4", "YWX", "123YWX567"));
    (check_replace_all("1234567", "7", "YWXZ", "123456YWX"));
    (check_replace_all("1", "1", "ABCDEFGHIJKL", "ABCDEFGHI"));
    (check_replace_all("11", "1", "ABCDEFGHIJKL", "ABCDEFGHI"));


}


/* test ExpandMacro() */
void test_noexpand(void **state) {
    check_ExpandMacro("test de ab4def", "test de ab4def");
}

void test_expandCall(void **state) {
    check_ExpandMacro("test de %", "test de dl1jbe");
}

void test_expandHiscall(void **state) {
    check_ExpandMacro("@ test de %", "lz1ab test de dl1jbe");
}


void test_expandHisRST(void **state) {
    check_ExpandMacro("ur [", "ur 579");
}

void test_expandHisRSTshort(void **state) {
    shortqsonr = SHORTCW;
    check_ExpandMacro("ur [", "ur 57N");
}

void test_expandQsoNr(void **state) {
    check_ExpandMacro("nr #", "nr 309");
}

void test_expandQsoNrshort(void **state) {
    shortqsonr = SHORTCW;
    check_ExpandMacro("nr #", "nr 3TN");
}

void test_expandHisNr(void **state) {
    check_ExpandMacro("was !", "was Alex");
}


/* Tests sendSPcall() */
void test_prepareSPcallCWnoDeMode(void **state) {
    demode = 0;
    assert_string_equal(SPcall = PrepareSPcall(), "dl1jbe\n");
}

void test_prepareSPcallCWDeMode(void **state) {
    demode = 1;
    assert_string_equal(SPcall = PrepareSPcall(), "DE dl1jbe\n");
}

void test_prepareSPcallDIGInoDeMode(void **state) {
    trxmode = DIGIMODE;
    digikeyer = NET_KEYER;
    demode = 0;
    assert_string_equal(SPcall = PrepareSPcall(), "|dl1jbe ");
}

void test_prepareSPcallDIGIDeMode(void **state) {
    trxmode = DIGIMODE;
    digikeyer = NET_KEYER;
    demode = 1;
    assert_string_equal(SPcall = PrepareSPcall(), "|lz1ab DE dl1jbe ");
}

void test_prepareSPcallMFJnoDeMode(void **state) {
    trxmode = DIGIMODE;
    digikeyer = MFJ1278_KEYER;
    demode = 0;
    assert_string_equal(SPcall = PrepareSPcall(), "{ |dl1jbe }");
}

void test_prepareSPcallMFJDeMode(void **state) {
    trxmode = DIGIMODE;
    digikeyer = MFJ1278_KEYER;
    demode = 1;
    assert_string_equal(SPcall = PrepareSPcall(), "{ |lz1ab DE dl1jbe }");
}

