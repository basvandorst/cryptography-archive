#
# SSLeay/crypto/asn1/Makefile
#

DIR=	asn1
TOP=	../..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=asn1
ERRC=asn1_err
GENERAL=Makefile README
TEST=
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=	a_object.c a_bitstr.c a_utctm.c a_int.c a_octet.c a_print.c \
	a_type.c a_set.c a_dup.c a_d2i_fp.c a_i2d_fp.c \
	a_sign.c a_verify.c \
	x_algor.c x_val.c x_pubkey.c x_sig.c x_req.c x_attrib.c \
	x_name.c x_cinf.c x_x509.c x_crl.c x_info.c \
	d2i_r_pr.c i2d_r_pr.c d2i_r_pu.c \
	t_req.c t_x509.c t_rsa.c t_dh.c \
	p7_i_s.c p7_signi.c p7_signd.c p7_recip.c p7_enc_c.c p7_evp.c \
	p7_dgst.c p7_s_e.c p7_enc.c p7_lib.c \
	f_int.c i2d_dh.c d2i_dh.c n_pkey.c \
	asn1_par.c asn1_lib.c $(ERRC).c
LIBOBJ= a_object.o a_bitstr.o a_utctm.o a_int.o a_octet.o a_print.o \
	a_type.o a_set.o  a_dup.o a_d2i_fp.o a_i2d_fp.o \
	a_sign.o a_verify.o \
	x_algor.o x_val.o x_pubkey.o x_sig.o x_req.o x_attrib.o \
	x_name.o x_cinf.o x_x509.o x_crl.o x_info.o \
	d2i_r_pr.o i2d_r_pr.o d2i_r_pu.o i2d_r_pu.o \
	t_req.o t_x509.o t_rsa.o t_dh.o \
	p7_i_s.o p7_signi.o p7_signd.o p7_recip.o p7_enc_c.o p7_evp.o \
	p7_dgst.o p7_s_e.o p7_enc.o p7_lib.o \
	f_int.o i2d_dh.o d2i_dh.o n_pkey.o \
	asn1_par.o asn1_lib.o $(ERRC).o

SRC= $(LIBSRC)

EXHEADER=  asn1.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

test:	test.c
	cc -g -I../../include -c test.c
	cc -g -I../../include -o test test.o -L../.. -lcrypto

pk:	pk.c
	cc -g -I../../include -c pk.c
	cc -g -I../../include -o pk pk.o -L../.. -lcrypto

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
	sed -e "/ERROR CODES/ r $(ERR).err" -e "/ERROR CODES/ q" \
		<$(ERR).h >$(ERR).new
	mv $(ERR).h $(ERR).old
	mv $(ERR).new $(ERR).h
	perl ../error/err_genc.pl $(ERR).h $(ERRC).c >$(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.
