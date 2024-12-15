#
# SSLeay/crypto/Makefile
#

DIR=	crypto
TOP=	..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG= -O
INSTALLTOP=/usr/local/ssl
PEX_LIB=
EX_LIB=

MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)
LIBS=

SDIRS=	md sha \
	des rc4 idea \
	bn rsa dh \
	buffer stack lhash rand error objects \
	evp pem pkcs7 x509 \
	asn1 meth conf txt_db

GENERAL=Makefile README

LIB=
LIBSRC=	
SRC= $(LIBSRC)

EXHEADER=
HEADER=	crypto.h location.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	@(cd ..; $(MAKE) DIRS=$(DIR) all)

all:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making all in $$i..."; \
	$(MAKE) CC=$(CC) INCLUDES='${INCLUDES}' CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' BN_MULW='${BN_MULW}' all ); \
	done;

links:
	/bin/rm -f Makefile
	$(TOP)/util/point.sh Makefile.ssl Makefile
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making links in $$i..."; \
	$(MAKE) links ); \
	done;

lib:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making lib in $$i..."; \
	$(MAKE) CC=$(CC) CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' lib ); \
	done;

tests:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making tests in $$i..."; \
	$(MAKE) CC=$(CC) CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' tests ); \
	done;

install:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making install in $$i..."; \
	$(MAKE) CC=$(CC) CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' install ); \
	done;

lint:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making lint in $$i..."; \
	$(MAKE) CC=$(CC) CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' lint ); \
	done;

depend:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making depend in $$i..."; \
	$(MAKE) MAKEFILE='${MAKEFILE}' INCLUDES='${INCLUDES}' MAKEDEPEND='${MAKEDEPEND}' depend ); \
	done;

clean:
	/bin/rm -f *.bak
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making clean in $$i..."; \
	$(MAKE) CC=$(CC) CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' clean ); \
	done;

dclean:
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making dclean in $$i..."; \
	$(MAKE) CC=$(CC) CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' dclean ); \
	done;

errors:
	perl ./error/err_code.pl */*.c ../ssl/*.c ../rsaref/*.c
	@for i in $(SDIRS) ;\
	do \
	(cd $$i; echo "making errors in $$i..."; \
	$(MAKE) errors ); \
	done;

