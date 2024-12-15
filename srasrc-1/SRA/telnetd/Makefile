
PROG=	telnetd

CFLAGS= -DTERMCAP -DKLUDGELINEMODE -DUSE_TERMIO -DAUTHENTICATE -DENCRYPT \
	-DKRB4 -DDIAGNOSTICS -DFILIO_H -I../lib -I../lib/libbsd/include

LIBS= 	../lib/libtelnet/libtelnet.a ../lib/kerberosIV/krb/libkrb.a \
	../lib/libbsd/libbsd.a ../lib/libpk/libpk.a  \
	../lib/gmp-1.2/libmp.a ../lib/libdes/des/libdes.a \
	../lib/libutil/libutil.a

OBJS=	authenc.o global.o slc.o state.o sys_term.o telnetd.o \
	termstat.o utility.o 

$(PROG):	$(OBJS) $(LIBS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS) $(LDADD) -ltermcap
	cp telnetd ../bin
clean:
	rm -f *.o $(PROG)
