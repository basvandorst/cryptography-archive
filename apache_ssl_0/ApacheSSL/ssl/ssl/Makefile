#
# SSLeay/ssl/Makefile
#

DIR=	ssl
TOP=	..
CC=	cc
INCLUDES= -I../crypto -I../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=ssl
ERRC=ssl_err
GENERAL=Makefile README
TEST=
APPS=

LIB=$(TOP)/libssl.a
LIBSRC=	ssl_lib.c ssl_err2.c ssl_trc.c ssl_cert.c ssl_conn.c \
	ssl_srvr.c ssl_clnt.c ssl_pkt.c ssl_auth.c ssl_stat.c \
	ssl_rsa.c ssl_null.c ssl_des.c ssl_idea.c ssl_rc4.c \
	ssl_md5.c ssl_sha.c $(ERRC).c
LIBOBJ= ssl_lib.o ssl_err2.o ssl_trc.o ssl_cert.o ssl_conn.o \
	ssl_srvr.o ssl_clnt.o ssl_pkt.o  ssl_auth.o ssl_stat.o \
	ssl_rsa.o ssl_null.o ssl_des.o ssl_idea.o ssl_rc4.o \
	ssl_md5.o ssl_sha.o $(ERRC).o


SRC= $(LIBSRC)

EXHEADER= ssl.h
HEADER=	$(EXHEADER) ssl_locl.h ssl_trc.h \
	ssl_rsa.h ssl_null.h ssl_md5.h ssl_sha.h ssl_des.h ssl_idea.h ssl_rc4.h

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ..; $(MAKE) DIRS=$(DIR) all)

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
	sed -e "/ERROR CODES/ r $(ERR).err" -e "/ERROR CODES/ q" \
		<$(ERR).h >$(ERR).new
	mv $(ERR).h $(ERR).old
	mv $(ERR).new $(ERR).h
	perl ../crypto/error/err_genc.pl $(ERR).h $(ERRC).c >$(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.
