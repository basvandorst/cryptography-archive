#
# SSLeay/crypto/env/Makefile
#

DIR=	evp
TOP=	../..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=envelope
ERRC=evp_err
GENERAL=Makefile
TEST=
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC= encode.c digest.c evp_enc.c evp_key.c \
	e_ecb_d.c e_cbc_d.c e_cfb_d.c e_ofb_d.c \
	e_ecb_i.c e_cbc_i.c e_cfb_i.c e_ofb_i.c \
	e_ecb_3d.c e_cbc_3d.c e_rc4.c e_names.c \
	e_cfb_3d.c e_ofb_3d.c \
	m_md2.c m_md5.c m_sha.c m_sha1.c m_names.c \
	p_open.c p_seal.c p_sign.c p_verify.c \
	$(ERRC).c

LIBOBJ=	encode.o digest.o evp_enc.o evp_key.o \
	e_ecb_d.o e_cbc_d.o e_cfb_d.o e_ofb_d.o \
	e_ecb_i.o e_cbc_i.o e_cfb_i.o e_ofb_i.o \
	e_ecb_3d.o e_cbc_3d.o e_rc4.o e_names.o \
	e_cfb_3d.o e_ofb_3d.o \
	m_md2.o m_md5.o m_sha.o m_sha1.o m_names.o \
	p_open.o p_seal.o p_sign.o p_verify.o \
	$(ERRC).o

SRC= $(LIBSRC)

EXHEADER= envelope.h
HEADER=	$(EXHEADER)

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
	$(MAKEDEPEND) $(INCLUDES) $(LIBSRC)

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
	perl ../error/err_genc.pl $(ERR).h $(ERRC).c >$(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.
