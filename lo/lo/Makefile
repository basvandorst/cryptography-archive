CFLAGS=-O2 -Wall -I.
LDFLAGS=-s -N

all:		losetup

losetup:	losetup.o

cvdes:		cvdes.c

clean:
	cd module; make clean
	rm -f *.o *~ losetup cvdes
