#include "cluster_bg.h"


int cluster_bg(int cluster_arg)
{

    extern int announcefilter;

    if (cluster_arg == NOCLUSTER)
	return (1);

    if (cluster_arg != CLUSTER)
    {
	announcefilter = FILTER_DX;

    }

    getclusterinfo();

    return (0);

}

int getclusterinfo(void)
{

    extern char spot_ptr[MAX_SPOTS][82];
    extern int ptr;
    extern int spotarray[];
    extern int announcefilter;
    extern int cluster;
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


