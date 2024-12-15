CC = gcc
CFLAGS = -O2 -ansi -I/usr/local/include/pari -DLONG_IS_32BIT -DULONG_NOT_DEFINED

all:    mattrans

mattrans:	mattrans.c
	$(CC) $(CFLAGS) -o mattrans mattrans.c -lpari -lm

