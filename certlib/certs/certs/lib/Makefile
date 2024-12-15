
# pragma ident "@(#)Makefile	1.31 95/11/24 Sun Microsystems"
#

include ../../Makefile.defs

OBJS		=	asn1_der_decode.o asn1_der_encode.o \
			Bstream.o Time.o \
      			ObjId.o Name.o \
			X509Cert.o Sig.o \
			Bigint.o randseed.o \
			Signed.o CRL.o \
			cafuncs.o utils.o \
			userfuncs.o \
			HashCert.o \
			X509skip.o SkipCert.o \
			cert_bn_glue.o md5.o md2.o \
			sha.o dsa_subr.o

SKIPINC		=	-I$(COMM_INC) 

HDRS		=	$(GLOBAL)

HFILES		=	Bigint.h Bstream.h CRL.h X509Cert.h \
			Name.h ObjId.h Sig.h Signed.h \
			Time.h asn1_der.h ca.h \
			my_types.h userfuncs.h \
			utils.h  SkipCert.h HashCert.h X509skip.h \
			bn_glue.h md2.h md5.h \
			sha.h dsa.h

BNDIR		=	../../bnlib
BNINC		=	-I$(BNDIR)
BNLIB		=	$(BNDIR)/libbn.a

CFLAGS		=  $(OPTIM) $(OS) $(D_TEMPLATES) $(BNINC) $(U_DEBUG) -D$(CLASS)

SCC		= $(CC) $(OS) $(OPTIM) $(U_DEBUG)

all:	libcert.a test 

clean:
	@$(RM) *.o libcert.a test
	@-$(SCCSCLEAN)

cleanall:
	@$(RM) -rf *.o libcert.a test
	@-$(SCCSCLEAN)

libcert.a: $(OBJS)
	$(AR_CC)  $@ $(OBJS)
	$(RANLIB) $@

# SOLARIS2 version

asn1_der_decode.o: asn1_der_decode.C $(HFILES)
	$(C++) $(CFLAGS) -c asn1_der_decode.C -o $@

asn1_der_encode.o: asn1_der_encode.C $(HFILES)
	$(C++) $(CFLAGS) -c asn1_der_encode.C -o $@

Bstream.o: Bstream.C Bstream.h
	$(C++) $(CFLAGS) -c Bstream.C -o $@

Time.o: Time.C Time.h
	$(C++) $(CFLAGS) -c Time.C -o $@

ObjId.o: ObjId.C $(HFILES)
	$(C++) $(CFLAGS) -c ObjId.C -o $@

Name.o: Name.C $(HFILES)
	$(C++) $(CFLAGS) -c Name.C -o $@

Sig.o: Sig.C $(HFILES)
	$(C++) $(CFLAGS) -c Sig.C -o $@

X509Cert.o: X509Cert.C $(HFILES)
	$(C++) $(CFLAGS) -c  X509Cert.C -o $@

X509skip.o: X509skip.C $(HFILES)
	$(C++) $(CFLAGS) -c  X509skip.C -o $@

randseed.o: randseed.C $(HFILES)
	$(C++) $(CFLAGS) -c randseed.C -o $@

CRL.o: CRL.C $(HFILES)
	$(C++) $(CFLAGS) -c CRL.C -o $@

Signed.o: Signed.C $(HFILES)
	$(C++) $(CFLAGS) -c Signed.C -o $@

cafuncs.o: cafuncs.C $(HFILES) $(HDRS)
	$(C++) $(CFLAGS) -c cafuncs.C -o $@

Bigint.o: Bigint.C Bigint.h 
	$(C++) $(CFLAGS) -c Bigint.C -o $@

utils.o: utils.C $(HFILES) utils.h
	$(C++) $(CFLAGS) -c utils.C -o $@

userfuncs.o: userfuncs.C $(HFILES)
	$(C++) $(CFLAGS) -c userfuncs.C -o $@

passwd.o: passwd.C $(HFILES)
	$(C++) $(CFLAGS) -c passwd.C -o $@

HashCert.o: HashCert.C $(HFILES)
	$(C++) $(CFLAGS) -c HashCert.C -o $@

SkipCert.o: SkipCert.C $(HFILES)
	$(C++) $(CFLAGS) -c SkipCert.C -o $@

cert_bn_glue.o: cert_bn_glue.C $(HFILES)
	$(C++) $(CFLAGS) -c cert_bn_glue.C -o $@

passwd_subr.o: passwd_subr.C $(HFILES)
	$(C++) $(CFLAGS) -c passwd_subr.C -o $@

dsa_subr.o: dsa_subr.C $(HFILES)
	$(C++) $(CFLAGS) -c dsa_subr.C -o $@

md5.o: md5.c $(HFILES)
	$(CC) -c md5.c -o $@

md2.o: md2.c $(HFILES)
	$(CC) -c md2.c -o $@

sha.o: sha.c $(HFILES)
	$(CC) -c $(BNINC) sha.c -o $@

#
# Test targets
#
test.o: test.C $(HFILES) 
	$(C++) $(CFLAGS) -c test.C -o $@

test: test.o libcert.a $(CRYPTOLIB)
	$(C++) $(CFLAGS) -o $@ test.o \
		libcert.a \
		$(BNLIB) $(CRYPTOLIB) $(CLASSLIB) $(OS_LIB)
