/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2012           Thomas Beierlein <tb@forth-ev.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
	/* ------------------------------------------------------------
	 *   write cabrillo  file
	 *
	 *--------------------------------------------------------------*/
#define _XOPEN_SOURCE 500
#include "writecabrillo.h"
#include <curses.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>


/* list of different tags for QSO: line items */
enum tag_t { NO_ITEM, FREQ, MODE, DATE, TIME, MYCALL, HISCALL, RST_S, RST_R, 
    EXC_S, EXCH, TX };	


/* conversion table between tag name in format file and internal tag */
struct tag_conv {
    char 	*item_name;
    enum tag_t  tag;
} tag_tbl[] = {
    { "FREQ",	FREQ 	},
    { "MODE",	MODE 	},
    { "DATE",	DATE 	},
    { "TIME", 	TIME 	},
    { "MYCALL", MYCALL 	},
    { "HISCALL",HISCALL },
    { "RST_S",  RST_S 	},
    { "RST_R",  RST_R 	},
    { "EXC_S",  EXC_S 	},
    { "EXCH",   EXCH 	},
    { "TX",     TX 	}
};


/* describes one item for printing the QSO: line in cabrillo */
struct line_item {
    enum tag_t tag;	/* item type */
    int len;		/* max. item length */
};

/* describes the cabrillo format to be used */
struct cabrillo_desc {
    char *name;			/* name of the cabrillo format in use */
    int item_count;		/* number items in QSO: line */
    GPtrArray *item_array;	/* array of items in QSO: line
    				 * must be from left to right */
};


/* represents different parts of a qso logline */
struct qso_t {
    char *logline;
    int band;
    int mode;
    char day;
    char month;
    int year;
    int hour;
    int min;
    int qso_nr;
    char *call;
    int rst_s;
    int rst_r;
    char *comment;
    float freq;
};

int is_comment(char *buffer);
struct qso_t *get_next_record (FILE *fp);
void free_qso(struct qso_t *ptr);


/** check if logline is only a comment */
int is_comment(char *buf) {

   if (buf[0] != ';' && strlen(buf) > 60) /** \todo better check */
	return 0;
   else
	return 1;
}


/** get next qso record from log
 *
 * Read next line from logfile until it is no comment.
 * Then parse the logline into a new allocated QSO data structure
 * and return that structure.
 *
 * \return ptr to new qso record (or NULL if eof)
 */
struct qso_t *get_next_record (FILE *fp)
{
    char buffer[160];
    char *tmp;
    struct qso_t *ptr;
    struct tm date_n_time;

    while ((fgets(buffer, sizeof(buffer), fp)) != NULL) {

	if (!is_comment(buffer)) {
		
	    ptr = g_malloc0 (sizeof(struct qso_t));

	    /* remember whole line */
	    ptr->logline = g_strdup( buffer );

	    /* split buffer into parts for qso_t record and parse
	     * them accordingly */
	    tmp = strtok( buffer, " \t");

	    /* band */
	    ptr->band = atoi( tmp );


	    /* mode */
	    if ( tmp[3] == 'C')
		ptr->mode = CWMODE;
	    else if (tmp[3] == 'S')
		ptr->mode = SSBMODE;
	    else
		ptr->mode = DIGIMODE;

	    /* date & time */
	    memset( &date_n_time, 0, sizeof(struct tm) );

	    strptime ( strtok( NULL, " \t" ), "%d-%b-%y", &date_n_time);
	    strptime ( strtok( NULL, " \t" ), "%H:%M", &date_n_time);

	    ptr->year = date_n_time.tm_year + 1900;	/* convert to
							   1968..2067 */
	    ptr->month = date_n_time.tm_mon + 1;	/* tm_mon = 0..11 */
	    ptr->day   = date_n_time.tm_mday;

	    ptr->hour  = date_n_time.tm_hour;
	    ptr->min   = date_n_time.tm_min;

	    /* qso number */
	    ptr->qso_nr = atoi( strtok( NULL, " \t" ) );

	    /* his call */
	    ptr->call = g_strdup( strtok( NULL, " \t" ) );

	    /* RST send and received */
	    ptr->rst_s = atoi( strtok( NULL, " \t" ) );
	    ptr->rst_r = atoi( strtok( NULL, " \t" ) );

	    /* comment (exchange) */
	    ptr->comment = g_strndup( buffer + 54, 13 );

	    /* frequency */
	    ptr->freq = atof( buffer + 80 );
	    if ( ( ptr->freq < 1800. ) || ( ptr->freq >= 30000. ) ) {
		ptr->freq = 0.;
	    }

	    return ptr;
	}
    }

    return NULL;
}


/** free qso record pointed to by ptr */
void free_qso(struct qso_t *ptr) {

    g_free( ptr->comment );
    g_free( ptr->logline );
    g_free( ptr->call );
    g_free( ptr );

}


void errorbox(char *s)
{
    printw("%s\nPress any key\n", s);
    refreshp();
    echo();
    sleep(4);
    noecho();
}


#define new
#ifdef new
/** free cabrillo format description */
void free_cabfmt(struct cabrillo_desc *desc) {
    int i;

    for ( i = 0; i < desc->item_array->len; i++ ) {
	g_free( g_ptr_array_index(desc->item_array, i) );
    }

    g_ptr_array_free( desc->item_array, TRUE );

    g_free( desc->name );
    g_free( desc );
}


/* translate item name into a tag */
enum tag_t translate_item_name( char *name ) {
    int i;

    /* lookup name in tag list */
    for ( i = 0; i < sizeof(tag_tbl)/sizeof(struct tag_conv); i++) {
    	if (strcmp( tag_tbl[i].item_name, name ) == 0 ) {
	    /* and return corresponding tab */
	    return tag_tbl[i].tag;
	}
    }

    /* if not found return NO_ITEM tag */
    return NO_ITEM;
}


/* parse item describing one entry 
 *
 * has to be in following format: item,length
 *   - item to print (date, time, call, ...)
 *   - max length
 */
struct line_item *parse_line_entry(char *line_entry) {
    struct line_item *item;
    gchar **parts;
    enum tag_t tag;

    item = g_malloc( sizeof(struct line_item) );
    parts = g_strsplit(line_entry, ",", 2);

    if ( g_strv_length(parts) == 2 ) {
	tag = translate_item_name( parts[0] );
	item->tag = tag;
	item->len = atoi( parts[1] );
    } 
    else {
	/* type is NO_ITEM */
	item->tag = NO_ITEM;
    }

    g_strfreev( parts );

    return item;
}


/** read cabrillo format description
 *
 * Try to read cabrillo format description for given format from
 * file and return a describing structure.
 *
 * \param filename	File to read description from
 * \param format	Name of the format to read
 * \return 		Pointer to a structure describing the format
 * 			(NULL if file or format not found or not readable)
 */
struct cabrillo_desc *read_cabrillo_format (char *filename, char *format) 
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gchar **list;
    gsize nrstrings;
    struct cabrillo_desc *cabdesc;
    int i;
    
    keyfile = g_key_file_new();

    if (!g_key_file_load_from_file( keyfile, filename, 
		G_KEY_FILE_NONE, &error)) {
	printw( "%s\n", error->message );
	refreshp();
	g_error_free( error );

	/* file does not exist or is wrongly formatted */
	g_key_file_free(keyfile);
	return NULL;
    }

    /* check if 'format' defined in file */
    if( g_key_file_has_group( keyfile, format ) == FALSE ) {

	/* group not found */
	g_key_file_free( keyfile );
	return NULL;
    }

    /* read needed keys */
    list = g_key_file_get_string_list( keyfile, format, 
			"QSO", &nrstrings, &error );

    if ( error && error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {

	/* if not found -> stop processing as that key is mandatory */
	g_error_free( error );
	g_key_file_free( keyfile );
	return NULL;
    }

    /* construct new format descriptor and fill it in */
    cabdesc = g_new( struct cabrillo_desc, 1 );
    cabdesc->name = g_strdup(format);
    cabdesc->item_array = g_ptr_array_new();
    cabdesc->item_count = nrstrings;

    for (i = 0; i < nrstrings; i++) {
	struct line_item *item;

	item = parse_line_entry( list[i] );
	if ( item ) {
	    /* add only well formatted entries */
	    g_ptr_array_add( cabdesc->item_array, item );
	}
    }

    if ( cabdesc->item_array->len != nrstrings ) {
	/* not all entries are ok -> stop processing */
	free_cabfmt( cabdesc );
	g_strfreev( list );
	g_key_file_free( keyfile );
	return NULL;
    }

    g_strfreev( list );

    /* possible further entries in format specification may contain information 
     * about allowed items for different categories:
     * CONTEST, CATEGORY-OPERATOR, CATEGORY_TRANSMITTER, CATEGORY-POWER,
     * CATEGORY-ASSISTED, CATEGORY-BAND, CATEGORY-MODE, C-STATION, C-TIME. 
     * C-OVERLAY
     */

    g_key_file_free( keyfile );

    /* return parsed cabrillo format description */
    return cabdesc;
}


int write_cabrillo(void)
{
    extern char* cabrillo;
    extern char logfile[];
    extern char exchange[];
    extern char call[];

    int rc;
    char* cab_dfltfile;
    struct cabrillo_desc *cabdesc;
    char standardexchange[70] = "";
    char cabrillo_tmp_name[80];
    char cmd[80];
    char buffer[4000] = "";

    FILE *fp1, *fp2;
    struct qso_t *qso;

    if (cabrillo == NULL) {
	errorbox("No cabrillo format defined (See doc/README.cabrillo)");
    	exit(1);
    }

    /* try to read cabrillo format first from local directory than from 
     * default data dir
     */
    cabdesc = read_cabrillo_format("cabrillo.fmt", cabrillo);
    if (!cabdesc) {
	cab_dfltfile = g_strconcat(PACKAGE_DATA_DIR, G_DIR_SEPARATOR_S,
	    "cabrillo.fmt", NULL);
	cabdesc = read_cabrillo_format(cab_dfltfile, cabrillo);
	g_free(cab_dfltfile);
    }

    if (!cabdesc) {
    	errorbox("Cabrillo format specification not found!");
	exit(1);
    }

    /* open logfile and create a cabrillo file */
    strcpy(cabrillo_tmp_name, call);
    cabrillo_tmp_name[strlen(call)-1] = '\0'; /* drop \n */
    strcat(cabrillo_tmp_name, ".cbr");

    if (strlen(exchange) > 0)
	strcpy(standardexchange, exchange);

    if ((fp1 = fopen(logfile, "r")) == NULL) {
	fprintf(stdout, "Opening logfile not possible.\n");
	free_cabfmt( cabdesc );
	return (1);
    }
    if ((fp2 = fopen("./cabrillo", "w")) == NULL) {
	fprintf(stdout, "Opening cbr file not possible.\n");
	free_cabfmt( cabdesc );
	fclose(fp1);		//added by F8CFE
	return (2);
    }

//    write_header();

    while ((qso = get_next_record(fp1))) {

//	prepare_line(qso, buffer);

	if (strlen(buffer) > 11)
	    fputs(buffer, fp2);

	free_qso(qso);
    }

    fclose(fp1);

    fputs("END-OF-LOG:\n", fp2);
    fclose(fp2);

    rc = system("cat cabrillo >> header");
    sprintf(cmd, "cp header %s", cabrillo_tmp_name);
    rc = system(cmd);
    rc = system("mv header summary.txt");

    free_cabfmt( cabdesc );

    return 0;
}

#else

int write_cabrillo(void)
{
    extern char backgrnd_str[];
    extern char logfile[];
    extern char call[];
    extern int cqww;
    extern int arrldx_usa;
    extern int other_flg;
    extern int wysiwyg_multi;
    extern int wysiwyg_once;
    extern int serial_grid4_mult;
    extern char mycqzone[];
    extern char exchange[];
    extern int cqwwm2;
    extern int arrlss;
    extern int wpx;
    extern char whichcontest[];

    int rc;
    char standardexchange[70] = "";
    char cabrillo_tmp_name[80];
    char cmd[80];
    char buf[181];
    char buffer[4000] = "";
    double freq;
    char freq_buf[16];

    FILE *fp1, *fp2;

    getsummary();

    strcpy(cabrillo_tmp_name, call);
    cabrillo_tmp_name[strlen(call)-1] = '\0'; /* drop \n */
    strcat(cabrillo_tmp_name, ".cbr");

    if (strlen(exchange) > 0)
	strcpy(standardexchange, exchange);

    if ((fp1 = fopen(logfile, "r")) == NULL) {
	fprintf(stdout, "Opening logfile not possible.\n");
	return (1);
    }
    if ((fp2 = fopen("./cabrillo", "w")) == NULL) {
	fprintf(stdout, "Opening cbr file not possible.\n");
	fclose(fp1);		//added by F8CFE
	return (2);
    }
    if (strlen(standardexchange) == 0) {
	nicebox(14, 0, 1, 78, "Exchange used:");
	mvprintw(15, 1,
		 "                                                       ");
	mvprintw(15, 1, "");
	attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
	echo();
	if (arrlss == 1)
	    getnstr(standardexchange, 6);
	else
	    getnstr(standardexchange, 10);

	noecho();
    }

    while ( fgets(buf, 180, fp1) != NULL ) {

	if (buf[0] != ';' && strlen(buf) > 60) {

	    buffer[0] = '\0';

	    strcat(buffer, "QSO: ");
/*------------------------------------------------------------------
frequency
-------------------------------------------------------------------*/
/* use exact FREQ if available */
	    freq = 0.;
	    if (strlen(buf) > 81) {
		freq = atof(buf+80);
	    }

	    if ((freq > 1799.) && (freq <= 30000.)) {
		sprintf(freq_buf, "%5d", (int)(freq+0.5));
		strcat(buffer, freq_buf);
	    } else {
		/* otherwise look into band definition */
		if (buf[1] == '6')
		    strcat(buffer, " 1800");
		else if (buf[1] == '8')
		    strcat(buffer, " 3500");
		else if (buf[1] == '4')
		    strcat(buffer, " 7000");
		else if (buf[1] == '2')
		    strcat(buffer, "14000");
		else if (buf[1] == '1' && buf[2] == '5')
		    strcat(buffer, "21000");
		else if (buf[1] == '1' && buf[2] == '0')
		    strcat(buffer, "28000");
	    }
/*------------------------------------------------------------------
mode
-------------------------------------------------------------------*/

	    if (buf[3] == 'C')
		strcat(buffer, " CW 20");
	    else if (buf[3] == 'S')
		strcat(buffer, " PH 20");
	    else
		strcat(buffer, " RY 20");

/*------------------------------------------------------------------
date
-------------------------------------------------------------------*/

	    strncat(buffer, buf + 14, 2);	/* year */

	    if (buf[10] == 'J' && buf[11] == 'a')
		strcat(buffer, "-01-");
	    if (buf[10] == 'F')
		strcat(buffer, "-02-");
	    if (buf[10] == 'M' && buf[12] == 'r')
		strcat(buffer, "-03-");
	    if (buf[10] == 'A' && buf[12] == 'r')
		strcat(buffer, "-04-");
	    if (buf[10] == 'M' && buf[12] == 'y')
		strcat(buffer, "-05-");
	    if (buf[10] == 'J' && buf[11] == 'u' && buf[12] == 'n')
		strcat(buffer, "-06-");
	    if (buf[10] == 'J' && buf[12] == 'l')
		strcat(buffer, "-07-");
	    if (buf[10] == 'A' && buf[12] == 'g')
		strcat(buffer, "-08-");
	    if (buf[10] == 'S')
		strcat(buffer, "-09-");
	    if (buf[10] == 'O')
		strcat(buffer, "-10-");
	    if (buf[10] == 'N')
		strcat(buffer, "-11-");
	    if (buf[10] == 'D')
		strcat(buffer, "-12-");
/*------------------------------------------------------------------
day
-------------------------------------------------------------------*/

	    strncat(buffer, buf + 7, 2);
/*------------------------------------------------------------------
time
-------------------------------------------------------------------*/

	    strncat(buffer, buf + 16, 3);
	    strncat(buffer, buf + 20, 3);
/*------------------------------------------------------------------
mycall
-------------------------------------------------------------------*/

	    strncat(buffer, call, strlen(call) - 1);	/* strip the \n */
	    strncat(buffer, backgrnd_str, 15 - strlen(call));
/*------------------------------------------------------------------
exchange given
-------------------------------------------------------------------*/

	    if (arrlss == 1) {
/*------------------------------------------------------------------
report given
-------------------------------------------------------------------*/

		sprintf(buffer + 41, "%4d", atoi(buf + 22));
		strcat(buffer, "                    ");
/*------------------------------------------------------------------
exchange given
-------------------------------------------------------------------*/
		sprintf(buffer + 46, "%c", standardexchange[0]);
		strcat(buffer, "                    ");
		sprintf(buffer + 48, "%2d", atoi(standardexchange + 1));
		strcat(buffer, "                    ");
		sprintf(buffer + 51, "%s", standardexchange + 3);
		strcat(buffer, "                    ");
		sprintf(buffer + 55, "%s", buf + 29);
		strcat(buffer, "                    ");
/*------------------------------------------------------------------
exchange received
-------------------------------------------------------------------*/

		sprintf(buffer + 66, "%4d", atoi(buf + 54));
		strcat(buffer, "                    ");
		sprintf(buffer + 71, "%c", buf[59]);
		strcat(buffer, "                    ");
		sprintf(buffer + 72, "%s", buf + 60);
		strcat(buffer, "                    ");
		sprintf(buffer + 75, "%s", buf + 63);
		strcat(buffer, "                    ");
		buffer[79] = '\0';
		strcat(buffer, "\n");

	    } else		// not arllss
	    {
/*------------------------------------------------------------------
report given
-------------------------------------------------------------------*/

		if (buf[3] == 'S')
		    strcat(buffer, "59  ");
		else
		    strcat(buffer, "599 ");
/*------------------------------------------------------------------
exchange given
-------------------------------------------------------------------*/

		if (other_flg == 1 || wysiwyg_multi == 1
		    || wysiwyg_once == 1) {
		    strcat(buffer, standardexchange);
		    strncat(buffer, "            ",
			    7 - strlen(standardexchange));
		}
		/* end other (wysiwyg) */
		else if ((wpx == 1) || ((standardexchange[0] == '#')
					&& (strcmp(whichcontest, "ssa_mt")
					    != 0))) {
		    strncat(buffer, buf + 23, 4);
		    strncat(buffer, standardexchange + 1, 7);
		    strcat(buffer, " ");
		}

		else if (cqww == 1) {
		    strcat(buffer, mycqzone);
		    strcat(buffer, "     ");
		}

		else if (arrldx_usa == 1) {
		    strncat(buffer, exchange, 2);
		    strcat(buffer, "     ");

		} else if (serial_grid4_mult == 1) {
		    strcat(buffer, "  ");
		    sprintf(buffer + 49, "%s", buf + 24);
		    sprintf(buffer + 52, "%s", standardexchange + 1);

		    strcat(buffer, "                ");
		    sprintf(buffer + 60, "%s          ", buf + 29);
		    buffer[74] = '\0';
		} else {

		    strncat(buffer, standardexchange, 10);
		    strncat(buffer, "     ", strlen(buffer) - 8);
		}

/*------------------------------------------------------------------
his call
-------------------------------------------------------------------*/

		if (strcmp(whichcontest, "ssa_mt") != 0)
		    strncat(buffer, buf + 29, 14);

/*------------------------------------------------------------------
rprt given
-------------------------------------------------------------------*/

		if (buf[3] == 'S')
		    strcat(buffer, "59  ");
		else
		    strcat(buffer, "599 ");

		if (serial_grid4_mult == 1) {
		    char ssa_mt_exchange[30];
		    int i = 0, j = 0, k = 0;
//                                      strncat(buffer, buf+54, 9);  // tbf for all contests? RC
		    strcat(buffer, "                      ");

		    sprintf(buffer + 79, "%03d   ", atoi(buf + 54));

		    for (i = 0; i < 12; i++) {
			if (isalpha(buf[54 + i])) {
			    for (j = 0; j < (13 - i); j++) {
				if (isalnum(buf[54 + i + j])) {
				    ssa_mt_exchange[k] = buf[54 + i + j];
				    k++;
				} else {
				    if (j > 0
					&& isspace(buf[54 + i + j - 1])) {
					ssa_mt_exchange[k] = '\0';
					break;
				    }
				}
			    }
			    if (j > 0)
				break;
			}
		    }

		    sprintf(buffer + 83, "%s        ", ssa_mt_exchange);
		    sprintf(buffer + 90, "%s", "0");
		} else
		    strncat(buffer, buf + 54, 6);

		strcat(buffer, "  ");

		if ((cqww == 1) && (cqwwm2 == 1)) {	// cqww M2 mode
		    if (buf[79] == '*') {
			strcat(buffer, " 1\n");
		    } else
			strcat(buffer, " 0\n");
		} else {
		    if (strcmp(whichcontest, "ssa_mt") == 1)
			strcat(buffer, " 0\n");
		    else
			strcat(buffer, "\n");
		}
	    }			// end else arrlss

	    if (strlen(buffer) > 11)
		fputs(buffer, fp2);

	}

    }				// end while !eof
    fclose(fp1);

    fputs("END-OF-LOG:\n", fp2);
    fclose(fp2);

    rc = system("cat cabrillo >> header");
    sprintf(cmd, "cp header %s", cabrillo_tmp_name);
    rc = system(cmd);
    rc = system("mv header summary.txt");

    return (0);
}

#endif


/*
    The ADIF function has been written according ADIF v1.00 specifications
    as shown on http://www.adif.org
    LZ3NY
*/
int write_adif(void)
{

    extern char logfile[];
    extern char exchange[];
    extern char whichcontest[];
    extern int exchange_serial;
    extern char modem_mode[];

    char buf[181] = "";
    char buffer[181] = "";
    char standardexchange[70] = "";
    char adif_tmp_name[40] = "";
    char adif_tmp_call[13] = "";
    char adif_tmp_str[2] = "";
    char adif_year_check[3] = "";
    char adif_rcvd_num[16] = "";
    char resultat[16];
    char adif_tmp_rr[5] = "";
    double freq;
    char freq_buf[16];

    int adif_mode_dep = 0;

    FILE *fp1, *fp2;

    if ((fp1 = fopen(logfile, "r")) == NULL) {
	fprintf(stdout, "Opening logfile not possible.\n");
	return (1);
    }
    strcpy(adif_tmp_name, whichcontest);
    strcat(adif_tmp_name, ".adi");

    if ((fp2 = fopen(adif_tmp_name, "w")) == NULL) {
	fprintf(stdout, "Opening ADIF file not possible.\n");
	fclose(fp1);		//added by F8CFE
	return (2);
    } 

    if (strlen(exchange) > 0)
	strcpy(standardexchange, exchange);

    /* in case using write_adif() without write_cabrillo() 
     * just ask for the needed information */
    if ((strlen(standardexchange) == 0) && (exchange_serial != 1)) {
	nicebox(14, 0, 1, 78, "Exchange used:");
	attron(COLOR_PAIR(C_WINDOW) | A_STANDOUT );
	mvprintw(15, 1,
		 "                                                                              ");
	mvprintw(15, 1, "");
	echo();
	getnstr(standardexchange, 30);
	noecho();
    }

    /* write header */
    fputs
	("################################################################################\n",
	 fp2);
    fputs
	("#                     ADIF v1.00 data file exported by TLF\n",
	 fp2);
    fputs
	("#              according to specifications on http://www.adif.org\n",
	 fp2);
    fputs("#\n", fp2);
    fputs
	("################################################################################\n",
	 fp2);
    fputs("<adif_ver:4>1.00\n<eoh>\n", fp2);

    while (fgets(buf, sizeof(buf), fp1)) {

	buffer[0] = '\0';

	if ((buf[0] != ';') && ((buf[0] != ' ') || (buf[1] != ' '))
	    && (buf[0] != '#') && (buf[0] != '\n') && (buf[0] != '\r')) {

/* CALLSIGN */
	    strcat(buffer, "<CALL:");
	    strncpy(adif_tmp_call, buf + 29, 12);
	    strcpy(adif_tmp_call, g_strstrip(adif_tmp_call));
	    snprintf(resultat, sizeof(resultat), "%zd",
		     strlen(adif_tmp_call));
	    strcat(buffer, resultat);
	    strcat(buffer, ">");
	    strcat(buffer, adif_tmp_call);

/* BAND */
	    if (buf[1] == '6')
		strcat(buffer, "<BAND:4>160M");
	    else if (buf[1] == '8')
		strcat(buffer, "<BAND:3>80M");
	    else if (buf[1] == '4')
		strcat(buffer, "<BAND:3>40M");
	    else if (buf[1] == '3')
		strcat(buffer, "<BAND:3>30M");
	    else if (buf[1] == '2')
		strcat(buffer, "<BAND:3>20M");
	    else if (buf[1] == '1' && buf[2] == '5')
		strcat(buffer, "<BAND:3>15M");
	    else if (buf[1] == '1' && buf[2] == '7')
		strcat(buffer, "<BAND:3>17M");
	    else if (buf[1] == '1' && buf[2] == '0')
		strcat(buffer, "<BAND:3>10M");

/* FREQ if available */
	    if (strlen(buf) > 81) {
		freq = atof(buf+80);
		freq_buf[0] = '\0';
		if ((freq > 1799.) && (freq < 10000.)) {
		    sprintf(freq_buf, "<FREQ:6>%.4f", freq/1000.);
		} else if (freq >= 10000.) {
		    sprintf(freq_buf, "<FREQ:7>%.4f", freq/1000.);
		}
		strcat(buffer, freq_buf);
	    }

/* QSO MODE */
	    if (buf[3] == 'C')
		strcat(buffer, "<MODE:2>CW");
	    else if (buf[3] == 'S')
		strcat(buffer, "<MODE:3>SSB");
	    else if (strcmp(modem_mode, "RTTY") == 0)
		strcat(buffer, "<MODE:4>RTTY");
	    else
		/* \todo DIGI is no allowed mode */
		strcat(buffer, "<MODE:4>DIGI");

/* QSO_DATE */
	    /* Y2K :) */
	    adif_year_check[0] = '\0';
	    strncpy(adif_year_check, buf + 14, 2);
	    if (atoi(adif_year_check) <= 70)
		strcat(buffer, "<QSO_DATE:8>20");
	    else
		strcat(buffer, "<QSO_DATE:8>19");

	    /* year */
	    strncat(buffer, buf + 14, 2);

	    /*month */
	    if (buf[10] == 'J' && buf[11] == 'a')
		strcat(buffer, "01");
	    if (buf[10] == 'F')
		strcat(buffer, "02");
	    if (buf[10] == 'M' && buf[12] == 'r')
		strcat(buffer, "03");
	    if (buf[10] == 'A' && buf[12] == 'r')
		strcat(buffer, "04");
	    if (buf[10] == 'M' && buf[12] == 'y')
		strcat(buffer, "05");
	    if (buf[10] == 'J' && buf[11] == 'u' && buf[12] == 'n')
		strcat(buffer, "06");
	    if (buf[10] == 'J' && buf[12] == 'l')
		strcat(buffer, "07");
	    if (buf[10] == 'A' && buf[12] == 'g')
		strcat(buffer, "08");
	    if (buf[10] == 'S')
		strcat(buffer, "09");
	    if (buf[10] == 'O')
		strcat(buffer, "10");
	    if (buf[10] == 'N')
		strcat(buffer, "11");
	    if (buf[10] == 'D')
		strcat(buffer, "12");

	    /*date */
	    strncat(buffer, buf + 7, 2);

/* TIME_ON */
	    strcat(buffer, "<TIME_ON:4>");
	    strncat(buffer, buf + 17, 2);
	    strncat(buffer, buf + 20, 2);

	    /* RS(T) flag */
	    if (buf[3] == 'S')		/* check for SSB */
		adif_mode_dep = 2;
	    else
		adif_mode_dep = 3;

/* RST_SENT */
	    strcat(buffer, "<RST_SENT:");
	    adif_tmp_str[1] = '\0';	/*       PA0R 02/10/2003  */
	    adif_tmp_str[0] = adif_mode_dep + 48;
	    strcat(buffer, adif_tmp_str);
	    strcat(buffer, ">");
	    strncat(buffer, buf + 44, adif_mode_dep);

/* STX - sent contest number */
	    strcat(buffer, "<STX:");

	    if ((exchange_serial == 1) || (standardexchange[0] == '#')) {
		strcat(buffer, "4>");
		strncat(buffer, buf + 23, 4);
	    } else {
		snprintf(resultat, sizeof(resultat), "%zd",
			 strlen(standardexchange));
		strcat(buffer, resultat);
		strcat(buffer, ">");
		strcat(buffer, g_strstrip(standardexchange));
	    }

/* RST_RCVD */
	    strncpy(adif_tmp_rr, buf + 49, 4);
	    strcpy(adif_tmp_rr, g_strstrip(adif_tmp_rr));
	    strcat(buffer, "<RST_RCVD:");
	    snprintf(resultat, sizeof(resultat), "%zd",
		     strlen(adif_tmp_rr));
	    strcat(buffer, resultat);
	    strcat(buffer, ">");
	    strncat(buffer, buf + 49, adif_mode_dep);

/* SRX - received contest number */
	    strncpy(adif_rcvd_num, buf + 54, 14);
	    strcpy(adif_rcvd_num, g_strstrip(adif_rcvd_num));
	    snprintf(resultat, sizeof(resultat), "%zd",
		     strlen(adif_rcvd_num));
	    strcat(buffer, "<SRX:");
	    strcat(buffer, resultat);
	    strcat(buffer, ">");
	    if (strcmp(buf + 54, " ") != 0)
		strcat(buffer, adif_rcvd_num);

/* <EOR> */
	    strcat(buffer, "<eor>\n");	//end of ADIF row

	    fputs(buffer, fp2);
	}
    }				// end fgets() loop

    fclose(fp1);
    fclose(fp2);

    return (0);
}				// end write_adif
