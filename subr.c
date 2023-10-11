#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Error somewhere, complain and quit
 */
void
error(s) char *s; {

	fprintf(stderr, "Error: %s\n", s);
	exit(-1);
}

/*
 * Copy a string into dynamic storage
 */
char *
string_copy(s) char *s; {
	char *marea;

	marea=calloc(strlen(s)+1, sizeof(char));
	return strcpy(marea, s);
}

/*
 * For reasonableness checking of input
 */
void
bound_check(var, description, lolim, hilim) int var, lolim, hilim; char *description; {
	char erbuf[512];

	if (var < lolim || var > hilim) {
		sprintf(erbuf, "%s(%d) should be between %d and %d", description, var, lolim, hilim);
		error(erbuf);
	}
}

int read_number() {
	int n;

	if(scanf("%d", &n) != 1)
		error("read_number");
	return n;
}

void read_line() {

	scanf("\n");
}
