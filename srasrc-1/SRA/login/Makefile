
CFLAGS += -DKERBEROS -DUSE_TERMIO_H 
CFLAGS += -I../lib -I../lib/libbsd/include

PROG=	login

LIBS= ../lib/kerberosIV/krb/libkrb.a ../lib/libdes/des/libdes.a
LIBS += ../lib/libbsd/libbsd.a ../lib/libutil/libutil.a

OBJS= klogin.o login.o

$(PROG):	$(OBJS) $(LIBS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS) $(LDADD)
	cp login ../bin

clean:
	rm -f *.o $(PROG)

