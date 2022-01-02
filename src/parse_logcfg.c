/*
* Tlf - contest logging program for amateur radio operators
* Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0rct@amsat.org>
* 		2011-2020           Thomas Beierlein <tb@forth-ev.de>
* 		2013 		    Fred DH5FS
*               2013-2016           Ervin Hegedus - HA2OS <airween@gmail.com>
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


#include <ctype.h>
#include <fcntl.h>
#include <hamlib/rig.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "bandmap.h"
#include "cabrillo_utils.h"
#include "change_rst.h"
#include "cw_utils.h"
#include "fldigixmlrpc.h"
#include "globalvars.h"
#include "getctydata.h"
#include "getexchange.h"
#include "getpx.h"
#include "getwwv.h"
#include "ignore_unused.h"
#include "lancode.h"
#include "locator2longlat.h"
#include "parse_logcfg.h"
#include "qtcvars.h"		// Includes globalvars.h
#include "setcontest.h"
#include "set_tone.h"
#include "startmsg.h"
#include "tlf_curses.h"
#include "searchlog.h"

bool exist_in_country_list();

void KeywordNotSupported(const char *keyword);
void ParameterNeeded(const char *keyword);
void ParameterUnexpected(const char *keyword);
void WrongFormat(const char *keyword);
void WrongFormat_details(const char *keyword, const char *details);

char *error_details = NULL;

#define LOGCFG_DAT_FILE    "logcfg.dat"

int read_logcfg(void) {

    static char defltconf[] = PACKAGE_DATA_DIR "/" LOGCFG_DAT_FILE;
    FILE *fp;

    if (config_file != NULL) {
	fp = fopen(config_file, "r");
	if (fp == NULL) {
	    showstring("Error opening config file: ", config_file);
	    return PARSE_ERROR;
	}
    } else {
	config_file = g_strdup(LOGCFG_DAT_FILE);

	if (access(config_file, R_OK) == -1) {
	    showmsg("No logcfg.dat found. Copying default config file.");
	    showmsg("Please adapt the settings to your needs.");
	    char *cmd = g_strdup_printf("cp %s %s", defltconf,
					LOGCFG_DAT_FILE);
	    IGNORE(system(cmd));
	    g_free(cmd);
	    sleep(2);
	}
	if ((fp = fopen(config_file, "r")) == NULL) {
	    showmsg("Error opening logcfg.dat file.");
	    return PARSE_ERROR;
	}

    }
    showstring("Reading config file:", config_file);

    int status = parse_configfile(fp);
    fclose(fp);

    return status;
}

static bool isCommentLine(char *buffer) {
    return buffer[0] == 0 || buffer[0] == '#' || buffer[0] == ';';
}

int parse_configfile(FILE *fp) {
    int status = PARSE_OK;
    char buffer[160];

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
	g_strchug(buffer);              // remove leading space
	if (isCommentLine(buffer)) {    // skip comments and empty lines
	    continue;
	}

	status = parse_logcfg(buffer);
	if (status != PARSE_OK) {
	    break;
	}
    }

    return status;
}

/** convert band string into index number (0..NBANDS-2) */
// note: NBANDS-1 is the OOB
int getidxbybandstr(char *confband) {
    char buf[8];

    g_strchomp(confband);

    for (int i = 0; i < NBANDS - 1; i++) {
	strcpy(buf, band[i]);
	if (strcmp(confband, g_strchug(buf)) == 0) {
	    return i;
	}
    }
    return -1;
}

////////////////////
// global variables for matcher functions:
GMatchInfo *match_info;
const char *parameter;

static int parse_int(const char *string, gint64 min, gint64 max, int *result) {

    gchar *str = g_strdup(string);
    g_strstrip(str);

    if (str[0] == 0) {  // empty input
	g_free(str);
	return PARSE_INVALID_INTEGER;
    }

    gchar *end_ptr = NULL;
    errno = 0;
    gint64 value = g_ascii_strtoll(str, &end_ptr, 10);

    if ((errno != 0 && errno != ERANGE)
	    || end_ptr == NULL || *end_ptr != 0) {

	g_free(str);
	return PARSE_INVALID_INTEGER;
    }

    g_free(str);

    if (errno == ERANGE || value < min || value > max) {
	return PARSE_INTEGER_OUT_OF_RANGE;
    }

    *result = (int)value;
    return PARSE_OK;
}

int cfg_bool_const(const cfg_arg_t arg) {
    *arg.bool_p = arg.bool_value;
    return PARSE_OK;
}

int cfg_contest_bool_const(const cfg_arg_t arg) {
    *(bool *)((char *)contest + arg.offset) = arg.bool_value;
    return PARSE_OK;
}

int cfg_int_const(const cfg_arg_t arg) {
    *arg.int_p  = arg.int_value;
    return PARSE_OK;
}

int cfg_integer(const cfg_arg_t arg) {
    return parse_int(parameter, (gint64)arg.min, (gint64)arg.max, arg.int_p);
}

//
// static: char_p, size > 0, base not used
// dynamic: char_pp, size optional, base optional
// message: msg, size > 0, base used
//
int cfg_string(const cfg_arg_t arg) {
    gchar *index = g_match_info_fetch(match_info, 1);
    int n = 0;
    if (NULL != index) {
	n = atoi(index);    // use provided index
	g_free(index);
    }

    char *str = g_strdup(parameter);
    // chomp/strip
    if (arg.chomp) {
	g_strchomp(str);
    }
    if (arg.strip) {
	g_strstrip(str);
    }
    // check length
    if (arg.size > 0 && strlen(str) >= arg.size) {
	g_free(str);
	return PARSE_STRING_TOO_LONG;
    }
    // replace trailing newline with a space
    if (arg.nl_to_space) {
	char *nl = strrchr(str, '\n');
	if (nl) {
	    *nl = ' ';
	}
    }

    // store value
    switch (arg.string_type) {
	case STATIC:
	    g_strlcpy(arg.char_p, str, arg.size);
	    g_free(str);
	    break;
	case MESSAGE:
	    g_strlcpy(arg.msg[arg.base + n], str, arg.size);
	    g_free(str);
	    break;
	case DYNAMIC:
	    if (arg.char_pp[arg.base + n] != NULL) {
		g_free(arg.char_pp[arg.base + n]);
	    }
	    arg.char_pp[arg.base + n] = str;
    }
    return PARSE_OK;
}

static int cfg_telnetport(const cfg_arg_t arg) {
    int rc = cfg_integer((cfg_arg_t) {.int_p = &portnum, .min = 1, .max = INT32_MAX});
    if (rc != PARSE_OK) {
	return rc;
    }
    packetinterface = TELNET_INTERFACE;
    return PARSE_OK;
}

// define colors: GREEN (header), CYAN (windows), WHITE (log win),
//  MAGENTA (marker / dupes), BLUE (input field) and YELLOW (window frames)
static int cfg_tlfcolor(const cfg_arg_t arg) {
    gchar *index = g_match_info_fetch(match_info, 1);
    int n = atoi(index);    // get index (1..6)
    g_free(index);

    int rc = PARSE_OK;
    char *str = g_strdup(parameter);
    g_strstrip(str);

    if (strlen(str) == 2 && isdigit(str[0]) && isdigit(str[1])) {
	if (n >= 2) {
	    ++n; // skip RED
	}
	tlfcolors[n][0] = str[0] - '0';
	tlfcolors[n][1] = str[1] - '0';
    } else {
	error_details = g_strdup("must be a 2 digit octal number");
	rc = PARSE_WRONG_PARAMETER;
    }

    g_free(str);

    return rc;
}

static int cfg_call(const cfg_arg_t arg) {
    int rc = cfg_string((cfg_arg_t) {
	.char_p = my.call, .size = sizeof(my.call),
	.strip = true, .string_type = STATIC
    });
    if (rc != PARSE_OK) {
	return rc;
    }
    if (strlen(my.call) <= 2) {
	error_details = g_strdup("too short");
	return PARSE_WRONG_PARAMETER;
    }

    for (char *p = my.call; *p; ++p) {
	*p = g_ascii_toupper(*p);
    }

    return PARSE_OK;
}

static int cfg_contest(const cfg_arg_t arg) {
    char contest[41];
    int rc = cfg_string((cfg_arg_t) {
	.char_p = contest, .size = 40, .strip = true, .string_type = STATIC
    });
    if (rc != PARSE_OK) {
	return rc;
    }
    setcontest(contest);
    return PARSE_OK;
}

static int cfg_bandoutput(const cfg_arg_t arg) {
    char *str = g_strdup(parameter);
    g_strstrip(str);

    int rc = PARSE_OK;

    if (g_regex_match_simple("^\\d{10}$", str, G_REGEX_CASELESS,
			     (GRegexMatchFlags)0)) {
	use_bandoutput = 1;
	for (int i = 0; i <= 9; i++) {	// 10x
	    bandindexarray[i] = str[i] - '0';
	}
    } else {
	error_details = g_strdup_printf("must be %d digits", NBANDS);
	rc = PARSE_WRONG_PARAMETER;
    }


    g_free(str);

    return rc;
}

static int cfg_n_points(const cfg_arg_t arg) {
    gchar *keyword = g_match_info_fetch(match_info, 0);

    if (g_str_has_prefix(keyword, "ONE")) {
	contest->points.type = FIXED;
	contest->points.point = 1;
    } else if (g_str_has_prefix(keyword, "TWO")) {
	contest->points.type = FIXED;
	contest->points.point = 2;
    } else if (g_str_has_prefix(keyword, "THREE")) {
	contest->points.type = FIXED;
	contest->points.point = 3;
    }

    g_free(keyword);

    return PARSE_OK;
}

static int cfg_bandmap(const cfg_arg_t arg) {
    cluster = MAP;

    /* init bandmap filtering */
    bm_config.allband = 1;
    bm_config.allmode = 1;
    bm_config.showdupes = 1;
    bm_config.skipdupes = 0;
    bm_config.livetime = 900;
    bm_config.onlymults = 0;

    /* Allow configuration of bandmap display if keyword
     * is followed by a '='
     * Parameter format is BANDMAP=<xxx>,<number>
     * <xxx> - string parsed for the letters B, M, D and S
     * <number> - spot livetime in seconds (>=30)
     */
    if (parameter != NULL) {
	char **bm_fields = g_strsplit(parameter, ",", 2);
	if (bm_fields[0] != NULL) {
	    char *ptr = bm_fields[0];
	    while (*ptr != '\0') {
		switch (*ptr++) {
		    case 'B': bm_config.allband = 0;
			break;
		    case 'M': bm_config.allmode = 0;
			break;
		    case 'D': bm_config.showdupes = 0;
			break;
		    case 'S': bm_config.skipdupes = 1;
			break;
		    case 'O': bm_config.onlymults = 1;
			break;
		    default:
			break;
		}
	    }
	}

	if (bm_fields[1] != NULL) {
	    int livetime;
	    g_strstrip(bm_fields[1]);
	    livetime = atoi(bm_fields[1]);
	    if (livetime >= 30)
		/* aging called each second */
		bm_config.livetime = livetime;
	}


	g_strfreev(bm_fields);
    }

    return PARSE_OK;
}

static int cfg_cwspeed(const cfg_arg_t arg) {
    int value = 0;	/* avoid warning about uninitialized variables */
    int rc = cfg_integer((cfg_arg_t) {.int_p = &value, .min = 6, .max = 60});
    if (rc != PARSE_OK) {
	return rc;
    }
    SetCWSpeed(value);
    return PARSE_OK;
}

static int cfg_cwtone(const cfg_arg_t arg) {
    int value = 0;	/* avoid warning about uninitialized variables */
    int rc = cfg_integer((cfg_arg_t) {.int_p = &value, .min = 0, .max = 999});
    if (rc != PARSE_OK) {
	return rc;
    }
    sprintf(tonestr, "%d", value);
    return PARSE_OK;
}

static int cfg_sunspots(const cfg_arg_t arg) {
    int value = 0;	/* avoid warning about uninitialized variables */
    int rc = cfg_integer((cfg_arg_t) {.int_p = &value, .min = 0, .max = 1000});
    if (rc != PARSE_OK) {
	return rc;
    }
    wwv_set_r(value);
    return PARSE_OK;
}

static int cfg_sfi(const cfg_arg_t arg) {
    int value = 0;	/* avoid warning about uninitialized variables */
    int rc = cfg_integer((cfg_arg_t) {.int_p = &value, .min = 0, .max = 1000});
    if (rc != PARSE_OK) {
	return rc;
    }
    wwv_set_sfi(value);
    return PARSE_OK;
}

static int cfg_tncport(const cfg_arg_t arg) {

    strncpy(tncportname, parameter, 39);

    packetinterface = TNC_INTERFACE;
    return PARSE_OK;
}

static int cfg_addnode(const cfg_arg_t arg) {
    if (nodes >= MAXNODES) {
	error_details = g_strdup_printf("max %d nodes allowed", MAXNODES);
	return PARSE_WRONG_PARAMETER;
    }
    /* split host name and port number, separated by colon */
    char **an_fields;
    an_fields = g_strsplit(parameter, ":", 2);
    /* copy host name */
    g_strlcpy(bc_hostaddress[nodes], g_strchomp(an_fields[0]),
	      sizeof(bc_hostaddress[0]));
    if (an_fields[1] != NULL) {
	/* copy host port, if found */
	g_strlcpy(bc_hostservice[nodes], g_strchomp(an_fields[1]),
		  sizeof(bc_hostservice[0]));
    }
    g_strfreev(an_fields);

    nodes++;
    lan_active = true;

    return PARSE_OK;
}

static int cfg_thisnode(const cfg_arg_t arg) {
    char *str = g_ascii_strup(parameter, -1);
    g_strstrip(str);

    if (strlen(str) != 1 || str[0] < 'A' || str[0] > 'A' + MAXNODES) {
	g_free(str);
	error_details = g_strdup_printf("name is A..%c", 'A' + MAXNODES - 1);
	return PARSE_WRONG_PARAMETER;
    }

    thisnode = str[0];

    g_free(str);
    return PARSE_OK;
}

static int cfg_mult_list(const cfg_arg_t arg) {
    int rc = cfg_string((cfg_arg_t) {
	.char_p = multsfile, .size = 80, .strip = true, .string_type = STATIC
    });
    if (rc != PARSE_OK) {
	return rc;
    }
    multlist = 1;
    return PARSE_OK;
}

static int cfg_markers(const cfg_arg_t arg) {
    int rc = cfg_string((cfg_arg_t) {
	.char_p = markerfile, .size = 120, .strip = true, .string_type = STATIC
    });
    if (rc != PARSE_OK) {
	return rc;
    }

    gchar *type = g_match_info_fetch(match_info, 1);
    if (strcmp(type, "") == 0) {
	xplanet = MARKER_ALL;
    } else if (strcmp(type, "DOT") == 0) {
	xplanet = MARKER_DOTS;
    } else if (strcmp(type, "CALL") == 0) {
	xplanet = MARKER_CALLS;
    }
    g_free(type);
    return PARSE_OK;
}

static int cfg_dx_n_sections(const cfg_arg_t arg) {
    dx_arrlsections = true;
    setcontest(whichcontest);
    return PARSE_OK;
}

static int cfg_countrylist(const cfg_arg_t arg) {
    /* FIXME: why "use only first COUNTRY_LIST definition" ? */
    /*static*/ char country_list_raw[50] = "";
    char temp_buffer[255] = "";
    char buffer[255] = "";
    FILE *fp;

    if (strlen(country_list_raw) == 0) {/* only if first definition */

	/* First of all we are checking if the parameter <xxx> in
	COUNTRY_LIST=<xxx> is a file name.  If it is we start
	parsing the file. If we  find a line starting with our
	case insensitive contest name, we copy the countries from
	that line into country_list_raw.
	If the input was not a file name we directly copy it into
	country_list_raw (must not have a preceeding contest name). */

	g_strlcpy(temp_buffer, parameter, sizeof(temp_buffer));
	g_strchomp(temp_buffer);	/* drop trailing whitespace */

	if ((fp = fopen(temp_buffer, "r")) != NULL) {

	    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

		g_strchomp(buffer);   /* no trailing whitespace*/

		/* accept only a line starting with the contest name
		 * (CONTEST=) followed by ':' */
		if (strncasecmp(buffer, whichcontest,
				strlen(whichcontest) - 1) == 0) {

		    strncpy(country_list_raw,
			    buffer + strlen(whichcontest) + 1,
			    strlen(buffer) - 1);
		}
	    }

	    fclose(fp);
	} else {	/* not a file */

	    if (strlen(temp_buffer) > 0)
		strcpy(country_list_raw, temp_buffer);
	}
    }

    /* parse the country_list_raw string into an array
     * (countrylist) for future use. */
    char *tk_ptr = strtok(country_list_raw, ":,.- \t");
    int counter = 0;

    if (tk_ptr != NULL) {
	while (tk_ptr) {
	    strcpy(countrylist[counter], tk_ptr);
	    tk_ptr = strtok(NULL, ":,.-_\t ");
	    counter++;  //FIXME: check index and clean not touched records
	}
    }

    /* on which multiplier side of the rules we are */
    getpx(my.call);
    mult_side = exist_in_country_list();
    setcontest(whichcontest);

    return PARSE_OK;
}

static int cfg_continentlist(const cfg_arg_t arg) {
    /* based on LZ3NY code, by HA2OS
       CONTINENT_LIST   (in file or listed in logcfg.dat),
       First of all we are checking if inserted data in
       CONTINENT_LIST= is a file name.  If it is we start
       parsing the file. If we got our case insensitive contest name,
       we copy the multipliers from it into multipliers_list.
       If the input was not a file name we directly copy it into
       cont_multiplier_list (must not have a preceeding contest name).
       The last step is to parse the multipliers_list into an array
       (continent_multiplier_list) for future use.
     */

    /* use only first CONTINENT_LIST definition */
    /*static*/ char cont_multiplier_list[50] = "";
    char temp_buffer[255] = "";
    char buffer[255] = "";
    FILE *fp;

    if (strlen(cont_multiplier_list) == 0) {	/* if first definition */
	g_strlcpy(temp_buffer, parameter, sizeof(temp_buffer));
	g_strchomp(temp_buffer);	/* drop trailing whitespace */

	if ((fp = fopen(temp_buffer, "r")) != NULL) {

	    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

		g_strchomp(buffer);   /* no trailing whitespace*/

		/* accept only a line starting with the contest name
		 * (CONTEST=) followed by ':' */
		if (strncasecmp(buffer, whichcontest,
				strlen(whichcontest) - 1) == 0) {

		    strncpy(cont_multiplier_list,
			    buffer + strlen(whichcontest) + 1,
			    strlen(buffer) - 1);
		}
	    }

	    fclose(fp);
	} else {	/* not a file */

	    if (strlen(temp_buffer) > 0)
		strcpy(cont_multiplier_list, temp_buffer);
	}
    }

    /* creating the array */
    char *tk_ptr = strtok(cont_multiplier_list, ":,.- \t");
    int counter = 0;

    if (tk_ptr != NULL) {
	while (tk_ptr) {
	    strncpy(continent_multiplier_list[counter], tk_ptr, 2);
	    tk_ptr = strtok(NULL, ":,.-_\t ");
	    counter++;  // FIXME check range + clean + value length check
	}
    }

    setcontest(whichcontest);

    return PARSE_OK;
}

static int cfg_country_list_only(const cfg_arg_t arg) {
    countrylist_only = true;
    if (mult_side) {
	countrylist_only = false;
    }
    return PARSE_OK;
}

static int cfg_bandweight_points(const cfg_arg_t arg) {
    static char bwp_params_list[50] = "";
    int bandindex = -1;

    if (strlen(bwp_params_list) == 0) {
	g_strlcpy(bwp_params_list, parameter, sizeof(bwp_params_list));
	g_strchomp(bwp_params_list);
    }

    char *tk_ptr = strtok(bwp_params_list, ";:,");
    if (tk_ptr != NULL) {
	while (tk_ptr) {

	    bandindex = getidxbybandstr(g_strchomp(tk_ptr));
	    tk_ptr = strtok(NULL, ";:,");
	    if (tk_ptr != NULL && bandindex >= 0) {
		bandweight_points[bandindex] = atoi(tk_ptr);
	    }
	    tk_ptr = strtok(NULL, ";:,");
	}
    }
    return PARSE_OK;
}

static int cfg_bandweight_multis(const cfg_arg_t arg) {
    static char bwm_params_list[50] = "";
    int bandindex = -1;

    if (strlen(bwm_params_list) == 0) {
	g_strlcpy(bwm_params_list, parameter, sizeof(bwm_params_list));
	g_strchomp(bwm_params_list);
    }

    char *tk_ptr = strtok(bwm_params_list, ";:,");
    if (tk_ptr != NULL) {
	while (tk_ptr) {

	    bandindex = getidxbybandstr(g_strchomp(tk_ptr));
	    tk_ptr = strtok(NULL, ";:,");
	    if (tk_ptr != NULL && bandindex >= 0) {
		bandweight_multis[bandindex] = atoi(tk_ptr);
	    }
	    tk_ptr = strtok(NULL, ";:,");
	}
    }
    return PARSE_OK;
}

static int cfg_pfx_num_multis(const cfg_arg_t arg) {
    /* based on LZ3NY code, by HA2OS
       PFX_NUM_MULTIS   (in file or listed in logcfg.dat),
       We directly copy it into pfxnummulti_str, then parse the prefixlist
       and fill the pfxnummulti array.
     */

    int counter = 0;
    int pfxnum;
    static char pfxnummulti_str[50] = "";
    char parsepfx[15] = "";

    g_strlcpy(pfxnummulti_str, parameter, sizeof(pfxnummulti_str));
    g_strchomp(pfxnummulti_str);

    /* creating the array */
    char *tk_ptr = strtok(pfxnummulti_str, ",");
    counter = 0;

    if (tk_ptr != NULL) {
	while (tk_ptr) {
	    parsepfx[0] = '\0';
	    if (isdigit(tk_ptr[strlen(tk_ptr) - 1])) {
		sprintf(parsepfx, "%sAA", tk_ptr);
	    } else {
		sprintf(parsepfx, "%s0AA", tk_ptr);
	    }
	    pfxnummulti[counter].countrynr = getctydata(parsepfx);
	    for (pfxnum = 0; pfxnum < 10; pfxnum++) {
		pfxnummulti[counter].qsos[pfxnum] = 0;
	    }
	    tk_ptr = strtok(NULL, ",");
	    counter++;
	}
    }
    pfxnummultinr = counter;
    setcontest(whichcontest);
    return PARSE_OK;
}

static int cfg_sc_volume(const cfg_arg_t arg) {
    int value = 0;
    int rc = cfg_integer((cfg_arg_t) {.int_p = &value, .min = 0, .max = 100});
    if (rc != PARSE_OK) {
	return rc;
    }
    sprintf(sc_volume, "%d", value);
    return PARSE_OK;
}

static int cfg_mfj1278_keyer(const cfg_arg_t arg) {
    int rc = cfg_string((cfg_arg_t) {
	.char_p = controllerport, .size = 80, .strip = true, .string_type = STATIC
    });
    if (rc != PARSE_OK) {
	return rc;
    }
    cwkeyer = MFJ1278_KEYER;
    digikeyer = MFJ1278_KEYER;
    return PARSE_OK;
}

static int cfg_gmfsk(const cfg_arg_t arg) {
    int rc = cfg_string((cfg_arg_t) {
	.char_p = controllerport, .size = 80, .strip = true, .string_type = STATIC
    });
    if (rc != PARSE_OK) {
	return rc;
    }
    digikeyer = GMFSK;
    return PARSE_OK;
}

static int cfg_change_rst(const cfg_arg_t arg) {
    change_rst = true;
    if (parameter == NULL) {
	rst_init(NULL);
	return PARSE_OK;
    }
    char *str = g_strdup(parameter);
    g_strstrip(str);
    /* comma separated list of RS(T) values 33..39, 43..39, 53..59 allowed.  */
    if (!g_regex_match_simple("^([3-5][3-9]\\d?\\s*,\\s*)*[3-5][3-9]\\d?$",
			      str, G_REGEX_CASELESS, (GRegexMatchFlags)0)) {
	g_free(str);
	error_details = g_strdup("must be a comma separated list of RS(T) values");
	return PARSE_WRONG_PARAMETER;
    }

    rst_init(str);

    g_free(str);
    return PARSE_OK;
}

static int cfg_rttymode(const cfg_arg_t arg) {
    trxmode = DIGIMODE;
    strcpy(modem_mode, "RTTY");
    return PARSE_OK;
}

static int cfg_myqra(const cfg_arg_t arg) {
    strcpy(my.qra, parameter);

    if (check_qra(my.qra) == 0) {
	return PARSE_WRONG_PARAMETER;
    }

    return PARSE_OK;
}

static int cfg_powermult(const cfg_arg_t arg) {
    if (fixedmult == 0.0 && atof(parameter) > 0.0) {
	fixedmult = atof(parameter);
    }

    return PARSE_OK;
}

static int cfg_qtc(const cfg_arg_t arg) {
    char *str = g_ascii_strup(parameter, -1);
    g_strstrip(str);

    int rc = PARSE_OK;

    if (strcmp(str, "RECV") == 0) {
	qtcdirection = RECV;
    } else if (strcmp(str, "SEND") == 0) {
	qtcdirection = SEND;
    } else if (strcmp(str, "BOTH") == 0) {
	qtcdirection = RECV | SEND;
    } else {
	error_details = g_strdup("must be RECV, SEND, or BOTH");
	rc = PARSE_WRONG_PARAMETER;
    }

    g_free(str);
    return rc;
}

static int cfg_qtcrec_record_command(const cfg_arg_t arg) {
    int p, q = 0, i = 0, s = 0;
    for (p = 0; p < strlen(parameter); p++) {
	if (p > 0 && parameter[p] == ' ') {
	    s = 1;
	    qtcrec_record_command_shutdown[p] = '\0';
	}
	if (s == 0) {
	    qtcrec_record_command_shutdown[p] = parameter[p];
	}
	if (parameter[p] == '$') {
	    qtcrec_record_command[i][q] = '\0';
	    i = 1;
	    p++;
	    q = 0;
	}
	if (parameter[p] != '\n') {
	    qtcrec_record_command[i][q] = parameter[p];
	}
	q++;
	qtcrec_record_command[i][q] = ' ';
    }

    if (qtcrec_record_command[i][q - 1] != '&') {
	qtcrec_record_command[i][q++] = ' ';
	qtcrec_record_command[i][q++] = '&';
    }
    qtcrec_record_command[i][q] = '\0';

    return PARSE_OK;
}

static int cfg_exclude_multilist(const cfg_arg_t arg) {
    char *str = g_ascii_strup(parameter, -1);
    g_strstrip(str);

    if (strcmp(str, "CONTINENTLIST") == 0) {
	g_free(str);
	if (strlen(continent_multiplier_list[0]) == 0) {
	    error_details = g_strdup("need to set the CONTINENTLIST first");
	    return PARSE_WRONG_PARAMETER;
	}
	exclude_multilist_type = EXCLUDE_CONTINENT;
    } else if (strcmp(str, "COUNTRYLIST") == 0) {
	g_free(str);
	if (strlen(countrylist[0]) == 0) {
	    error_details = g_strdup("need to set the COUNTRYLIST first");
	    return PARSE_WRONG_PARAMETER;
	}
	exclude_multilist_type = EXCLUDE_COUNTRY;
    } else {
	g_free(str);
	error_details = g_strdup("must be CONTINENTLIST or COUNTRYLIST");
	return PARSE_WRONG_PARAMETER;
    }

    return PARSE_OK;
}

static int cfg_fldigi(const cfg_arg_t arg) {
#ifndef HAVE_LIBXMLRPC
    showmsg("WARNING: XMLRPC not compiled - skipping setup.");
    sleep(2);
    digikeyer = NO_KEYER;
#else
    if (parameter != NULL) {
	int rc = cfg_string((cfg_arg_t) {
	    .char_p = fldigi_url, .size = sizeof(fldigi_url), .strip = true,
	    .string_type = STATIC
	});
	if (rc != PARSE_OK) {
	    return rc;
	}
    }

    digikeyer = FLDIGI;
    if (!fldigi_isenabled()) {
	fldigi_toggle();
    }
#endif

    return PARSE_OK;
}

static int cfg_minitest(const cfg_arg_t arg) {
    if (parameter == NULL) {
	minitest = MINITEST_DEFAULT_PERIOD;
	return PARSE_OK;
    }

    int value = 1;	/* avoid warning about divide by zero */
    int rc = cfg_integer((cfg_arg_t) {.int_p = &value, .min = 60, .max = 1800});
    if (rc != PARSE_OK) {
	return rc;
    }

    if ((3600 % value) != 0) {
	error_details = g_strdup("must be an integral divider of 3600 seconds");
	return PARSE_WRONG_PARAMETER;
    }

    minitest = value;

    return PARSE_OK;
}

static int cfg_unique_call_multi(const cfg_arg_t arg) {
    char *str = g_ascii_strup(parameter, -1);
    g_strstrip(str);

    if (strcmp(str, "ALL") == 0) {
	unique_call_multi = UNIQUECALL_ALL;
    } else if (strcmp(str, "BAND") == 0) {
	unique_call_multi = UNIQUECALL_BAND;
    } else {
	g_free(str);
	error_details = g_strdup("must be ALL or BAND");
	return PARSE_WRONG_PARAMETER;
    }

    g_free(str);
    return PARSE_OK;
}

static int cfg_digi_rig_mode(const cfg_arg_t arg) {
    char *str = g_ascii_strup(parameter, -1);
    g_strstrip(str);

    if (strcmp(str, "USB") == 0) {
	digi_mode = RIG_MODE_USB;
    } else if (strcmp(str, "LSB") == 0) {
	digi_mode = RIG_MODE_LSB;
    } else if (strcmp(str, "RTTY") == 0) {
	digi_mode = RIG_MODE_RTTY;
    } else if (strcmp(str, "RTTYR") == 0) {
	digi_mode = RIG_MODE_RTTYR;
    } else {
	g_free(str);
	error_details = g_strdup("must be USB, LSB, RTTY, or RTTYR");
	return PARSE_WRONG_PARAMETER;
    }

    g_free(str);
    return PARSE_OK;
}

static int cfg_cabrillo_field(const cfg_arg_t arg) {
    char *str = NULL;
    if (parameter != NULL) {
	str = g_strdup(parameter);
	g_strstrip(str);
    }
    gchar *name = g_match_info_fetch(match_info, 1);

    int rc = add_cabrillo_field(name, str);

    FREE_DYNAMIC_STRING(name);
    FREE_DYNAMIC_STRING(str);
    return rc;
}

static config_t logcfg_configs[] = {
    {"CONTEST_MODE",        CFG_BOOL_TRUE(iscontest)},
    {"MIXED",               CFG_BOOL_TRUE(mixedmode)},
    {"IGNOREDUPE",          CFG_BOOL_TRUE(ignoredupe)},
    {"USE_CONTINENTLIST_ONLY",  CFG_BOOL_TRUE(continentlist_only)},
    {"RADIO_CONTROL",           CFG_BOOL_TRUE(trx_control)},
    {"PORTABLE_MULT_2",     CFG_BOOL_TRUE(portable_x2)},

    {"USEPARTIALS",	    CFG_BOOL_TRUE(use_part)},
    {"PARTIALS",	    CFG_BOOL_TRUE(partials)},
    {"RECALL_MULTS",	    CFG_CONTEST_BOOL_TRUE(recall_mult)},
    {"WYSIWYG_MULTIBAND",   CFG_BOOL_TRUE(wysiwyg_multi)},
    {"WYSIWYG_ONCE",	    CFG_BOOL_TRUE(wysiwyg_once)},
    {"RIT_CLEAR",	    CFG_BOOL_TRUE(rit)},
    {"SHORT_SERIAL",	    CFG_INT_ONE(shortqsonr)},
    {"SCOREWINDOW",	    CFG_BOOL_TRUE(showscore_flag)},
    {"CHECKWINDOW",	    CFG_BOOL_TRUE(searchflg)},
    {"SEND_DE",		    CFG_BOOL_TRUE(demode)},
    {"SERIAL_EXCHANGE",	    CFG_CONTEST_BOOL_TRUE(exchange_serial)},
    {"COUNTRY_MULT",	    CFG_BOOL_TRUE(country_mult)},
    {"CQWW_M2",		    CFG_BOOL_TRUE(cqwwm2)},
    {"LAN_DEBUG",	    CFG_BOOL_TRUE(landebug)},
    {"CALLUPDATE",	    CFG_BOOL_TRUE(call_update)},
    {"TIME_MASTER",	    CFG_BOOL_TRUE(time_master)},
    {"CTCOMPATIBLE",	    CFG_BOOL_TRUE(ctcomp)},
    {"SERIAL\\+SECTION",    CFG_BOOL_TRUE(serial_section_mult)},
    {"SECTION_MULT",	    CFG_BOOL_TRUE(sectn_mult)},
    {"NOB4",		    CFG_BOOL_TRUE(nob4)},
    {"SHOW_TIME",	    CFG_BOOL_TRUE(show_time)},
    {"RXVT",		    CFG_BOOL_TRUE(use_rxvt)},
    {"WAZMULT",		    CFG_BOOL_TRUE(wazmult)},
    {"ITUMULT",		    CFG_BOOL_TRUE(itumult)},
    {"CONTINENT_EXCHANGE",  CFG_BOOL_TRUE(exc_cont)},
    {"NOAUTOCQ",	    CFG_BOOL_TRUE(noautocq)},
    {"NO_BANDSWITCH_ARROWKEYS", CFG_BOOL_TRUE(no_arrows)},
    {"SOUNDCARD",	    CFG_BOOL_TRUE(sc_sidetone)},
    {"LOWBAND_DOUBLE",	    CFG_BOOL_TRUE(lowband_point_mult)},
    {"CLUSTER_LOG",	    CFG_BOOL_TRUE(clusterlog)},
    {"SERIAL\\+GRID4",	    CFG_BOOL_TRUE(serial_grid4_mult)},
    {"LOGFREQUENCY",	    CFG_BOOL_TRUE(logfrequency)},
    {"NO_RST",		    CFG_BOOL_TRUE(no_rst)},
    {"SERIAL_OR_SECTION",   CFG_BOOL_TRUE(serial_or_section)},
    {"PFX_MULT",            CFG_BOOL_TRUE(pfxmult)},
    {"PFX_MULT_MULTIBAND",  CFG_BOOL_TRUE(pfxmultab)},
    {"QTCREC_RECORD",	    CFG_BOOL_TRUE(qtcrec_record)},
    {"QTC_AUTO_FILLTIME",   CFG_BOOL_TRUE(qtc_auto_filltime)},
    {"QTC_RECV_LAZY",	    CFG_BOOL_TRUE(qtc_recv_lazy)},
    {"BMAUTOGRAB",      CFG_BOOL_TRUE(bmautograb)},
    {"BMAUTOADD",       CFG_BOOL_TRUE(bmautoadd)},
    {"SPRINTMODE",      CFG_BOOL_TRUE(sprint_mode)},
    {"KEYER_BACKSPACE", CFG_BOOL_TRUE(keyer_backspace)},
    {"SECTION_MULT_ONCE",   CFG_BOOL_TRUE(sectn_mult_once)},

    {"F([1-9]|1[0-2])", CFG_MESSAGE(message, -1)},  // index is 1-based
    {"S&P_TU_MSG",      CFG_MESSAGE(message, SP_TU_MSG)},
    {"CQ_TU_MSG",       CFG_MESSAGE(message, CQ_TU_MSG)},
    {"ALT_([0-9])",     CFG_MESSAGE(message, CQ_TU_MSG + 1)},
    {"S&P_CALL_MSG",    CFG_MESSAGE(message, SP_CALL_MSG)},

    {"VKM([1-9]|1[0-2])",   CFG_MESSAGE(ph_message, -1)},
    {"VKCQM",               CFG_MESSAGE(ph_message, CQ_TU_MSG)},
    {"VKSPM",               CFG_MESSAGE(ph_message, SP_TU_MSG)},

    {"DKF([1-9]|1[0-2])",   CFG_MESSAGE_DYNAMIC(digi_message, -1)},
    {"DKCQM",               CFG_MESSAGE_DYNAMIC(digi_message, CQ_TU_MSG)},
    {"DKSPM",               CFG_MESSAGE_DYNAMIC(digi_message, SP_TU_MSG)},
    {"DKSPC",               CFG_MESSAGE_DYNAMIC(digi_message, SP_CALL_MSG)},
    {"ALT_DK([1-9]|10)",    CFG_MESSAGE_DYNAMIC(digi_message, CQ_TU_MSG)},

    {"QR_F([1-9]|1[0-2])",      CFG_MESSAGE(qtc_recv_msgs, -1) },
    {"QR_VKM([1-9]|1[0-2])",    CFG_MESSAGE(qtc_phrecv_message, -1) },
    {"QR_VKCQM",                CFG_MESSAGE(qtc_phrecv_message, CQ_TU_MSG) },
    {"QR_VKSPM",                CFG_MESSAGE(qtc_phrecv_message, SP_TU_MSG) },

    {"QS_F([1-9]|1[0-2])",      CFG_MESSAGE(qtc_send_msgs, -1) },
    {"QS_VKM([1-9]|1[0-2])",    CFG_MESSAGE(qtc_phsend_message, -1) },
    {"QS_VKCQM",                CFG_MESSAGE(qtc_phsend_message, CQ_TU_MSG) },
    {"QS_VKSPM",                CFG_MESSAGE(qtc_phsend_message, SP_TU_MSG) },

    {"TLFCOLOR([1-6])",  NEED_PARAM, cfg_tlfcolor},

    {"LAN_PORT",        CFG_INT(lan_port, 1000, INT32_MAX)},
    {"TIME_OFFSET",     CFG_INT(timeoffset, -23, 23)},
    {"NETKEYERPORT",    CFG_INT(netkeyer_port, 1, INT32_MAX)},
    {"TNCSPEED",        CFG_INT(tnc_serial_rate, 0, INT32_MAX)},
    {"RIGSPEED",        CFG_INT(serial_rate, 0, INT32_MAX)},
    {"CQDELAY",         CFG_INT(cqdelay, 3, 60)},
    {"SSBPOINTS",       CFG_INT(ssbpoints, 0, INT32_MAX)},
    {"CWPOINTS",        CFG_INT(cwpoints, 0, INT32_MAX)},
    {"WEIGHT",          CFG_INT(weight, -50, 50)},
    {"TXDELAY",         CFG_INT(txdelay, 0, 50)},
    {"TUNE_SECONDS",    CFG_INT(tune_seconds, 1, 100)},
    {"RIGMODEL",        CFG_INT(myrig_model, 0, 99999)},
    {"COUNTRY_LIST_POINTS", CFG_INT(countrylist_points, 0, INT32_MAX)},
    {"MY_COUNTRY_POINTS",   CFG_INT(my_country_points, 0, INT32_MAX)},
    {"MY_CONTINENT_POINTS", CFG_INT(my_cont_points, 0, INT32_MAX)},
    {"DX_POINTS",           CFG_INT(dx_cont_points, 0, INT32_MAX)},
    {"CWBANDWIDTH",         CFG_INT(cw_bandwidth, 0, INT32_MAX)},
    {"CONTINENT_LIST_POINTS",   CFG_INT(continentlist_points, 0, INT32_MAX)},

    {"NETKEYER",        CFG_INT_CONST(cwkeyer, NET_KEYER)},
    {"HAMLIB_KEYER",    CFG_INT_CONST(cwkeyer, HAMLIB_KEYER)},
    {"FIFO_INTERFACE",  CFG_INT_CONST(packetinterface, FIFO_INTERFACE)},
    {"LONG_SERIAL",     CFG_INT_CONST(shortqsonr, 0)},
    {"CLUSTER",         CFG_INT_CONST(cluster, CLUSTER)},
    {"SSBMODE",         CFG_INT_CONST(trxmode, SSBMODE)},
    {"RIGPTT",          CFG_INT_CONST(rigptt, CAT_PTT_WANTED)},

    {"RIGCONF",         CFG_STRING_STATIC(rigconf, 80)},
    {"LOGFILE",         CFG_STRING_STATIC(logfile, 120)},
    {"KEYER_DEVICE",    CFG_STRING_STATIC(keyer_device, 10)},
    {"NETKEYERHOST",    CFG_STRING_STATIC(netkeyer_hostaddress, 16)},
    {"TELNETHOST",      CFG_STRING_STATIC(pr_hostaddress, 48)},
    {"QTC_CAP_CALLS",   CFG_STRING_STATIC(qtc_cap_calls, 40)},
    {"SYNCFILE",        CFG_STRING_STATIC(synclogfile, 120)},
    {"SC_DEVICE",       CFG_STRING_STATIC(sc_device, 40)},
    {"INITIAL_EXCHANGE",       CFG_STRING_STATIC(exchange_list, 40)},
    {"DIGIMODEM",       CFG_STRING_STATIC(rttyoutput, 120)},
    {"FKEY-HEADER",     CFG_STRING_STATIC(fkey_header, sizeof(fkey_header))},

    {"CABRILLO",    CFG_STRING(cabrillo)},
    {"CALLMASTER",  CFG_STRING(callmaster_filename)},
    {"EDITOR",      CFG_STRING(editor_cmd)},

    {"RIGPORT",         CFG_STRING_NOCHOMP(rigportname)},
    {"CLUSTERLOGIN",    CFG_STRING_STATIC_NOCHOMP(clusterlogin, 80)},

    {"CALL",            NEED_PARAM, cfg_call},
    {"(CONTEST|RULES)", NEED_PARAM, cfg_contest},
    {"TELNETPORT",      NEED_PARAM, cfg_telnetport},
    {"BANDOUTPUT",      NEED_PARAM, cfg_bandoutput},
    {"(ONE_POINT|(TWO|THREE)_POINTS)",  NO_PARAM, cfg_n_points},
    {"BANDMAP",         OPTIONAL_PARAM, cfg_bandmap},
    {"CWSPEED",         NEED_PARAM, cfg_cwspeed},
    {"CWTONE",          NEED_PARAM, cfg_cwtone},
    {"SUNSPOTS",        NEED_PARAM, cfg_sunspots},
    {"SFI",             NEED_PARAM, cfg_sfi},
    {"TNCPORT",         NEED_PARAM, cfg_tncport},
    {"ADDNODE",         NEED_PARAM, cfg_addnode},
    {"THISNODE",        NEED_PARAM, cfg_thisnode},
    {"MULT_LIST",       NEED_PARAM, cfg_mult_list},
    {"MARKER(|DOT|CALL)S",  NEED_PARAM, cfg_markers},
    {"DX_&_SECTIONS",   NO_PARAM, cfg_dx_n_sections},
    {"COUNTRYLIST",     NEED_PARAM, cfg_countrylist},
    {"CONTINENTLIST",   NEED_PARAM, cfg_continentlist},
    {"USE_COUNTRYLIST_ONLY",    NO_PARAM, cfg_country_list_only},
    {"SIDETONE_VOLUME", NEED_PARAM, cfg_sc_volume},
    {"MFJ1278_KEYER",   NEED_PARAM, cfg_mfj1278_keyer},
    {"CHANGE_RST",      OPTIONAL_PARAM, cfg_change_rst},
    {"GMFSK",           NEED_PARAM, cfg_gmfsk},
    {"RTTYMODE",        NO_PARAM, cfg_rttymode},
    {"MYQRA",           NEED_PARAM, cfg_myqra},
    {"POWERMULT",       NEED_PARAM, cfg_powermult},
    {"QTC",             NEED_PARAM, cfg_qtc},
    {"BANDWEIGHT_POINTS",   NEED_PARAM, cfg_bandweight_points},
    {"BANDWEIGHT_MULTIS",   NEED_PARAM, cfg_bandweight_multis},
    {"PFX_NUM_MULTIS",      NEED_PARAM, cfg_pfx_num_multis},
    {"QTCREC_RECORD_COMMAND",   NEED_PARAM, cfg_qtcrec_record_command},
    {"EXCLUDE_MULTILIST",   NEED_PARAM, cfg_exclude_multilist},
    {"FLDIGI",              OPTIONAL_PARAM, cfg_fldigi},
    {"MINITEST",            OPTIONAL_PARAM, cfg_minitest},
    {"UNIQUE_CALL_MULTI",   NEED_PARAM, cfg_unique_call_multi},
    {"DIGI_RIG_MODE",       NEED_PARAM, cfg_digi_rig_mode},
    {"CABRILLO-(.+)",       OPTIONAL_PARAM, cfg_cabrillo_field},

    {NULL}  // end marker
};


static int check_match(const config_t *cfg, const char *keyword) {
    gchar *pattern = g_strdup_printf("^%s$", cfg->regex);
    GRegex *regex = g_regex_new(pattern, 0, 0, NULL);
    g_free(pattern);

    int result = PARSE_NO_MATCH;    // default: not found

    g_regex_match(regex, keyword, 0, &match_info);
    if (g_match_info_matches(match_info)) {

	if (cfg->param_kind == NEED_PARAM && parameter == NULL) {
	    result = PARSE_MISSING_PARAMETER;
	} else if (cfg->param_kind == NO_PARAM && parameter != NULL) {
	    result = PARSE_EXTRA_PARAMETER;
	} else {
	    result = cfg->func(cfg->arg);
	}
    }
    g_match_info_free(match_info);
    g_regex_unref(regex);

    return result;
}

static int apply_config(const char *keyword, const char *param,
			const config_t *configs) {

    parameter = param;      // save for matcher functions

    int result = PARSE_NO_MATCH;

    for (const config_t *cfg = configs; cfg->regex ; ++cfg) {
	result = check_match(cfg, keyword);
	if (result != PARSE_NO_MATCH) {
	    break;
	}
    }

    switch (result) {
	case PARSE_OK:
	    return PARSE_OK;

	case PARSE_NO_MATCH:
	    KeywordNotSupported(keyword);
	    break;

	case PARSE_MISSING_PARAMETER:
	    ParameterNeeded(keyword);
	    break;

	case PARSE_EXTRA_PARAMETER:
	    ParameterUnexpected(keyword);
	    break;

	case PARSE_INVALID_INTEGER:
	    WrongFormat_details(keyword, "invalid number");
	    break;

	case PARSE_INTEGER_OUT_OF_RANGE:
	    WrongFormat_details(keyword, "value out of range");
	    break;

	case PARSE_STRING_TOO_LONG:
	    WrongFormat_details(keyword, "value too long");
	    break;

	default:
	    if (error_details != NULL) {
		WrongFormat_details(keyword, error_details);
		g_free(error_details);
	    } else {
		WrongFormat(keyword);
	    }
    }

    return PARSE_ERROR;
}
////////////////////


int parse_logcfg(char *inputbuffer) {

    /* split the inputline at '=' to max 2 elements
     *
     * leave the components in fields[0] (keyword) and
     * fields[1] for the parameters
     *
     * if only 1 component (no '='), it is only a keyword
     *    g_strstrip it and test for keywordlist
     *
     * if 2 components (there is a '=' in the line)
     *    g_strstrip first component and test for keywordlist
     *    g_strchug second component -> strip leading space from parameters
     *
     * That allows plain keywords and also keywords with parameters (which
     * follows a '=' sign
     */

    char **fields = g_strsplit(inputbuffer, "=", 2);
    g_strstrip(fields[0]);

    if (*fields[0] == '\0') { 	/* only whitespace found? */
	g_strfreev(fields);
	return (PARSE_OK);
    }

    if (g_strv_length(fields) == 2) {   /* strip leading whitespace */
	g_strchug(fields[1]);		/* from parameters */
    }

    int result = apply_config(fields[0], fields[1], logcfg_configs);

    g_strfreev(fields);
    return result;
}


/** Complain about problems in configuration
 *
 * Complains in standout mode about some problem. Beep and wait for
 * 2 seconds
 *
 * \param msg  The reason for the problem to be shown
 */
void Complain(char *msg) {
    attron(A_STANDOUT);
    showmsg(msg);
    attroff(A_STANDOUT);
    beep();
}

/** Complain about not supported keyword */
void KeywordNotSupported(const char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer, "Keyword '%s' not supported. See man page.\n", keyword);
    Complain(msgbuffer);
}

/** Complain about missing parameter */
void ParameterNeeded(const char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Keyword '%s' must be followed by a parameter ('=....'). See man page.\n",
	    keyword);
    Complain(msgbuffer);
}

void ParameterUnexpected(const char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Keyword '%s' can't have a parameter. See man page.\n",
	    keyword);
    Complain(msgbuffer);
}

/** Complain about wrong parameter format */
void WrongFormat(const char *keyword) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Wrong parameter format for keyword '%s'. See man page.\n",
	    keyword);
    Complain(msgbuffer);
}

void WrongFormat_details(const char *keyword, const char *details) {
    char msgbuffer[192];
    sprintf(msgbuffer,
	    "Wrong parameter for keyword '%s': %s.\n",
	    keyword, details);
    Complain(msgbuffer);
}
