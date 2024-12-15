CC = gcc
CFLAGS = -O2 -ansi -I/usr/local/include/pari -DULONG_NOT_DEFINED -DLONG_IS_32BIT

all:    mattrans

mattrans:	mattrans.c
	$(CC) $(CFLAGS) -o mattrans mattrans.c -lpari -lm

