
CC=acc
CFLAGS=-fast

strandomtest: haval.o strandom.o strandomtest.o 
	   ${CC} ${CFLAGS} haval.o strandom.o strandomtest.o -o $@
haval.o strandom.o strandomtest.o: havalapp.h

clean:
	/usr/bin/rm -f *.o strandomtest

