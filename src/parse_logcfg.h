/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */


#ifndef PARSE_LOGCFG_H
#define PARSE_LOGCFG_H

#include <stdbool.h>

enum {
    PARSE_OK,
    PARSE_ERROR,
    PARSE_NO_MATCH,
    PARSE_MISSING_PARAMETER,
    PARSE_EXTRA_PARAMETER,
    PARSE_WRONG_PARAMETER,
    PARSE_INVALID_INTEGER,
    PARSE_INTEGER_OUT_OF_RANGE,
    PARSE_STRING_TOO_LONG,
};

int read_logcfg(void);
int parse_configfile(FILE *fp);
int parse_logcfg(char *inputbuffer);

extern char *error_details;

////////////////////////////////////
// config parsing definitions

enum {
    NO_PARAM,
    NEED_PARAM,
    OPTIONAL_PARAM,
};

enum {
    DYNAMIC,
    STATIC,
    MESSAGE,
};

typedef struct {
    union {     // targets
	int *int_p;
	bool *bool_p;
	char *char_p;
	char **char_pp;
	char (*msg)[80];
	size_t offset;
    };
    union {     // extra info
	int int_value;
	bool bool_value;
	struct {
	    int string_type;
	    int base, size;
	    bool chomp, strip, nl_to_space;
	};
	struct {
	    int min, max;
	};
    };
} cfg_arg_t;

typedef struct {
    char *regex;
    int param_kind;
    int (*func)(const cfg_arg_t arg);
    cfg_arg_t arg;
} config_t;

#define CFG_BOOL_TRUE(var)  NO_PARAM, cfg_bool_const, \
        (cfg_arg_t){.bool_p=&var, .bool_value=true}

#define CFG_CONTEST_BOOL_TRUE(var)  NO_PARAM, cfg_contest_bool_const, \
	(cfg_arg_t){.offset=offsetof(contest_config_t, var), \
	    .bool_value=true}

#define CFG_INT_CONST(var,n)    NO_PARAM, cfg_int_const, \
        (cfg_arg_t){.int_p=&var, .int_value=n}

#define CFG_INT_ONE(var)    CFG_INT_CONST(var, 1)

#define CFG_INT(var,minval,maxval)  NEED_PARAM, cfg_integer, \
        (cfg_arg_t){.int_p=&var, .min=minval, .max=maxval}

#define CFG_STRING_STATIC(var,bufsize)  NEED_PARAM, cfg_string, \
        (cfg_arg_t){.char_p=var, .size=bufsize, .chomp=true, \
                    .string_type=STATIC}

#define CFG_STRING(var)         NEED_PARAM, cfg_string, \
        (cfg_arg_t){.char_pp=&var, .chomp=true, \
                    .string_type=DYNAMIC}

#define CFG_MESSAGE(var, i)   NEED_PARAM, cfg_string, \
        (cfg_arg_t){.msg=var, .base=i, .size=80, .chomp=true, \
                    .string_type=MESSAGE}

#define CFG_MESSAGE_DYNAMIC(var, i) NEED_PARAM, cfg_string, \
        (cfg_arg_t){.char_pp=var, .base=i, .size=80, .nl_to_space=true, \
                    .string_type=DYNAMIC}

// FIXME: remove NOCHOMPs
#define CFG_STRING_STATIC_NOCHOMP(var,bufsize) NEED_PARAM, cfg_string, \
        (cfg_arg_t){.char_p=var, .size=bufsize, \
                    .string_type=STATIC}

#define CFG_STRING_NOCHOMP(var) NEED_PARAM, cfg_string, \
        (cfg_arg_t){.char_pp=&var, \
                    .string_type=DYNAMIC}


#endif // PARSE_LOGCFG_H
