CC=gcc
LIBES= -lgmp_ext -lgmp

install:	sqrt
	echo No install for test code

sqrt:	sqrt.o
	$(CC) $(LDFLAGS) -o sqrt sqrt.o $(LIBES)

clean:
	-rm -f sqrt sqrt.o

relink:
	-rm -f sqrt
	make $(MFLAGS) sqrt
