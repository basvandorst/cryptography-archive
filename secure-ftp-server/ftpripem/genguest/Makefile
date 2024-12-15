CC=gcc
CFLG= -g
DEFS=-DUNIX -DUSEBSD -DUSE_SOCKETS -DSUN -DUNISTD -DDOGETRUSAGE -DPROTOTYPES=1
CFLAGS=$(CFLG) $(DEFS)
GENROUTINES=genguest.o getretb.o getsys.o strutil.o normstr.o echo.o\
   hexbin.o rsaref.a
all: genguest

genguest: $(GENROUTINES)
	$(CC) $(CFLAGS) -o genguest $(GENROUTINES)

clean:
	rm -f *.o genguest

cg:
	cp genguest /usr/local/bin

