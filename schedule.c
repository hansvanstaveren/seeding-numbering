#include <stdlib.h>
#include "subr.h"


/*
 * Modifications for more than two groups:
 * change sizeA and sizeB to "number of groups, array of groupsizes"
 * change args of read_schedule
 */

static
struct schedule {
	int sch_nrounds;
	int sch_nmeetings;
	int sch_ngroups;
	int *sch_groupsize;
} sch_info;

static int **meet;	/* pairs meet in round meet[p1][p2] */


void read_schedule(ngroups, groupsizes) int *ngroups, **groupsizes; {
	int round, meeting;
	int pair1, pair2;
	int i;
	int size, totalsize;
	
	sch_info.sch_nrounds = read_number();
	sch_info.sch_nmeetings = read_number();
	sch_info.sch_ngroups = read_number();
	sch_info.sch_groupsize = calloc(sch_info.sch_ngroups, sizeof(int));
	totalsize = 0;
	for (i=0; i<sch_info.sch_ngroups; i++) {
		size = read_number();
		sch_info.sch_groupsize[i] = size;
		totalsize += size;
	}
	meet = (int **) calloc(totalsize, sizeof(int *));
	for (i=0; i<totalsize; i++) {
		meet[i] = (int *) calloc(totalsize, sizeof(int));
	}
	*ngroups = sch_info.sch_ngroups;
	*groupsizes = sch_info.sch_groupsize;
	for (round=1; round <= sch_info.sch_nrounds; round++) {
		for (meeting=1; meeting <= sch_info.sch_nmeetings; meeting++) {
			pair1 = read_number();
			bound_check(pair1, "pair number in meeting data",
				1, totalsize);
			pair2 = read_number();
			bound_check(pair2, "pair number in meeting data",
				1, totalsize);
			meet[pair1-1][pair2-1] = meet[pair2-1][pair1-1] = round;
		}
		read_line();
	}
}

int play_in_round(int pair1, int pair2) {

	return meet[pair1-1][pair2-1];
}
