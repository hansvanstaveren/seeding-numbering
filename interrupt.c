#include <signal.h>

#undef TESTING

int interrupted;

static void catchint() {

	interrupted=1;
}

void setup_catchint() {

	signal(SIGINT, catchint);
}


#ifdef TESTING
main() {

	setup_catchint();
	while (!interrupted)
		sleep(1);
	printf("Must have had INT\n");
}
#endif
