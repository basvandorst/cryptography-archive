
# pragma ident "@(#)Makefile	1.14 96/01/30 Sun Microsystems"
#

include ../../Makefile.defs

BNDIR		= ../../bnlib
BNLIB		= $(BNDIR)/libbn.a
CERTLIB		= ../lib/libcert.a

LIBS		= $(CERTLIB) $(BNLIB) 

CERTINC		= -I../lib
BNINC		= -I$(BNDIR)

C_DEFS          = -D$(CLASS) $(CRYPTOINC) $(CERTINC) $(BNINC)
CFLAGS		=  $(OS) $(C_DEFS) $(D_TEMPLATES) $(OPTIM) $(U_DEBUG)

SCC		= $(CC) $(OS) $(U_DEBUG)

all:	X509toHashCert	

clean:
	@$(RM) -rf *.o X509toHashCert
	@-$(SCCSCLEAN)

cleanall:
	@-$(SCCSCLEAN)

# SOLARIS2 version

X509toHashCert.o: X509toHashCert.C
	$(C++) $(CFLAGS) -c X509toHashCert.C -o $@	

X509toHashCert: X509toHashCert.o $(LIBS)
	$(C++) $(CFLAGS) -o $@ X509toHashCert.o \
		$(LIBS) $(CLASSLIB)

