#include "cluster_bg.h"

extern int cluster;
extern int announcefilter;
extern int bandinx;

int cluster_bg(int cluster_arg)
{

    extern char spot_ptr[MAX_SPOTS][82];
    extern int spotarray[MAX_SPOTS];
    extern char spotinfo[32][82];
    extern int announcefilter;
    extern int bandinx;

    static int clustermode = 0;
    int k, l;
    char textbuffer[82];

    if (cluster_arg == NOCLUSTER)
	return (1);

    if (cluster_arg == MAP)
	clustermode = bandinx;
    if (cluster_arg == SPOTS)
	clustermode = 9;
    if (cluster_arg != CLUSTER)
    {
	announcefilter = FILTER_DX;

	k = getclusterinfo();

	if (k > (MAX_SPOTS - 2))
	    k = MAX_SPOTS - 2;

	if (clustermode != 9) {	// map

	    if (k < 8)
		k = 8;

	    for (l = k - 8; l <= k; l++) {

		strncpy(textbuffer, spot_ptr[spotarray[l]], 82);
		spotinfo[l][0] = '\0';

		if (strlen(textbuffer) > 40) {
		    strncpy(spotinfo[l], textbuffer + 17, 21);
		    strncat(spotinfo[l], textbuffer + 69, 8);
		    spotinfo[l][29] = '\0';
		}
	    }

	} else {

	    if (k < 8)
		k = 8;

	    for (l = k - 8; l <= k; l++) {

		strncpy(textbuffer, spot_ptr[spotarray[l]], 82);
		spotinfo[l][0] = '\0';

		if (strlen(textbuffer) > 40) {
		    strncpy(spotinfo[l], textbuffer + 17, 21);
		    strncat(spotinfo[l], textbuffer + 69, 8);
		    spotinfo[l][29] = '\0';
		}
	    }

	}
    }

    if (cluster_arg == CLUSTER) {	/* all cluster info  */

	k = getclusterinfo();

	if (k > (MAX_SPOTS - 2))
	    k = MAX_SPOTS - 2;

    }

    return (0);

}

int getclusterinfo(void)
{

    extern char spot_ptr[MAX_SPOTS][82];
    extern char lastwwv[];
    extern int ptr;
    extern int spot_rdr;
    extern int spotarray[];
    extern int announcefilter;
    extern char call[];
    extern int bandinx;

    int bg, nd, i;
    int si = 0;
    int in_map = 0;
    char calldupe[12];

    strcpy(calldupe, call);
    calldupe[strlen(call) - 1] = '\0';

    nd = ptr;
    bg = spot_rdr;
    i = 0;
    si = 0;

    for (si = 0; si < (MAX_SPOTS - 2); si++)
	spotarray[si] = -1;
    si = 0;

    while (1) {

	if (strstr(spot_ptr[i], "DX de") != NULL) {

	    in_map = 1;

	    if (cluster == MAP) {

		in_map = 0;

		switch (atoi(spot_ptr[i] + 17)) {
		case 1800 ... 1850:
		    if (bandinx == 0)
			in_map = 1;
		    break;

		case 3500 ... 4000:
		    if (bandinx == 1)
			in_map = 1;
		    break;

		case 7000 ... 7200:
		    if (bandinx == 2)
			in_map = 1;
		    break;

		case 10100 ... 10150:
		    if (bandinx == 3)
			in_map = 1;
		    break;

		case 14000 ... 14350:
		    if (bandinx == 4)
			in_map = 1;
		    break;

		case 18068 ... 18168:
		    if (bandinx == 5)
			in_map = 1;
		    break;

		case 21000 ... 21450:
		    if (bandinx == 6)
			in_map = 1;
		    break;

		case 24900 ... 24950:
		    if (bandinx == 7)
			in_map = 1;
		    break;

		case 28000 ... 29600:
		    if (bandinx == 8)
			in_map = 1;
		    break;

		default:
		    in_map = 0;
		}

	    } else {
		in_map = 1;	//always 1 if not in MAP mode
	    }

	    if (in_map == 1) {
		spotarray[si] = i;
		si++;

	    }

	    i++;

	} else if (strstr(spot_ptr[i], "WCY") != NULL) {
	    if ((cluster == CLUSTER)) {
		spotarray[si] = i;
		si++;
	    }
	    strncpy(lastwwv, spot_ptr[i], 82);
	    i++;
	} else if (strstr(spot_ptr[i], "WWV") != NULL) {
	    if ((cluster == CLUSTER)) {
		spotarray[si] = i;
		si++;
	    }
	    strncpy(lastwwv, spot_ptr[i], 82);
	    i++;
	} else if (strstr(spot_ptr[i], calldupe) != NULL) {
	    if ((cluster == CLUSTER) && (announcefilter <= 2)) {
		spotarray[si] = i;
		si++;
		i++;
	    } else
		i++;

	} else if (strstr(spot_ptr[i], "To ALL") != NULL) {
	    if ((cluster == CLUSTER) && (announcefilter <= 1)) {
		spotarray[si] = i;
		si++;

	    }
	    i++;
	} else if ((cluster == CLUSTER) && (announcefilter == 0)
		   && (strlen(spot_ptr[i]) > 20)) {

	    spotarray[si] = i;
	    si++;

	    i++;

	} else
	    i++;

	if (i > (ptr - 1))
	    break;

    }

    return (si - 1);
}

/* ------------------------------------------------------

	globals for bandmap

------------------------------------------------------*/

char *bandmap[MAX_SPOTS];
struct tln_logline *temps;
int allspots = 0;

/* ----------------------------------------------------*/

int loadbandmap(void)
{

    extern int cluster;
    extern char *bandmap[MAX_SPOTS];
    extern struct tln_logline *loghead;
    extern struct tln_logline *temps;
    extern struct tm *time_ptr;
    extern int bandmap_pos;
    extern int countries[];
    extern int call_band[];
    extern int allspots;
    extern char datalines[MAX_DATALINES][81];
    extern int xplanet;
    extern char markerfile[];
    extern int countrynr;
    extern char lastmsg[];
    extern char cqzone[];

    int i = 0, j, jj, changeflg, k, m, x, y, done;
    int in_map;
    int spotminutes = 0;
    int sysminutes = 0;
    int timediff = 0;
    int recent;
    int linepos;
    int worked;
    int thisband = 10;
    int dupe;
    int spot_age[MAX_SPOTS];
    float spot_freq[MAX_SPOTS];
    float freqbuffer;
    int timebuff;

    char *thisline;
    char *tmp;
    char spotcall[20];
    char spottime[6];
    char syshour[3];
    char sysmins[3];
    char spotline[38];
    char tmp1[81];
    char tmp2[81];
    char callcopy[81];
    char cqzonebuffer[3];
    FILE *fp;
    char marker_out[40];
    int lon;
    int lat;
    int yy, zz;
    int nofile = 0;
    int iswarc = 0;
    char xplanetmsg[160];

    for (i = 0; i < 200; i++)
	bandmap[i] = NULL;

    j = 0;

    i = 0;

    get_time();
//strftime(syshour, 80, "%H", time_ptr);        ### bug fix
//strftime(sysmins, 80, "%M", time_ptr);        ### bug fix
    strftime(syshour, sizeof(syshour), "%H", time_ptr);
    strftime(sysmins, sizeof(sysmins), "%M", time_ptr);
    sysminutes = 60 * atoi(syshour) + atoi(sysmins);

    if (loghead) {
	firstlogline();
    }

    while (temps != NULL) {

	thisline = nextlogline();

	if (thisline == NULL)
	    break;
	else {
	    if (strncmp(thisline, "DX de ", 6) == 0) {

		strncpy(spotcall, thisline + 26, 5);
		spotcall[5] = '\0';

		strncpy(spottime, thisline + 70, 4);	// how old?
		spottime[4] = spottime[3];
		spottime[3] = spottime[2];
		spottime[2] = ':';
		spotminutes = 60 * atoi(spottime) + atoi(spottime + 3);
		timediff = (sysminutes - spotminutes) + 5;
		if (timediff + 30 < 0)
		    timediff += 1440;

		if ((timediff + 30) <= (MAXMINUTES + 30))
		    recent = 1;
		else {
		    recent = 0;
		    thisline[0] = 'd';
		}

		if (recent == 1) {

		    done = 0;	// duplicate ? kill it.

		    for (k = 0; k <= i - 1; k++) {
			callcopy[0] = '\0';
			strncat(callcopy, bandmap[k] + 26, 5);

			if (strncmp(callcopy, spotcall, 4) == 0) {
			    bandmap[k][0] = 'd';
			    break;
			}
		    }

		    if (cluster == MAP)
			in_map = 0;
		    else
			in_map = 1;

		    thisband = 10;

		    switch (atoi(thisline + 17))	// right freq?
		    {
		    case 1800 ... 1850:
			if (bandinx == 0)
			    in_map = 1;
			thisband = BANDINDEX_160;
			break;

		    case 3500 ... 4000:
			if (bandinx == 1)
			    in_map = 1;
			thisband = BANDINDEX_80;
			break;

		    case 7000 ... 7200:
			if (bandinx == 2)
			    in_map = 1;
			thisband = BANDINDEX_40;
			break;

		    case 10100 ... 10150:
			if (bandinx == 3)
			    in_map = 1;
			break;

		    case 14000 ... 14350:
			if (bandinx == 4)
			    in_map = 1;
			thisband = BANDINDEX_20;
			break;

		    case 18068 ... 18168:
			if (bandinx == 5)
			    in_map = 1;
			break;

		    case 21000 ... 21450:
			if (bandinx == 6)
			    in_map = 1;
			thisband = BANDINDEX_15;
			break;

		    case 24890 ... 24990:
			if (bandinx == 7)
			    in_map = 1;
			break;

		    case 28000 ... 29600:
			if (bandinx == 8)
			    in_map = 1;
			thisband = BANDINDEX_10;
			break;

		    default:
			in_map = 0;
			thisband = BANDINDEX_160;

		    }

		    if (done == 0 && in_map == 1 && recent == 1) {
			bandmap[i] = thisline;
			spot_age[i] = timediff;
			spot_freq[i] = atof(thisline + 17);
			i++;
		    }
		    done = 0;
		}
	    }
	}
    }
    // ---------------------sort the array ---------------------------------------
    changeflg = 1;

    while ((changeflg == 1) && (cluster == MAP)) {	//  sort the spots

	changeflg = 0;

	for (j = 0; j < i - 1; j++) {

	    strcpy(tmp1, bandmap[j]);
	    strcpy(tmp2, bandmap[j + 1]);

	    if ((tmp1 == NULL) || (tmp2 == NULL))
		break;

	    if ((atof(tmp1 + 16)) > (atof(tmp2 + 16))) {
		tmp = bandmap[j];
		timebuff = spot_age[j];
		freqbuffer = spot_freq[j];
		bandmap[j] = bandmap[j + 1];
		spot_age[j] = spot_age[j + 1];
		spot_freq[j] = spot_freq[j + 1];
		bandmap[j + 1] = tmp;
		spot_age[j + 1] = timebuff;
		spot_freq[j + 1] = freqbuffer;
		changeflg = 1;

	    }

	}

	if (changeflg == 0)
	    break;
    }				// end while

    //------------------end sort ---------------------------------------------

    attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);	// display it

    for (j = 15; j < 23; j++)
	mvprintw(j, 4, "                           ");

    if (cluster == SPOTS)
	linepos = (i < 8 ? 0 : i - 8);
    else
	linepos = 0;

    if (cluster == MAP)
	linepos = bandmap_pos;

    jj = 0;

    if (xplanet > 0 && nofile == 0) {
	if ((fp = fopen(markerfile, "w")) == NULL) {	//start with empty file
	    nofile = 1;
	    mvprintw(24, 0, "Opening marker file not possible.\n");
	    refresh();
	} else
	    fclose(fp);
    }

    for (j = linepos; j < linepos + 8; j++) {

	if (bandmap[j] != NULL) {
	    strncpy(spotline, bandmap[j] + 17, 22);
	    spotline[22] = '\0';
	    strncpy(spottime, bandmap[j] + 70, 5);
	    spottime[5] = '\0';
	    strcat(spotline, spottime);

	    strncpy(callcopy, bandmap[j] + 26, 16);
	    for (m = 0; m < 16; m++) {
		if (callcopy[m] == ' ') {
		    callcopy[m] = '\0';
		    break;
		}
	    }
	    yy = countrynr;
	    strcpy(cqzonebuffer, cqzone);

	    x = getctydata(callcopy);

	    strcpy(cqzone, cqzonebuffer);	// to be fixed: getctydata.c should not change cqzone
	    countrynr = yy;

	    y = searchcallarray(callcopy);

	    worked = 0;
	    dupe = 1;

	    if (cluster == MAP)
		thisband = bandinx;

	    switch (thisband) {
	    case BANDINDEX_160:
		if ((countries[x] & BAND160) != 0)
		    worked = 1;
		if ((call_band[y] & BAND160) == 0)
		    dupe = 0;
		break;
	    case BANDINDEX_80:
		if ((countries[x] & BAND80) != 0)
		    worked = 1;
		if ((call_band[y] & BAND80) == 0)
		    dupe = 0;
		break;
	    case BANDINDEX_40:
		if ((countries[x] & BAND40) != 0)
		    worked = 1;
		if ((call_band[y] & BAND40) == 0)
		    dupe = 0;
		break;
	    case BANDINDEX_20:
		if ((countries[x] & BAND20) != 0)
		    worked = 1;
		if ((call_band[y] & BAND20) == 0)
		    dupe = 0;
		break;
	    case BANDINDEX_15:
		if ((countries[x] & BAND15) != 0)
		    worked = 1;
		if ((call_band[y] & BAND15) == 0)
		    dupe = 0;
		break;
	    case BANDINDEX_10:
		if ((countries[x] & BAND10) != 0)
		    worked = 1;
		if ((call_band[y] & BAND10) == 0)
		    dupe = 0;
		break;
	    default:
		worked = 1;
		dupe = 0;

	    }
	    if (x != 0 && xplanet > 0 && nofile == 0) {

		if ((fp = fopen(markerfile, "a")) == NULL) {
		    mvprintw(24, 0, "Opening markerfile not possible.\n");
		}

		strcpy(marker_out, "                         ");

		lon = atoi(datalines[x] + 40);
		lat = atoi(datalines[x] + 50) * -1;
		sprintf(marker_out, "%d   %d", lon, lat);

		marker_out[12] = '\0';
		strcat(marker_out, "   \"");
		if (xplanet == 1 || xplanet == 3)
		    strcat(marker_out, callcopy);
		if (spot_age[j] > 15 && cluster != SPOTS)
		    strcat(marker_out, "\"   color=Green\n");
		else {

		    iswarc = 0;
		    if (spot_freq[j] >= 10100.0 && spot_freq[j] <= 10150.0)
			iswarc = 1;
		    if (spot_freq[j] >= 18068.0 && spot_freq[j] <= 18168.0)
			iswarc = 1;
		    if (spot_freq[j] >= 24890.0 && spot_freq[j] <= 24990.0)
			iswarc = 1;

		    if (iswarc == 0) {
			if (cluster == MAP) {
			    if (worked == 1) {
				strcat(marker_out, "\"   color=Yellow\n");
				worked = 0;
			    } else
				strcat(marker_out, "\"   color=White\n");
			} else {
			    if (spot_freq[j] < 3500.0)
				strcat(marker_out, "\"   color=Red\n");
			    if (spot_freq[j] >= 3500.0
				&& spot_freq[j] <= 4000.0)
				strcat(marker_out, "\"   color=Magenta\n");
			    if (spot_freq[j] >= 7000.0
				&& spot_freq[j] <= 7300.0)
				strcat(marker_out, "\"   color=Yellow\n");
			    if (spot_freq[j] >= 14000.0
				&& spot_freq[j] <= 14350.0)
				strcat(marker_out, "\"   color=Blue\n");
			    if (spot_freq[j] >= 21000.0
				&& spot_freq[j] <= 21450.0)
				strcat(marker_out, "\"   color=White\n");
			    if (spot_freq[j] >= 28000.0
				&& spot_freq[j] <= 29000.0)
				strcat(marker_out, "\"   color=Green\n");
			    if (iswarc == 1)
				strcat(marker_out, "\"   color=Cyan\n");

			}
		    } else {
//                                              iswarc = 0;
			strcat(marker_out, "\"   color=Cyan\n");

		    }

		}
		fputs(marker_out, fp);

		fclose(fp);
	    }

	    if (y == 0)
		dupe = 1;

	    if (worked == 1 && thisband < 10) {
		if (allspots == 1) {
		    attron(COLOR_PAIR(COLOR_YELLOW));
		    mvprintw(15 + jj, 4, "%s", spotline);

		    switch (spot_age[j]) {
		    case 0 ... 15:
			attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
			mvprintw(15 + jj, 25, "%s", spotline + 21);
		    }

		    jj++;
		    if (jj > 7)
			break;
		}
	    } else {
		attron(COLOR_PAIR(COLOR_CYAN) | A_STANDOUT);
		mvprintw(15 + jj, 4, "%s", spotline);
		switch (spot_age[j]) {
		case 0 ... 15:
		    attron(COLOR_PAIR(COLOR_GREEN) | A_STANDOUT);
		    mvprintw(15 + jj, 25, "%s", spotline + 21);
		}
		jj++;
		if (jj > 7)
		    break;

	    }
	}

    }

    if (xplanet == 1 && nofile == 0) {

	xplanetmsg[0] = '\0';
	strcat(xplanetmsg, "-82 -120 ");
	strcat(xplanetmsg, "\"");
	strcat(xplanetmsg, lastmsg);

	for (zz = 0; zz < strlen(lastmsg); zz++)
	    if (lastmsg[zz] == 34)
		lastmsg[zz] = ' ';

	strcat(xplanetmsg, "\"   color=Cyan\n");

	if ((fp = fopen(markerfile, "a")) == NULL) {
	    mvprintw(24, 0, "Opening markerfile not possible.\n");
	} else {
	    if (strlen(xplanetmsg) > 20)
		fputs(xplanetmsg, fp);

	    fclose(fp);
	}
    }

    if (cluster == MAP && allspots == 1)
	nicebox(14, 3, 8, 27, "Bandmap");
    if (cluster == MAP && allspots == 0)
	nicebox(14, 3, 8, 27, "Needed");

    if (cluster == SPOTS)
	nicebox(14, 3, 8, 27, "Spots");

    if (cluster == MAP && linepos > 7) {
	mvprintw(14, 17, "+");
	if (cluster == MAP && linepos > 7)
	    mvprintw(23, 17, "v");
	mvprintw(14, 23, "%d", linepos);
    }

    refresh();

    return (i);			//---------------------------the end  ------------------
}

char *firstlogline(void)
{
    extern struct tln_logline *loghead;
    extern struct tln_logline *temps;

    temps = loghead;
    return temps->text;
}

char *nextlogline(void)
{
    extern struct tln_logline *temps;

    if (temps->next != NULL) {
	temps = temps->next;
	return temps->text;
    } else
	return (NULL);

}
