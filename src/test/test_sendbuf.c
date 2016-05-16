#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "../tlf.h"
#include "../sendbuf.h"
extern char buffer[];
extern void ExpandMacro(void);
extern void replace_all(char *buf, int size, const char *what,
                        const char *rep);


int tests_run = 0;
int test_verbose = 0;
char test_msg[1024];

extern char call[20];
extern int shortqsonr;
extern int noleadingzeros;
extern char comment[];
extern char his_rst[];
extern char hiscall[];
extern char hiscall_sent[];
extern char qsonrstr[5];


static char *test_ExpandMacro(const char *input, const char *exp)
{
    strcpy(buffer, input);

    if (test_verbose) {
        printf("|%s| ==> ", buffer);
    }
    ExpandMacro();
    if (test_verbose) {
        printf("|%s|\n", buffer);
    }

    sprintf(test_msg, "got >%s<\nexp >%s<\n", buffer, exp);
    mu_assert(test_msg, 0 == strcmp(buffer, exp));

    return NULL;
}

static char *test_replace_all(char *input, const char *what, char *rep,
                              char *exp)
{
    char sandbox[30 + 1];
    memset(sandbox, 'S', 30);
    sandbox[30] = 0;
    char *buf = sandbox + 10;

    strcpy(buf, input);

    if (test_verbose) {
        printf("|%s| ==> ", buf);
    }
    replace_all(buf, 10, what, rep);
    if (test_verbose) {
        printf("|%s|\n", buf);
    }
    // check the result
    sprintf(test_msg, "for input |%s|\ngot |%s|\nexp |%s|\n", input, buf,
            exp);
    mu_assert(test_msg, 0 == strcmp(buf, exp));

    // check if the rest of sandbox is intact
    for (int i = 0; i < 30; ++i) {
        if (10 <= i && i < 20) {
            continue;
        }
        sprintf(test_msg, "wrong char at %d: 0x%02x", i, sandbox[i]);
        mu_assert(test_msg, sandbox[i] == 'S');
    }

    return NULL;
}

static char *all_tests()
{
    // basic replace tests
    // - 'what' is empty
    mu_run_test(test_replace_all("1noop", "", "A", "1noop"));
    // - 'repl' is empty
    mu_run_test(test_replace_all("2noop", "X", "", "2noop"));
    mu_run_test(test_replace_all("2noop", "o", "", "2np"));
    mu_run_test(test_replace_all("onoop", "o", "", "np"));
    mu_run_test(test_replace_all("0000", "0000", "", ""));
    // - 'what' not found
    mu_run_test(test_replace_all("3noop", "X", "Y", "3noop"));
    // - simple replacement
    mu_run_test(test_replace_all("012345", "2", "Y", "01Y345"));
    mu_run_test(test_replace_all("012322", "2", "Y", "01Y3YY"));
    mu_run_test(test_replace_all("012345678", "8", "Y", "01234567Y"));
    // - shorter replacement
    mu_run_test(test_replace_all("001200300", "00", "Y", "Y12Y3Y"));
    mu_run_test(test_replace_all("000000", "00", "Y", "YYY"));
    mu_run_test(test_replace_all("0000", "0000", "YW", "YW"));
    // - longer replacement
    mu_run_test(test_replace_all("012345", "0", "YW", "YW12345"));
    mu_run_test(test_replace_all("012305", "0", "YW", "YW123YW5"));
    mu_run_test(test_replace_all("0123405", "0", "YW", "YW1234YW5"));
    mu_run_test(test_replace_all("01234505", "0", "YW", "YW12345YW"));
    mu_run_test(test_replace_all("012345605", "0", "YW", "YW123456Y"));
    mu_run_test(test_replace_all("012340", "0", "YW", "YW1234YW"));
    mu_run_test(test_replace_all("0120", "0", "YWX", "YWX12YWX"));
    mu_run_test(test_replace_all("01230", "0", "YWX", "YWX123YWX"));
    mu_run_test(test_replace_all("012340", "0", "YWX", "YWX1234YW"));
    mu_run_test(test_replace_all("0123450", "0", "YWX", "YWX12345Y"));
    mu_run_test(test_replace_all("01234560", "0", "YWX", "YWX123456"));
    mu_run_test(test_replace_all("01234567", "0", "YWX", "YWX123456"));
    mu_run_test(test_replace_all("012305", "0", "YWX", "YWX123YWX"));
    mu_run_test(test_replace_all("012300", "0", "YWX", "YWX123YWX"));
    mu_run_test(test_replace_all("01200", "0", "YWX", "YWX12YWXY"));
    mu_run_test(test_replace_all("1200", "0", "YWX", "12YWXYWX"));
    mu_run_test(test_replace_all("1234567", "7", "YWX", "123456YWX"));
    mu_run_test(test_replace_all("1234567", "4", "YWX", "123YWX567"));
    mu_run_test(test_replace_all("12345678", "4", "YWX", "123YWX567"));
    mu_run_test(test_replace_all("1234567", "7", "YWXZ", "123456YWX"));
    mu_run_test(test_replace_all("1", "1", "ABCDEFGHIJKL", "ABCDEFGHI"));
    mu_run_test(test_replace_all("11", "1", "ABCDEFGHIJKL", "ABCDEFGHI"));

    // ExpandMacros tests
    strcpy(call, "N1CALL\n");   // code adds a trailing \n, see read_logcfg()
    strcpy(hiscall, "TE5T");
    strcpy(comment, "comment");
    strcpy(his_rst, "599");

    mu_run_test(test_ExpandMacro("noop1", "noop1"));
    mu_run_test(test_ExpandMacro("a % b", "a N1CALL b"));

    shortqsonr = LONGCW;
    noleadingzeros = 0;
    mu_run_test(test_ExpandMacro("a % b #", "a N1CALL b 001"));

    noleadingzeros = 1;
    mu_run_test(test_ExpandMacro("a % b #", "a N1CALL b 1"));

    shortqsonr = SHORTCW;
    noleadingzeros = 0;
    mu_run_test(test_ExpandMacro("a % b #", "a N1CALL b TT1"));

    noleadingzeros = 1;
    mu_run_test(test_ExpandMacro("a % b #", "a N1CALL b 1"));
    noleadingzeros = 0;

    strcpy(qsonrstr, "1294");
    mu_run_test(test_ExpandMacro("a !b #", "a commentb 12N4"));
    strcpy(qsonrstr, "0294");
    mu_run_test(test_ExpandMacro("a !b #", "a commentb 2N4"));

    strcpy(qsonrstr, "0039");
    shortqsonr = SHORTCW;
    mu_run_test(test_ExpandMacro("a b[ #", "a b5NN T3N"));
    shortqsonr = LONGCW;
    mu_run_test(test_ExpandMacro("a b[ #", "a b599 039"));

    hiscall_sent[0] = '\0';
    mu_run_test(test_ExpandMacro("@ @ de %", "TE5T TE5T de N1CALL"));
    strcpy(hiscall_sent, "TE");
    mu_run_test(test_ExpandMacro("@ @ de %", "5T TE5T de N1CALL"));

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        test_verbose = 1;
    }

    char *result = all_tests();

    if (result) {
        printf("TEST FAILED:\n%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
