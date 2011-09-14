#include "cluster_bg.h"
#include "dxcc.h"

// \todo drop
#include "bandmap.h"

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
	clustermode = NBANDS;
    if (cluster_arg != CLUSTER)
    {
	announcefilter = FILTER_DX;

	k = getclusterinfo();

	if (k > (MAX_SPOTS - 2))
	    k = MAX_SPOTS - 2;

	if (clustermode != NBANDS) {	// map

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
    extern int spotarray[];
    extern int announcefilter;
    extern char call[];
    extern int bandinx;

    int i;
    int si = 0;
    int in_map = 0;
    char calldupe[12];

    strcpy(calldupe, call);
    calldupe[strlen(call) - 1] = '\0';

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
    extern int xplanet;
    extern char markerfile[];
    extern char lastmsg[];

    int i = 0, j, jj, changeflg, k, m, x, y, done;
    int in_map;
    int spotminutes = 0;
    int sysminutes = 0;
    int timediff = 0;
    int linepos;
    int worked;
    int thisband = 10;		/** \todo should it be NBANDS? */
    int dupe;
    int spot_age[MAX_SPOTS];
    float spot_freq[MAX_SPOTS];
    float freqbuffer;
    int timebuff;

    char *thisline;
    char *tmp;
    char spotcall[20];
    char spottime[6];
    char spotline[38];
    char tmp1[81];
    char tmp2[81];
    char callcopy[81];
    FILE *fp;
    char marker_out[60];
    int lon;
    int lat;
    int zz;
    int nofile = 0;
    int iswarc = 0;
    char xplanetmsg[160];
    dxcc_data *dx;

    for (i = 0; i < MAX_SPOTS; i++)
	bandmap[i] = NULL;

    j = 0;

    i = 0;

    get_time();
    sysminutes = 60 * time_ptr->tm_hour + time_ptr->tm_min;

    /* parse log of cluster output and find DX announcements.
     * Copy them to bandmap array and find spot_age and spot_freq 
     */
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
		spottime[5] = '\0';
		spotminutes = 60 * atoi(spottime) + atoi(spottime + 3);
		timediff = (sysminutes - spotminutes) + 5;
		if (timediff + 30 < 0)
		    timediff += 1440;

		/* is spot recent? */
		if ((timediff + 30) <= (MAXMINUTES + 30)) {

		    /* yes, so process it */

		    done = 0;	
		    
		    /* look for duplicates already in bandmap 
		     * => kill it older one and keep younger entry */
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

		    if (done == 0 && in_map == 1) {
			bandmap[i] = thisline;
			spot_age[i] = timediff;
			spot_freq[i] = atof(thisline + 17);
			i++;
		    }
		    done = 0;
		} else {
		    /* no longer recent => hide it for strcmp "DX de" */
		    thisline[0] = 'd';
		}

	    }
	}
    }
    /* ---------------------sort the arrays ----------------------------------
     */
    changeflg = 1;

    while ((changeflg == 1) && (cluster == MAP)) {	//  sort the spots

	changeflg = 0;

	for (j = 0; j < i - 1; j++) {

	    strcpy(tmp1, bandmap[j]);
	    strcpy(tmp2, bandmap[j + 1]);

	    if ((tmp1 == NULL) || (tmp2 == NULL))
		break;

	    /* tb 30nov10 anstatt atof zu nutzen können wir spot_freq Werte 
	     * vergleichen
	     */
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
    }

    //------------------end sort ---------------------------------------------


    if (cluster == SPOTS)
	linepos = (i < 8 ? 0 : i - 8);
    else
	linepos = 0;

    if (cluster == MAP)
	linepos = bandmap_pos;

    jj = 0;

    /* prune markerfile by opening it for write */
    if (xplanet > 0 && nofile == 0) {
	if ((fp = fopen(markerfile, "w")) == NULL) {
	    nofile = 1;			/* remember: no write possible */
	    mvprintw(24, 0, "Opening marker file not possible.\n");
	    refresh();
	} else
	    fclose(fp);
    }

    for (j = linepos; j < linepos + 8; j++) {

	if (bandmap[j] != NULL) {
	    strncpy(spotline, bandmap[j] + 17, 22);	// freq and call
	    spotline[22] = '\0';
	    strncpy(spottime, bandmap[j] + 70, 5);	// time
	    spottime[5] = '\0';
	    strcat(spotline, spottime);

	    strncpy(callcopy, bandmap[j] + 26, 16);	// call
	    for (m = 0; m < 16; m++) {
		if (callcopy[m] == ' ') {
		    callcopy[m] = '\0';
		    break;
		}	/* use strcspn? */
	    }

	    x = getctynr(callcopy);		// CTY of station

	    y = searchcallarray(callcopy);	// lookup index of call in
	    					// callarray (if already worked)

	    worked = 0;
	    dupe = 1;

	    if (cluster == MAP)
		thisband = bandinx;

	    /* check if country was already worked on this band */
	    if ((countries[x] & inxes[thisband]) != 0)
		worked = 1;	/* no new country/multi */

	    /* check if already worked on these band */
	    if (y != -1) {	/*  found */
		if ((call_band[y] & inxes[thisband]) == 0)
		    dupe = 0;	/* not worked on this band yet */
	    }

	    if (inxes[thisband] == 0) {	/* WARC band */
		worked = 1;		/* show as not needed */
		dupe = 0;		/* station new, but no country/multi */
	    }

	    if (x != 0 && xplanet > 0 && nofile == 0) {

		if ((fp = fopen(markerfile, "a")) == NULL) {
		    mvprintw(24, 0, "Opening markerfile not possible.\n");
		}

		/* show no callsign if MARKERDOTS */
		if (xplanet == 2)
		    callcopy[0]='\0';

		dx = dxcc_by_index(x);
		lon = (int)(dx -> lon);
		lat = (int)(dx -> lat) * -1;
		sprintf(marker_out, "%4d   %4d   \"%s\"   color=", 
			lon, lat, callcopy);

		if (spot_age[j] > 15 && cluster != SPOTS)
		    strcat(marker_out, "Green\n");
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
				strcat(marker_out, "Yellow\n");
				worked = 0;
			    } else
				strcat(marker_out, "White\n");
			} else {
			    if (spot_freq[j] < 3500.0)
				strcat(marker_out, "Red\n");
			    if (spot_freq[j] >= 3500.0
				&& spot_freq[j] <= 4000.0)
				strcat(marker_out, "Magenta\n");
			    if (spot_freq[j] >= 7000.0
				&& spot_freq[j] <= 7300.0)
				strcat(marker_out, "Yellow\n");
			    if (spot_freq[j] >= 14000.0
				&& spot_freq[j] <= 14350.0)
				strcat(marker_out, "Blue\n");
			    if (spot_freq[j] >= 21000.0
				&& spot_freq[j] <= 21450.0)
				strcat(marker_out, "White\n");
			    if (spot_freq[j] >= 28000.0
				&& spot_freq[j] <= 29000.0)
				strcat(marker_out, "Green\n");
			    if (iswarc == 1)
				strcat(marker_out, "Cyan\n");

			}
		    } else {
//                                              iswarc = 0;
			strcat(marker_out, "Cyan\n");
		    }
		}
		fputs(marker_out, fp);

		fclose(fp);
	    }

	}
    }

    /* append last dx cluster message to markerfile; will be shown at bottom */
    if (xplanet == 1 && nofile == 0) {

	xplanetmsg[0] = '\0';
	strcat(xplanetmsg, " -82 -120 ");
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


    bandmap_show();

    refresh();

    return (i);			
    //--------------------------- the end  ------------------
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
