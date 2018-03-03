#include "test.h"

#include "curses.h"

int __wrap_key_get() {
    return -1;
}

// wrap with a plugin function
int_func_t key_poll_func = NULL;

int __wrap_key_poll() {
    if (key_poll_func != NULL) {
	return key_poll_func();
    }
    return -1;
}

int_func_t getch_func = NULL;

int __wrap_wgetch(WINDOW *w) {
    if (getch_func != NULL) {
	return getch_func();
    }
    return -1;
}

void __wrap_refreshp() {
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

