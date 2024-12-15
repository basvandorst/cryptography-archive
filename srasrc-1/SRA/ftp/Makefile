#	@(#)Makefile	5.11 (Berkeley) 5/11/90

LIBS=   ../lib/libbsd/libbsd.a ../lib/libpk/libpk.a \
	../lib/libdes/des/libdes.a ../lib/gmp-1.2/libmp.a

PROG=	ftp
OBJS=	cmds.o cmdtab.o ftp.o glob.o main.o ruserpass.o domacro.o

CFLAGS= -I../lib/libpk 

$(PROG):	$(OBJS) $(LIBS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS) $(LDADD)
	cp ftp ../bin

clean:
	rm -f *.o $(PROG)
