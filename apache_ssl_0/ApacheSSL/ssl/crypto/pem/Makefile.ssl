#
# SSLeay/crypto/pem/Makefile
#

DIR=	pem
TOP=	../..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=pem
ERRC=pem_err
GENERAL=Makefile
TEST=
APPS=

CTX_SIZE= ctx_size

LIB=$(TOP)/libcrypto.a
LIBSRC= pem_sign.c pem_seal.c pem_info.c pem_lib.c $(ERRC).c

LIBOBJ=	pem_sign.o pem_seal.o pem_info.o pem_lib.o $(ERRC).o

SRC= $(LIBSRC)

EXHEADER= pem.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	pem.h lib

pem.h: $(CTX_SIZE)
	sed "s/^#define EVP_CIPHER_CTX_SIZE.*/#define EVP_CIPHER_CTX_SIZE `./$(CTX_SIZE) EVP_CIPHER_CTX`/" < pem.h | \
	sed "s/^#define EVP_CIPHER_SIZE.*/#define EVP_CIPHER_SIZE `./$(CTX_SIZE) EVP_CIPHER`/" | \
	sed "s/^#define EVP_MD_CTX_SIZE.*/#define EVP_MD_CTX_SIZE `./$(CTX_SIZE) EVP_MD_CTX`/" | \
	sed "s/^#define EVP_MD_SIZE.*/#define EVP_MD_SIZE `./$(CTX_SIZE) EVP_MD`/" | \
	sed "s/^#define EVP_ENCODE_CTX_SIZE.*/#define EVP_ENCODE_CTX_SIZE `./$(CTX_SIZE) EVP_ENCODE_CTX`/" | \
	sed "s/^#define EVP_MAX_MD_SIZE.*/#define EVP_MAX_MD_SIZE `./$(CTX_SIZE) EVP_MAX_MD`/" >pem.h.new 
	mv -f pem.h.new pem.h

$(CTX_SIZE): $(CTX_SIZE).o
	$(CC) $(CFLAGS) -o $(CTX_SIZE) $(CTX_SIZE).o

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
	$(MAKEDEPEND) $(INCLUDES) $(CTX_SIZE).c $(LIBSRC)

dclean:
	sed -e '/^# DO NOT DELETE THIS LINE/ q' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	/bin/rm -f $(CTX_SIZE) *.o *.obj lib tags core .nfs* *.old *.bak fluff

errors:
	sed -e "/ERROR CODES/ r $(ERR).err" -e "/ERROR CODES/ q" \
		<$(ERR).h >$(ERR).new
	mv $(ERR).h $(ERR).old
	mv $(ERR).new $(ERR).h
	perl ../error/err_genc.pl $(ERR).h $(ERRC).c >$(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.
