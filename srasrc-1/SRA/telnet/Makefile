
PROG=	telnet

CFLAGS= -DTERMCAP -DKLUDGELINEMODE -DUSE_TERMIO -DAUTHENTICATE -DENCRYPT \
	-DKRB4 -DFILIO_H -I../lib/libbsd/include -I../lib 

LIBS= ../lib/libtelnet/libtelnet.a ../lib/kerberosIV/krb/libkrb.a \
	../lib/libbsd/libbsd.a ../lib/libpk/libpk.a \
	../lib/gmp-1.2/libmp.a ../lib/libdes/des/libdes.a

OBJS=	authenc.o commands.o main.o network.o ring.o sys_bsd.o telnet.o \
	terminal.o tn3270.o utilities.o

$(PROG):	$(OBJS) $(LIBS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS) $(LDADD) -ltermcap
	cp telnet ../bin

clean:
	rm -f *.o $(PROG)
