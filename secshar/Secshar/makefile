CC = gcc
objects = main.o GF257.o idea.o random.o util.o

secshar : $(objects)
	   $(CC) -o secshar $(objects) -lm

main.o: main.c crypt.h
	$(CC) -g -c main.c

GF257.o: GF257.c crypt.h
	$(CC) -g -c GF257.c

idea.o: idea.c crypt.h
	$(CC) -g -c idea.c

random.o: random.c crypt.h
	$(CC) -g -c random.c

util.o: util.c crypt.h
	$(CC) -g -c util.c

