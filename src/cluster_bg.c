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


