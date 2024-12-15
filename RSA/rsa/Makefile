SHELL=/bin/sh

#CFLAGS= -O +Aa
CFLAGS= -O
LDFLAGS=

all:	genprim genrsa rsa

genprim:	genprim.o rnd.o prim.o nio.o arith.o
	$(CC) $(LDFLAGS) -o genprim genprim.o rnd.o nio.o prim.o arith.o

genrsa:		genrsa.o rnd.o prim.o nio.o arith.o
	$(CC) $(LDFLAGS) -o genrsa genrsa.o rnd.o nio.o prim.o arith.o

rsa:		rsa.o nio.o arith.o
	$(CC) $(LDFLAGS) -o rsa rsa.o nio.o arith.o
	ln rsa rsaencode
	ln rsa rsadecode

rsa.o genrsa.o genprim.o nio.o prim.o arith.o:	conf.h
rsa.o genrsa.o genprim.o nio.o prim.o arith.o:	arith.h
rsa.o genrsa.o genprim.o nio.o:	nio.h
genrsa.o genprim.o prim.o:	prim.h
genrsa.o genprim.o rnd.o:	rnd.h

clean:
	rm -f *.bak *.ba *~ \#* core *.o

clobber: clean
	rm -f genrsa genprim rsa rsadecode rsaencode
