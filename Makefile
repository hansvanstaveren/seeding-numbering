CFLAGS=-I.

all:	mitchell hhj numberlines seeding baromhowell

clean:
	rm mitchell mitchell.exe hhj hhj.exe numberlines numberlines.exe seeding seeding.exe baromhowell baromhowell.exe *.o


mitchell:	mitchell.o subr.o
	$(CC) $(CFLAGS) -o mitchell mitchell.o subr.o

hhj:	hhj.o subr.o
	$(CC) $(CFLAGS) -o hhj hhj.o subr.o

baromhowell:	baromhowell.o subr.o
	$(CC) $(CFLAGS) -o baromhowell baromhowell.o subr.o

seeding-old: seeding-old.o subr.o
	$(CC) $(CFLAGS) -o seeding-old seeding-old.o subr.o

seeding: seeding.o subr.o getopt.o
	$(CC) $(CFLAGS) -o seeding seeding.o subr.o getopt.o

seeding.o: seeding.h

numberlines.o: subr.h schedule.h usertime.h interrupt.h

numberlines:	numberlines.o schedule.o subr.o usertime.o interrupt.o
	$(CC) $(CFLAGS) -o numberlines numberlines.o schedule.o subr.o usertime.o interrupt.o
