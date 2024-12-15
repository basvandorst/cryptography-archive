# I used gcc to compile the program
# change gcc to the name of the compiler you use
CC = gcc
objects = rnd.o rc4.o

rnd : $(objects)
	   $(CC) -o rnd $(objects) -lm

rnd.o: rnd.c rc4.h
	$(CC) -g -c rnd.c

rc4.o: rc4.c rc4.h
	$(CC) -g -c rc4.c

