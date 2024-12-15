LIBS =  md5c.o
CCF = -O -c

PROGS = ranH ranM

all: $(PROGS)

ranH: ranH.o $(LIBS)
	gcc -o $@ $@.o $(LIBS)

ranH.o: ranH.c md5.h
	gcc $(CCF) ranH.c

ranM: ranM.o rnd.o
	gcc -o $@ $@.o rnd.o

rnd.o: rnd.c
	gcc $(CCF) rnd.c

ranM.o: ranM.c 
	gcc $(CCF) ranM.c

clean: 
	rm -f ranH.o rnd.o ranM.o $(PROGS) *~ core
