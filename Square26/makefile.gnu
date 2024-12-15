# Makefile for Square 2.5 (assuming GNU-make)
#
# This makefile is provided as an example.
#
# Note: On some architectures endianness is detected by the
#       compiler directives in the code;
#       safer is to specify it explicitly

CC = gcc

CFLAGS =  -Wall -O3 -fomit-frame-pointer -ffast-math -s \
	-funroll-loops -DTEST_SQUARE=1 -DMASKED_BYTE_EXTRACTION=1 \
	-DLITTLE_ENDIAN=1

sqtest:	sqtest.o square.o sqecb.o sqcbc.o sqcts.o sqcfb.o sqofb.o sqhash.o

sqecb:	sqecb.o square.o

sqcbc:	sqcbc.o square.o

sqcts:	sqcts.o square.o

sqcfb:	sqcfb.o square.o

sqofb:	sqofb.o square.o

sqhash:	sqhash.o square.o

square.o: square.c square.h square.tab
	$(CC) square.c $(CFLAGS) -c

square.tab: sqgen
	sqgen

clean:
	rm *.o square.tab sqgen
