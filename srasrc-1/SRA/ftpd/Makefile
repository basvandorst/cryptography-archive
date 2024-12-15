#	@(#)Makefile	5.15 (Berkeley) 7/1/90

PROG=	ftpd
OBJS=	ftpd.o ftpcmd.o glob.o logwtmp.o popen.o vers.o
MAN8=	ftpd.0
CLEANFILES= ftpcmd.c y.tab.c

CFLAGS= -I../lib/libpk 
LIBS=   ../lib/libbsd/libbsd.a ../lib/libpk/libpk.a \
	../lib/libdes/des/libdes.a ../lib/gmp-1.2/libmp.a


$(PROG):	$(OBJS) $(LIBS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS) $(LDADD)
	cp ftpd ../bin

clean:
	rm -f *.o $(PROG) $(CLEANFILES)

ftpcmd.c:	ftpcmd.y
	if [ -f /usr/bin/yacc ]; then \
		/usr/bin/yacc ftpcmd.y; \
	else \
	if [ -f /usr/ccs/bin/yacc ]; then \
		/usr/ccs/bin/yacc ftpcmd.y; \
	else \
		bison -y ftpcmd.y; \
	fi; \
	fi;
	mv y.tab.c ftpcmd.c
ftpcmd.o:	ftpcmd.c
	$(CC) -c $(CFLAGS) ftpcmd.c

