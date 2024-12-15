#
# Makefile for telnet - hacked about for SSL purposes
#


PROG=	telnet

CFLAGS= -DKLUDGELINEMODE -DUSE_TERMIO -DAUTHENTICATE -DUSE_SSL \
	-DFILIO_H \
	-I../lib/libbsd/include -I../lib \
	-I$(SSLTOP)/include


LIBS=   ../lib/libtelnet/libtelnet.a \
	../lib/libbsd/libbsd.a \
        $(SSLTOP)/lib/libssl.a \
	$(SSLTOP)/lib/libcrypto.a 
#	$(SSLTOP)/lib/libdes.a

OBJS=	authenc.o commands.o main.o network.o ring.o sys_bsd.o telnet.o \
	terminal.o tn3270.o utilities.o

$(PROG):	$(OBJS) $(LIBS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS) $(LDADD) 
	cp telnet ../bin

clean:
	rm -f *.o $(PROG) tags

