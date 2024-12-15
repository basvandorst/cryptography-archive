#
# SSLeay/rsaref/Makefile
#

DIR=	rsaref
TOP=	..
CC=	cc
INCLUDES= -I../crypto -I../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=rsaref
ERRC=rsar_err
GENERAL=Makefile
TEST=
APPS=

LIB=$(TOP)/libRSAglue.a
LIBSRC=	rsaref.c $(ERRC).c
LIBOBJ= rsaref.o $(ERRC).o

SRC= $(LIBSRC)

EXHEADER=
HEADER=	$(EXHEADER) rsaref.h

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ..; $(MAKE) DIRS=rsaref all)

all:	lib

lib:	$(LIBOBJ)
	ar r $(LIB) $(LIBOBJ)
	$(TOP)/util/ranlib.sh $(LIB)
	@touch lib

links:
	/bin/rm -f Makefile
	$(TOP)/util/point.sh Makefile.ssl Makefile
	$(TOP)/util/mklink.sh ../include $(EXHEADER)
	$(TOP)/util/mklink.sh ../test $(TEST)
	$(TOP)/util/mklink.sh ../apps $(APPS)

install:

#	@for i in $(EXHEADER) ; \
#	do  \
#	(cp $$i $(INSTALLTOP)/include/$$i; \
#	chmod 644 $(INSTALLTOP)/include/$$i ) \
#	done;

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
	sed -e "/ERROR CODES/ r $(ERR).err" -e "/ERROR CODES/ q" \
		<$(ERR).h >$(ERR).new
	mv $(ERR).h $(ERR).old
	mv $(ERR).new $(ERR).h
	perl ../crypto/error/err_genc.pl $(ERR).h $(ERRC).c >$(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.
