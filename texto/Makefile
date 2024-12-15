CC = gcc 

# Uncomment this on a solaris machine, or any other machine with uuencode
# that uses the space character instead of the ` character 
#

# DEFINES = -DUU_SPACE

all:	texto

sgi:
	$(MAKE) all CC="cc -cckr"

linux: 
	$(MAKE) all CC="gcc"

texto:	main.o init_words.o oht.o
	$(CC) $(DEFINES) -o texto main.o init_words.o oht.o

main.o:	main.c texto.h
	$(CC) $(DEFINES) -c main.c

init_words.o:	init_words.c texto.h
	$(CC) $(DEFINES) -c init_words.c

oht.o:	oht.c	oht.h
	$(CC) $(DEFINES) -c oht.c

clean:
	rm -f core texto *.o 

dist:
	$(MAKE) clean; tar cvf texto.tar *; gzip texto.tar
