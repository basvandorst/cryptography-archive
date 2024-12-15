
#pragma ident "@(#)Makefile	1.7 96/01/30"

include ../../Makefile.defs

BIGINTDIR       = ../../bnlib
BNLIB           = $(BIGINTDIR)/libbn.a

CERTSRCDIR      = ../lib
CERTLIB         = $(CERTSRCDIR)/libcert.a

INCS		= -I$(BIGINTDIR) -I$(CERTSRCDIR)

LIBS            = $(CERTLIB) $(CRYPTOLIB) $(CLASSLIB) $(BNLIB)

CFLAGS		= $(U_DEBUG) $(INCS) $(OS) -D$(CLASS) $(D_TEMPLATES) $(OPTIM)

all: print_cert md5

clean:
	@-rm -rf *.o md5 print_cert
	@-$(SCCSCLEAN)

cleanall:
	@-rm -rf *bin.*
	@-$(SCCSCLEAN)


md5:	md5prog.o md5.o
	$(CC) $(OPTIM) -I$(CERTSRCDIR) -o $@ md5prog.o md5.o

md5prog.o:	md5prog.c
	$(CC) $(OPTIM) -c -I$(CERTSRCDIR) -o $@ md5prog.c

md5.o:	$(CERTSRCDIR)/md5.c
	$(CC) $(OPTIM) -c -I$(CERTSRCDIR) -o $@ $(CERTSRCDIR)/md5.c

print_cert:	print_cert.o $(CERTLIB)
	$(C++) $(CFLAGS) -o $@ print_cert.o $(LIBS)

print_cert.o:	print_cert.C
	$(C++) -c $(CFLAGS) -o $@ print_cert.C
