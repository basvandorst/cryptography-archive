# Makefile for RIPEM utilities in ripem/util
CC=gcc
CFLAGS= -O

PRCODE_ROUTINES=tprencod.o prencode.o usage.o getopt.o

all: prencode

prencode: $(PRCODE_ROUTINES)
	$(CC) -o prencode $(CFLAGS) $(PRCODE_ROUTINES)

clean: 
	rm -f *.o prencode core
