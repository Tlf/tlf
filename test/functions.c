#include "test.h"

int key_get() {
    return -1;
}

const char STRING_NOT_SET[] = "__NOT_SET__";

const char *showmsg_spy = STRING_NOT_SET;
const char *showstring_spy1 = STRING_NOT_SET;
const char *showstring_spy2 = STRING_NOT_SET;

void showmsg(char *message) {
    showmsg_spy = message;
}

void showstring(char *message1, char *message2) {
    showstring_spy1 = message1;
    showstring_spy2 = message2;
}

unsigned int __wrap_sleep(unsigned int seconds) {
    // no sleeping in tests
    return 0;
}

