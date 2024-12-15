# 
.SUFFIXES: .prt  .vw .1
SRCS = des.c getopt.c 

UNDER_RCS = $(SRCS) des.test des.1

all:	des 

neat:
	rm -f *.o *BAK .emacs* 

clobber: neat
	rm -f des

ci:
	ci -f -u $(UNDER_RCS)

firstci: rcsdir ci
	rcs -q -U $(UNDER_RCS)
	chmod u+w $(UNDER_RCS)
rcsdir:
	mkdir RCS

des: des.o getopt.o
	cc -o des  des.c getopt.o -lcrypt
	strip des

getopt.o: getopt.c
	cc -c getopt.o getopt.c

.1.prt:
	tbl .1 | psroff -man -h 

.1.vw:
	tbl .1 | psroff -man -t | psview -F1.0
