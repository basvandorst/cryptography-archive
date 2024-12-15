LIBS =  rsaref.a
CCF = -O -c

PROGS = ranH ranM ranD 

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

ranD: ranD.o $(LIBS)
	gcc -o $@ $@.o $(LIBS)

ranD.o: ranD.c des.h
	gcc $(CCF) ranD.c

clean: 
	rm -f *.o $(PROGS) *~ core
