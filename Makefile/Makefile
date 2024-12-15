# Simple Makefile for compiling MD4
#
CC=	gcc
CFLAGS= -O2 -DMD=4 -I../mddriver -I.

all:	md4

mddriver.o: 
	$(CC) $(CFLAGS) -c ../mddriver/mddriver.c 

md4:	md4c.o mddriver.o
	$(CC) $(CFLAGS) -o md4 md4c.c mddriver.o
	make clean

modified.o:
	$(CC) $(CFLAGS) -c ../mddriver/modified.c

modified: md4c.o modified.o
	$(CC) $(CFLAGS) -o md4 md4c.c modified.o  
	make clean
clean:
	rm -f *.o
