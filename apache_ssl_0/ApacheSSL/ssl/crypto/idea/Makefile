#
# SSLeay/crypto/idea/Makefile
#

DIR=	idea
TOP=	../..
CC=	cc
INCLUDES=
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=ideatest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=i_cbc.c i_cfb64.c i_ofb64.c i_ecb.c i_skey.c
LIBOBJ=i_cbc.o i_cfb64.o i_ofb64.o i_ecb.o i_skey.o

SRC= $(LIBSRC)

EXHEADER= idea.h
HEADER=	idea_locl.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	ar r $(LIB) $(LIBOBJ)
	$(TOP)/util/ranlib.sh $(LIB)
	@touch lib

links:
	/bin/rm -f Makefile
	$(TOP)/util/point.sh Makefile.ssl Makefile
	$(TOP)/util/mklink.sh ../../include $(EXHEADER)
	$(TOP)/util/mklink.sh ../../test $(TEST)
	$(TOP)/util/mklink.sh ../../apps $(APPS)

install:
	@for i in $(EXHEADER) ; \
	do  \
	(cp $$i $(INSTALLTOP)/include/$$i; \
	chmod 644 $(INSTALLTOP)/include/$$i ) \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

depend:
	$(MAKEDEPEND) $(INCLUDES) $(PROGS) $(LIBSRC)

dclean:
	sed -e '/^# DO NOT DELETE THIS LINE/ q' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	/bin/rm -f *.o *.obj lib tags core .nfs* *.old *.bak fluff

errors:

# DO NOT DELETE THIS LINE -- make depend depends on it.
