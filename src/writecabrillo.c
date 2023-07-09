/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012-2013           Thomas Beierlein <tb@forth-ev.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/* ------------------------------------------------------------
 *   write Cabrillo  file
 *
 *--------------------------------------------------------------*/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "globalvars.h"
#include "get_time.h"
#include "log_utils.h"
#include "tlf_curses.h"
#include "ui_utils.h"
#include "utils.h"
#include "cabrillo_utils.h"
#include "sendbuf.h"
#include "bands.h"

char exchange[40];      // format of sent exchange

struct linedata_t *get_next_record(FILE *fp);
struct linedata_t *get_next_qtc_record(FILE *fp, int qtcdirection);
void free_linedata(struct linedata_t *ptr);


struct linedata_t *parse_logline(char *buffer) {
    struct linedata_t *ptr;

    ptr = g_malloc0(sizeof(struct linedata_t));

    /* remember whole line */
    ptr->logline = g_strdup(buffer);
    ptr->qtcdirection = 0;
    ptr->qsots = 0;


    struct qso_t *qso = parse_qso(buffer);

    ptr-> band = qso->band;
    ptr-> mode = qso->mode;
    ptr-> day  = qso->day;
    ptr-> month = qso->month;
    ptr-> year = qso->year;
    ptr-> hour = qso->hour;
    ptr-> min = qso->min;
    ptr-> qso_nr = qso->qso_nr;
    ptr-> call = qso->call;
    ptr-> rst_s = qso->rst_s;
    ptr-> rst_r = qso->rst_r;
    ptr-> comment = qso->comment;
    ptr-> freq = qso->freq;
    ptr-> tx = qso->tx;

    g_strchomp(ptr->comment);   // remove trailing spaces

    g_free(qso);	/* free qso_t struct but not the
			   allocated string copies */
    return ptr;
}

/** get next qso record from log
 *
 * Read next line from logfile until it is no comment.
 * Then parse the logline into a new allocated QSO data structure
 * and return that structure.
 *
 * \return ptr to new qso record (or NULL if eof)
 */
struct linedata_t *get_next_record(FILE *fp) {
    char* buffer;
    size_t buffer_len = 160;
    struct linedata_t *ptr;
    int read;

    buffer = (char*)calloc(buffer_len, sizeof(char));
    while ((read = getline(&buffer, &buffer_len, fp)) != -1) {
        if (!log_is_comment(buffer)) {
            ptr = parse_logline(buffer);
            return ptr;
        }
    }

    free(buffer);
    return NULL;
}

/** get next qtc record from log
 *
 * Read next line from logfile.
 * Then parse the received or sent qtc logline into a new allocated received QTC data structure
 * and return that structure.
 *
 * \return ptr to new qtc record (or NULL if eof)
 */
struct linedata_t *get_next_qtc_record(FILE *fp, int qtcdirection) {
    char* buffer;
    size_t buffer_len = 100;
    char *tmp;
    char *sp;
    struct linedata_t *ptr;
    int pos, shift, read;
    struct tm date_n_time;

    if (fp == NULL) {
	    return NULL;
    }

    buffer = (char*)calloc(buffer_len, sizeof(char));
    if ((read = getline(&buffer, &buffer_len, fp)) == -1) {
	    return NULL;
    }

    ptr = g_malloc0(sizeof(struct linedata_t));

    /* remember whole line */
    ptr->logline = g_strdup(buffer);
    ptr->qtcdirection = qtcdirection;

    /* tx */
    if (qtcdirection == RECV) {
        pos = 28;
        shift = 0;
    } else {
        pos = 33;
        shift = 5;
    }
    ptr->tx = (buffer[pos] == ' ') ? 0 : 1;

    /* split buffer into parts for linedata_t record and parse
      * them accordingly */
    tmp = strtok_r(buffer, " \t", &sp);

    /* band */
    ptr->band = atoi(tmp);

    /* mode */
    if (strcasestr(tmp, "CW"))
	    ptr->mode = CWMODE;
    else if (strcasestr(tmp, "SSB"))
	    ptr->mode = SSBMODE;
    else
	    ptr->mode = DIGIMODE;

    /* qso number */
    ptr->qso_nr = atoi(strtok_r(NULL, " \t", &sp));

    /* in case of SEND direction, the 3rd field is the original number of sent QSO,
       but it doesn't need for QTC line */
    if (qtcdirection & SEND) {
	    tmp = strtok_r(NULL, " \t", &sp);
    }
    /* date & time */
    memset(&date_n_time, 0, sizeof(struct tm));

    strptime(strtok_r(NULL, " \t", &sp), DATE_FORMAT, &date_n_time);
    strptime(strtok_r(NULL, " \t", &sp), TIME_FORMAT, &date_n_time);

    ptr->qsots = timegm(&date_n_time);
    ptr->year = date_n_time.tm_year + 1900;	/* convert to
							1968..2067 */
    ptr->month = date_n_time.tm_mon + 1;	/* tm_mon = 0..11 */
    ptr->day   = date_n_time.tm_mday;

    ptr->hour  = date_n_time.tm_hour;
    ptr->min   = date_n_time.tm_min;

    if (ptr->tx == 1) {
        /* ignore TX if set */
        strtok_r(NULL, " \t", &sp);
    }
    /* his call */
    ptr->call = g_strdup(strtok_r(NULL, " \t", &sp));

    /* QTC serial and number */
    ptr->qtc_serial = atoi(strtok_r(NULL, " \t", &sp));
    ptr->qtc_number = atoi(strtok_r(NULL, " \t", &sp));

    ptr->qtc_qtime = g_strdup(strtok_r(NULL, " \t", &sp));
    ptr->qtc_qcall = g_strdup(strtok_r(NULL, " \t", &sp));
    ptr->qtc_qserial = g_strdup(strtok_r(NULL, " \t", &sp));

    /* frequency */
    ptr->freq = atof(buffer + 80 + shift) * 1000.0;
    if (freq2bandindex(ptr->freq) == BANDINDEX_OOB) {
	    ptr->freq = 0.;
    }

    free(buffer);
    return ptr;
}

/** free  linedata record pointed to by ptr */
void free_linedata(struct linedata_t *ptr) {

    if (ptr != NULL) {
	g_free(ptr->comment);
	g_free(ptr->logline);
	g_free(ptr->call);
	if (ptr->qtc_qtime != NULL) {
	    g_free(ptr->qtc_qtime);
	    g_free(ptr->qtc_qcall);
	    g_free(ptr->qtc_qserial);
	}
	g_free(ptr);
    }
}

/** write out information */
void info(char *s) {
    attron(modify_attr(COLOR_PAIR(C_INPUT) | A_STANDOUT));
    mvaddstr(13, 29, s);
    refreshp();
}


const char *to_mode[] = {
    "CW",
    "PH",
    "RY"
};

/* add 'src' to 'dst' with max. 'len' chars left padded */
void add_lpadded(char *dst, char *src, int len) {

    int l = strlen(src);
    if (l > len) {
	sprintf(dst, "!ERROR: field too wide (%s)", src);  // overwrites buffer
	return;
    }

    strcat(dst, " ");   // add delimiter

    char *field = g_malloc(len + 1);
    memset(field, ' ', len);
    memcpy(field + len - l, src, l);
    field[len] = '\0';
    strcat(dst, field);
    g_free(field);
}

/* add 'src' to 'dst' with max. 'len' char right padded */
void add_rpadded(char *dst, char *src, int len) {

    int l = strlen(src);
    if (l > len) {
	sprintf(dst, "!ERROR: field too wide (%s)", src);  // overwrites buffer
	return;
    }

    strcat(dst, " ");   // add delimiter

    char *field = g_malloc(len + 1);
    memset(field, ' ', len);
    memcpy(field, src, l);
    field[len] = '\0';
    strcat(dst, field); // add field
    g_free(field);
}

/* get the n-th token of a string, return empty string if no n-th token */
/* default separator is whitespace (space or tab) */
gchar *get_nth_token(gchar *str, int n, const char *separator) {
    gchar *string = g_strdup(str);
    const char *delim = (separator != NULL ? separator : " \t");
    gchar *ptr;
    char *sp;

    ptr = strtok_r(string, delim, &sp);

    while (n > 0 && ptr != NULL) {
	ptr = strtok_r(NULL, delim, &sp);
	n--;
    }

    /* if no n-th element in string, return empty string */
    if (ptr == NULL)
	ptr = strdup("");
    else
	ptr = strdup(ptr);

    g_free(string);
    return ptr;
}

static gchar *get_sent_exchage(int qso_nr) {
    char number[6];
    sprintf(number, "%04d", qso_nr);
    gchar *result = g_strndup(exchange, 80);
    replace_n(result, 80, "#", number, 99);
    return result;
}

/* format QSO: or QTC: line according to Cabrillo format description
 * and put it into buffer */
void prepare_line(struct linedata_t *qso, struct cabrillo_desc *desc,
		  char *buf) {

    freq_t freq;
    int i;
    char tmp[80];
    struct line_item *item;
    gchar *token;
    int item_count;
    GPtrArray *item_array;

    if (qso == NULL) {
	strcpy(buf, "");
	return;
    }

    freq = qso->freq;
    if (freq == 0)
	freq = (freq_t) band2freq(qso->band);

    if (qso->qtcdirection == 0) {
	strcpy(buf, "QSO:");		/* start the line */
	item_count = desc->item_count;
	item_array = desc->item_array;
    } else {
	if (desc->qtc_item_array == NULL) {
	    strcpy(buf, "");		/* no QTC format description */
	    return;
	}
	strcpy(buf, "QTC:");		/* start the line */
	item_count = desc->qtc_item_count;
	item_array = desc->qtc_item_array;
    }

    for (i = 0; i < item_count; i++) {
	item = g_ptr_array_index(item_array, i);
	switch (item->tag) {
	    case FREQ:
		sprintf(tmp, "%d", (int)(freq / 1000.0));
		add_lpadded(buf, tmp, item->len);
		break;
	    case MODE:
		sprintf(tmp, "%s", to_mode[qso->mode]);
		add_lpadded(buf, tmp, item->len);
		break;
	    case DATE:
		sprintf(tmp, "%4d-%02d-%02d",
			qso->year, qso->month, qso->day);
		add_lpadded(buf, tmp, item->len);
		break;
	    case TIME:
		sprintf(tmp, "%02d%02d", qso->hour, qso->min);
		add_lpadded(buf, tmp, item->len);
		break;
	    case MYCALL:
		add_rpadded(buf, my.call, item->len);
		break;
	    case HISCALL:
		add_rpadded(buf, qso->call, item->len);
		break;
	    case RST_S:
		sprintf(tmp, "%d", qso->rst_s);
		add_rpadded(buf, tmp, item->len);
		break;
	    case RST_R:
		sprintf(tmp, "%d", qso->rst_r);
		add_rpadded(buf, tmp, item->len);
		break;
	    case EXCH:
		add_rpadded(buf, qso->comment, item->len);
		break;
	    case EXC1:
		token = get_nth_token(qso->comment, 0, desc->exchange_separator);
		add_rpadded(buf, token, item->len);
		g_free(token);
		break;
	    case EXC2:
		token = get_nth_token(qso->comment, 1, desc->exchange_separator);
		add_rpadded(buf, token, item->len);
		g_free(token);
		break;
	    case EXC3:
		token = get_nth_token(qso->comment, 2, desc->exchange_separator);
		add_rpadded(buf, token, item->len);
		g_free(token);
		break;
	    case EXC4:
		token = get_nth_token(qso->comment, 3, desc->exchange_separator);
		add_rpadded(buf, token, item->len);
		g_free(token);
		break;
	    case EXC_S:
		token = get_sent_exchage(qso->qso_nr);
		add_rpadded(buf, token, item->len);
		g_free(token);
		break;
	    case TX:
		sprintf(tmp, "%1d", qso->tx);
		add_rpadded(buf, tmp, item->len);
		break;
	    case QTCRCALL:
		if (qso->qtcdirection == 1) {	// RECV
		    strcpy(tmp, my.call);
		}
		if (qso->qtcdirection == 2) {	// SEND
		    strcpy(tmp, qso->call);
		}
		add_rpadded(buf, g_strchomp(tmp), item->len);
		break;
	    case QTCHEAD:
		tmp[0] = '\0';
		sprintf(tmp, "%*d/%d", 3, qso->qtc_serial, qso->qtc_number);
		add_rpadded(buf, g_strchomp(tmp), item->len);
		break;
	    case QTCSCALL:
		if (qso->qtcdirection == 1) {	// RECV
		    strcpy(tmp, qso->call);
		}
		if (qso->qtcdirection == 2) {	// SEND
		    strcpy(tmp, my.call);
		}
		add_rpadded(buf, g_strchomp(tmp), item->len);
		break;
	    case QTC:
		sprintf(tmp, "%s %-13s %4s", qso->qtc_qtime, qso->qtc_qcall, qso->qtc_qserial);
		add_rpadded(buf, g_strchomp(tmp), item->len);
		break;
	    case NO_ITEM:
	    default:
		; // no action
	}
	if (buf[0] == '!') {
	    break;      // there was an error
	}
    }
    strcat(buf, "\n"); 		/* closing nl */
}

static void set_exchange_format() {
    if (contest->exchange_serial) {
	strcpy(exchange, "#");  // contest is using serial number
	return;
    }
    get_cabrillo_field_value(find_cabrillo_field(CBR_EXCHANGE), exchange,
			     sizeof(exchange));
}

//
// process QSO/QTC data according to cabdesc
//  - fills buffer and writes it into fp2
//  - qso data is freed
// returns true on success
//         false on error, with buffer containing
//               the error text starting with an exclamation mark
//
static bool process_record(struct linedata_t *qso,
			   struct cabrillo_desc *cabdesc,
			   char *buffer, FILE *fp2) {

    prepare_line(qso, cabdesc, buffer);
    free_linedata(qso);

    if (strlen(buffer) > 5) {
	fputs(buffer, fp2);
    }

    bool ok = true;
    if (buffer[0] == '!') {
	g_strchomp(buffer);
	info(buffer + 1);   // show error message
	sleep(2);
	ok = false;
    }

    return ok;
}

int write_cabrillo(void) {

    struct cabrillo_desc *cabdesc;
    char cabrillo_file_name[80];
    char buffer[4000] = "";

    FILE *fp1, *fp2, *fpqtcrec = NULL, *fpqtcsent = NULL;
    struct linedata_t *qso, *qtcrec = NULL, *qtcsent = NULL;
    int qsonr, qtcrecnr, qtcsentnr;

    if (cabrillo == NULL) {
	info("Missing CABRILLO= keyword (see man page)");
	sleep(2);
	return 1;
    }

    char *cab_file = find_available("cabrillo.fmt");

    cabdesc = read_cabrillo_format(cab_file, cabrillo);

    g_free(cab_file);

    if (!cabdesc) {
	info("Cabrillo format specification not found!");
	sleep(2);
	return 2;
    }

    /* open logfile and create a Cabrillo file */
    if ((fp1 = fopen(logfile, "r")) == NULL) {
	info("Can't open logfile.");
	sleep(2);
	free_cabfmt(cabdesc);
	return 1;
    }
    if (cabdesc->qtc_item_array != NULL) {
	if (qtcdirection & 1) {
	    fpqtcrec = fopen(QTC_RECV_LOG, "r");
	    if (fpqtcrec == NULL) {
		info("Can't open received QTC logfile.");
		sleep(2);
		free_cabfmt(cabdesc);
		fclose(fp1);
		return 1;
	    }
	}
	if (qtcdirection & 2) {
	    fpqtcsent = fopen(QTC_SENT_LOG, "r");
	    if (fpqtcsent == NULL) {
		info("Can't open sent QTC logfile.");
		sleep(2);
		free_cabfmt(cabdesc);
		fclose(fp1);
		if (fpqtcrec != NULL) fclose(fpqtcrec);
		return 1;
	    }
	}
    }

    get_cabrillo_file_name(cabrillo_file_name);
    if ((fp2 = fopen(cabrillo_file_name, "w")) == NULL) {
	info("Can't create Cabrillo file.");
	sleep(2);
	free_cabfmt(cabdesc);
	fclose(fp1);
	if (fpqtcsent != NULL) fclose(fpqtcsent);
	if (fpqtcrec != NULL) fclose(fpqtcrec);
	return 2;
    }


    /* exchange and header information */
    set_exchange_format();

    write_cabrillo_header(fp2);

    time_t start_time = get_time();
    info("Writing Cabrillo file");

    qsonr = 0;
    qtcrecnr = 0;
    qtcsentnr = 0;
    bool ok = true;
    while (ok && (qso = get_next_record(fp1))) {

	qsonr++;
	ok = process_record(qso, cabdesc, buffer, fp2);
	if (!ok) {
	    break;      // stop processing immediately
	}

	if (fpqtcrec != NULL && qtcrec == NULL) {
	    qtcrec = get_next_qtc_record(fpqtcrec, RECV);
	    if (qtcrec != NULL) {
		qtcrecnr = qtcrec->qso_nr;
	    }
	}
	if (fpqtcsent != NULL && qtcsent == NULL) {
	    qtcsent = get_next_qtc_record(fpqtcsent, SEND);
	    if (qtcsent != NULL) {
		qtcsentnr = qtcsent->qso_nr;
	    }
	}
	while (qtcrecnr == qsonr || qtcsentnr == qsonr) {
	    if (qtcsent == NULL || (qtcrec != NULL && qtcrec->qsots < qtcsent->qsots)) {
		ok = process_record(qtcrec, cabdesc, buffer, fp2);

		qtcrec = get_next_qtc_record(fpqtcrec, RECV);
		if (qtcrec != NULL) {
		    qtcrecnr = qtcrec->qso_nr;
		} else {
		    qtcrecnr = 0;
		}
	    } else {
		ok = process_record(qtcsent, cabdesc, buffer, fp2);

		qtcsent = get_next_qtc_record(fpqtcsent, SEND);
		if (qtcsent != NULL) {
		    qtcsentnr = qtcsent->qso_nr;
		} else {
		    qtcsentnr = 0;
		}
	    }

	    if (!ok) {
		break;      // stop QTC processing, will exit main loop later
	    }
	}

    }

    fclose(fp1);

    fputs("END-OF-LOG:\n", fp2);
    fclose(fp2);
    if (fpqtcrec != NULL) {
	fclose(fpqtcrec);
    }

    free_cabfmt(cabdesc);

    if (get_time() == start_time) {
	sleep(1);
    }

    return 0;
}


/* Writing Log as ADIF file */


void add_adif_field(char *adif_line, char *field, char *value) {
    char *tmp;

    if (strlen(field) == 0)
	return;

    if (value == NULL) {
	tmp = g_strdup_printf("<%s>", field);
    } else {
	tmp = g_strdup_printf("<%s:%zd>%s",
			      field, strlen(value), value);
    }
    strcat(adif_line, tmp);
    g_free(tmp);
}

void add_adif_field_formated(char *buffer, char *field, char *fmt, ...) {
    va_list args;
    char *value;

    va_start(args, fmt);
    value = g_strdup_vprintf(fmt, args);
    va_end(args);

    add_adif_field(buffer, field, value);
    g_free(value);
}

/* write ADIF header to open file */
void write_adif_header(FILE *fp) {

    char timebuf[100];

    fputs
    ("################################################################################\n",
     fp);
    fputs("#                     ADIF v3.10 data file exported by TLF\n", fp);
    fputs("#              according to specifications on http://www.adif.org\n",
	  fp);
    fputs
    ("################################################################################\n",
     fp);

    format_time(timebuf, sizeof(timebuf), CREATED_DATE_TIME_FORMAT);
    fprintf(fp, "Created %s for %s\n\n", timebuf, my.call);

    /* Write contest name */
    fprintf(fp, "Contest Name: %s\n", whichcontest);
    fputs("<adif_ver:4>3.10\n", fp);
    fputs("<programid:3>TLF\n", fp);
    fprintf(fp, "<programversion:%zu>%s\n", strlen(VERSION), VERSION);
    fputs("<eoh>\n", fp);
}

/* format QSO line from buf according to ADIF format description
 * and put it into buffer */
void prepare_adif_line(char *buffer, struct linedata_t *qso) {

    char *tmp;

    strcpy(buffer, "");

    /* CALLSIGN */
    add_adif_field(buffer, "CALL", qso->call);

    /* BAND */
    add_adif_field_formated(buffer, "BAND", "%dM", qso->band);

    /* FREQ if available */
    if (qso->freq > 0) {
	// write MHz
	add_adif_field_formated(buffer, "FREQ", "%.4f",
				qso->freq / 1000000.0);
    }

    /* QSO MODE */
    if (qso->mode == CWMODE)
	tmp = "CW";
    else if (qso->mode == SSBMODE)
	tmp = "SSB";
    else if (strcmp(modem_mode, "RTTY") == 0)
	tmp = "RTTY";
    else
	/* \todo DIGI is no allowed mode */
	tmp = "DIGI";
    add_adif_field(buffer, "MODE", tmp);

    /* QSO_DATE */
    add_adif_field_formated(buffer, "QSO_DATE", "%4d%02d%02d",
			    qso->year, qso->month, qso->day);

    /* TIME_ON */
    add_adif_field_formated(buffer, "TIME_ON", "%02d%02d",
			    qso->hour, qso->min);

    /* RST_SENT */
    if (!no_rst) {
	add_adif_field_formated(buffer, "RST_SENT", "%d", qso->rst_s);
    }

    /* Sent contest serial number or exchange */
    bool serial_only = (strcmp(exchange, "#") == 0);
    tmp = get_sent_exchage(qso->qso_nr);
    g_strstrip(tmp);
    add_adif_field(buffer, (serial_only ? "STX" : "STX_STRING"), tmp);
    g_free(tmp);

    /* RST_RCVD */
    if (!no_rst) {
	add_adif_field_formated(buffer, "RST_RCVD", "%d", qso->rst_r);
    }

    /* Received contest serial number or exchange */
    tmp = g_strdup(qso->comment);
    g_strstrip(tmp);
    add_adif_field(buffer, (serial_only ? "SRX" : "SRX_STRING"), tmp);
    g_free(tmp);

    /* <EOR> - end of ADIF row */
    strcat(buffer, "<eor>\n");
}

/*
    The ADIF function has been written according ADIF v3.10 specifications
    as shown on http://www.adif.org
*/
int write_adif(void) {

    struct linedata_t *qso;
    char buffer[181] = "";
    char adif_tmp_name[40] = "";

    FILE *fp1, *fp2;

    if ((fp1 = fopen(logfile, "r")) == NULL) {
	info("Opening logfile not possible.");
	sleep(2);
	return 1;
    }
    strcpy(adif_tmp_name, whichcontest);
    strcat(adif_tmp_name, ".adi");

    if ((fp2 = fopen(adif_tmp_name, "w")) == NULL) {
	info("Opening ADIF file not possible.");
	sleep(2);
	fclose(fp1);		//added by F8CFE
	return 2;
    }


    /* in case using write_adif() without write_cabrillo() before
     * just get the needed information */
    set_exchange_format();

    time_t start_time = get_time();
    info("Writing ADIF file");

    write_adif_header(fp2);

    while ((qso = get_next_record(fp1))) {

	prepare_adif_line(buffer, qso);
	fputs(buffer, fp2);

	free_linedata(qso);
    }

    fclose(fp1);
    fclose(fp2);

    if (get_time() == start_time) {
	sleep(1);
    }

    return 0;
}				// end write_adif
