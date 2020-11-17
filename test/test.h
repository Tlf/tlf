#ifndef _TEST_H
#define _TEST_H

//
// common includes for the test groups
//

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <stdbool.h>

#include <glib.h>

#include <cmocka.h>

extern const char STRING_NOT_SET[];

extern const char *showmsg_spy, *showstring_spy1, *showstring_spy2;

typedef int (*int_func_t)(void);

// plugin functions for the mocks
extern int_func_t key_poll_func;
extern int_func_t getch_func;

// the real functions
extern int __real_key_poll();

// number of mvprintw lines to store
#define NLAST   30
// length of a mvprintw line
#define LINESZ  100
extern char mvprintw_history[NLAST][LINESZ];
extern void clear_mvprintw_history();

#endif
