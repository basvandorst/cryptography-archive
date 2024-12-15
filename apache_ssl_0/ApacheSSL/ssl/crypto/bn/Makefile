#
# SSLeay/crypto/bn/Makefile
#

DIR=	bn
TOP=	../..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl
BN_MULW=	bn_mulw.o

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=bn
ERRC=bn_err
GENERAL=Makefile
TEST=bntest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=	bn_add.c bn_div.c bn_exp.c bn_lib.c bn_mod.c bn_mul.c \
	bn_print.c bn_rand.c bn_shift.c bn_sub.c bn_word.c \
	bn_gcd.c bn_prime.c $(ERRC).c bn_sqr.c bn_mulw.c

LIBOBJ=	bn_add.o bn_div.o bn_exp.o bn_lib.o bn_mod.o bn_mul.o \
	bn_print.o bn_rand.o bn_shift.o bn_sub.o bn_word.o \
	bn_gcd.o bn_prime.o $(ERRC).o bn_sqr.o $(BN_MULW)


SRC= $(LIBSRC)

EXHEADER= bn.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

knuth: bn_knuth.c
	cc -pg -I.. -I../../include bn_knuth.c -o knuth $(LIB) #../../../libefence.a

knuth.fast: bn_knuth.c
	cc -pg -fast -I.. -I../../include bn_knuth.c -o knuth $(LIB) #../../../libefence.a


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
	/bin/rm -f *.o */*.o *.obj lib tags core .nfs* *.old *.bak fluff

errors:
	sed -e "/ERROR CODES/ r $(ERR).err" -e "/ERROR CODES/ q" \
		<$(ERR).h >$(ERR).new
	mv $(ERR).h $(ERR).old
	mv $(ERR).new $(ERR).h
	perl ../error/err_genc.pl $(ERR).h $(ERRC).c >$(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.
