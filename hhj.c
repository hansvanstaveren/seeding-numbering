#include <stdio.h>
#include "subr.h"

main() {
	int nround, nmeetings, ngroups, pairsA, pairsB;
	int *mar;
	int i,m,r;

	nround = read_number();
	nmeetings = read_number();
	ngroups = read_number();
	bound_check(ngroups, "number of groups", 2, 2);
	pairsA = read_number();
	pairsB = read_number();
	/*
	 * The format is the same as numberlines on imput
	 * Then HHJ will generate all meetings from the first round
	 * The first groups moves (2 follows 1 etc), the second stays put
	 */
	mar = (int *) calloc(2*nmeetings,sizeof(int));
	for (m=1; m <= nmeetings; m++) {
		mar[2*m-2] = read_number();
		mar[2*m-1] = read_number();
		bound_check(mar[2*m-2], "pair number in meeting data", 1, pairsA+pairsB);
		bound_check(mar[2*m-1], "pair number in meeting data", 1, pairsA+pairsB);
	}
	printf("%d\n%d\n%d\n%d\n", nround, nmeetings, 1, pairsA+pairsB);
	for (r=0; r<nround; r++) {
		for (i=0; i<2*nmeetings; i++) {
			printf("%d ", mar[i] > pairsA ? mar[i] : (mar[i]+r-1)%pairsA+1);
		}
		printf("\n");
	}
	return 0;
}
