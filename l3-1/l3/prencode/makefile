CC=gcc
CFLAGS= -O # -I/usr/include
INSTDIR=/usr/local/bin

PRCODE_ROUTINES=tprencod.o prencode.o usage.o getopt.o

all: prencode

prencode: $(PRCODE_ROUTINES)
	$(CC) -o prencode $(CFLAGS) $(PRCODE_ROUTINES)

install: prencode
	install -c -m 0755 prencode $(INSTDIR)

clean: 
	rm -f *.o prencode core
