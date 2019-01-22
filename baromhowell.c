#include <stdio.h>
#include "subr.h"

int main(int argc, const char *argv[]) {
	int nround, nmeetings, ngroups;
	int *mar;
	int i,m,r;
	int pairs, movingpairs, lowpair, highpair;

	bound_check(argc, "argument count", 2, 2);
	pairs = atoi(argv[1]);
	if (pairs%2)
		error("pairs should be even");
	nround = pairs-1;
	movingpairs = pairs-1;
	nmeetings = pairs/2;
	ngroups = 1;
	mar = (int *) calloc(2*nmeetings,sizeof(int));
	lowpair=1; highpair=pairs;
	for (m=1; m <= nmeetings; m++) {
		mar[2*m-2] = lowpair;
		mar[2*m-1] = highpair;
		lowpair++; highpair--;
	}
	printf("%d\n%d\n%d\n%d\n", nround, nmeetings, ngroups, pairs);
	for (r=0; r<nround; r++) {
		for (i=0; i<2*nmeetings; i++) {
			printf("%d ", mar[i] > movingpairs ?
				mar[i] : (mar[i]+r-1)%movingpairs+1);
		}
		printf("\n");
	}
	return 0;
}
