#include <time.h>

usertime() {
	clock_t clocktime;

	clocktime = clock();
	return clocktime/CLOCKS_PER_SEC+1;
}
