#
# SSLeay/apps/Makefile.ssl
#

DIR=		apps
TOP=		..
CC=		cc
INCLUDES=	-I../include
CFLAG=		-g
INSTALLTOP=	/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -fMakefile.ssl
MAKEFILE=	Makefile.ssl

PEX_LIBS=
EX_LIBS= 

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile

LIBCRYPTO=../libRSAglue.a ../libcrypto.a ../../rsaref/install/rsaref.a
LIBSSL=../libssl.a

SSLEAY= ssleay
CA=	ca
SPEED=	speed
MD2=	md2
MD5=	md5
SHA=	sha
RC4=	rc4
ENC=	enc
DGST=	dgst
VERIFY=	verify
ASN1PARSE=asn1parse
CRL=	crl
RSA=	rsa
X509=	x509
GENRSA=	genrsa
GENDH=	gendh
PEM2BIN=pem2bin
BIN2PEM=bin2pem
REQ=req
S_SERVER=s_server
S_CLIENT=s_client
S_FILTER=s_filter
PEM_MAIL=pem_mail
PKCS7=pkcs7
DH=dh
ERRSTR=	errstr
HASHDIR=hashdir
S_TIME=s_time

SCRIPTS=makecert der_chop
E_EXE=	$(SPEED) $(CA) $(ENC) $(DGST) \
	$(VERIFY) $(ASN1PARSE) \
	$(CRL) $(RSA) $(X509) $(GENRSA) $(GENDH) $(PEM2BIN) $(BIN2PEM) \
	$(S_SERVER) $(S_CLIENT) $(S_FILTER) \
	$(REQ) $(DH) $(ERRSTR) $(HASHDIR) $(S_TIME)

EXE= $(SSLEAY)

PROGS= $(SSLEAY).c

APPSOBJ=apps.o
S_OBJ=	s_cb.o s_socket.o

SSLOBJ=	e_verify.o e_derpar.o e_req.o \
	e_dgst.o e_dh.o e_enc.o e_gendh.o e_errstr.o e_ca.o \
	e_crl.o e_rsa.o e_x509.o e_genrsa.o e_pem2bin.o e_bin2pem.o \
	e_server.o e_client.o e_filter.o $(APPSOBJ) $(S_OBJ) e_speed.o \
	e_hashdir.o e_s_time.o

OBJ=	speed.o md2.o sha.o md5.o enc.o dgst.o rc4.o verify.o asn1parse.o req.o \
	crl.o rsa.o x509.o genrsa.o pem2bin.o bin2pem.o ca.o \
	s_server.o s_client.o s_filter.o errstr.o hashdir.o \
	pem_mail.o pkcs7.o $(APPSOBJ) $(S_OBJ)

# md2.c sha.c md5.c rc4.c
SRC=	speed.c enc.c dgst.c verify.c asn1parse.c req.c ca.c \
	crl.c rsa.c x509.c genrsa.c pem2bin.c bin2pem.c \
	s_cb.c s_server.c s_client.c s_socket.c s_filter.c \
	pem_mail.c pkcs7.c apps.c errstr.c hashdir.c

EXHEADER= 
HEADER=	s_net.h testcert.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

# top:
#	@(cd ..; $(MAKE) DIRS=$(DIR) all)

all:	exe

exe:	$(EXE)

install: mklinks
	@for i in $(EXE) $(SCRIPTS) mklinks; \
	do  \
	(echo installing $$i; \
	 cp $$i $(INSTALLTOP)/bin/$$i; \
	 chmod 755 $(INSTALLTOP)/bin/$$i ) \
	 done; \
	cp ssleay.conf $(INSTALLTOP)/lib
	cd $(INSTALLTOP)/bin; \
	/bin/sh ./mklinks; \
	/bin/rm -f ./mklinks

tags:
	ctags $(SRC)

tests:

links:
	/bin/rm -f Makefile
	$(TOP)/util/point.sh Makefile.ssl Makefile

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

depend:
	$(MAKEDEPEND) $(INCLUDES) $(PROGS) $(SRC)

dclean:
	sed -e '/^# DO NOT DELETE THIS LINE/ q' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

errors:

clean:
	/bin/rm -f *.o *.obj lib tags core .nfs* *.old *.bak fluff $(EXE)

# $(LIBSSL):
#	(cd ../ssl; $(MAKE))

# $(LIBCRYPTO):
# 	(cd ../crypto; $(MAKE))

$(SSLEAY): progs.h $(SSLEAY).o $(SSLOBJ) $(LIBCRYPTO) $(LIBSSL)
	$(CC) -o $(SSLEAY) $(CFLAGS) $(SSLEAY).o $(SSLOBJ) $(PEX_LIBS) $(LIBSSL) $(LIBCRYPTO) $(EX_LIBS)

progs.h: g_ssleay.pl
	./g_ssleay.pl $(E_EXE) >progs.h

mklinks:
	./g_ssleay.pl $(E_EXE) >progs.h

$(SPEED): $(SPEED).o $(LIBCRYPTO)
	$(CC) -o $(SPEED) $(CFLAGS) $(SPEED).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(CA): $(CA).o $(LIBCRYPTO)
	$(CC) -o $(CA) $(CFLAGS) $(CA).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(MD2): $(MD2).o $(LIBCRYPTO)
	$(CC) -o $(MD2) $(CFLAGS) $(MD2).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(SHA): $(SHA).o $(LIBCRYPTO)
	$(CC) -o $(SHA) $(CFLAGS) $(SHA).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(MD5): $(MD5).o $(LIBCRYPTO)
	$(CC) -o $(MD5) $(CFLAGS) $(MD5).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(RC4): $(RC4).o $(LIBCRYPTO)
	$(CC) -o $(RC4) $(CFLAGS) $(RC4).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(ENC): $(ENC).o $(LIBCRYPTO)
	$(CC) -o $(ENC) $(CFLAGS) $(ENC).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(DGST): $(DGST).o $(LIBCRYPTO)
	$(CC) -o $(DGST) $(CFLAGS) $(DGST).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(VERIFY): $(VERIFY).o $(LIBCRYPTO)
	$(CC) -o $(VERIFY) $(CFLAGS) $(VERIFY).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(ASN1PARSE): $(ASN1PARSE).o $(APPSOBJ) $(LIBCRYPTO)
	$(CC) -o $(ASN1PARSE) $(CFLAGS) $(ASN1PARSE).o $(APPSOBJ) $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(CRL): $(CRL).o $(APPSOBJ) $(LIBCRYPTO)
	$(CC) -o $(CRL) $(CFLAGS) $(CRL).o $(APPSOBJ) $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(RSA): $(RSA).o $(APPSOBJ) $(LIBCRYPTO)
	$(CC) -o $(RSA) $(CFLAGS) $(RSA).o $(APPSOBJ) $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(REQ): $(REQ).o $(APPSOBJ) $(LIBCRYPTO)
	$(CC) -o $(REQ) $(CFLAGS) $(REQ).o $(APPSOBJ) $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(X509): $(X509).o $(APPSOBJ) $(LIBCRYPTO)
	$(CC) -o $(X509) $(CFLAGS) $(X509).o $(APPSOBJ) $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(GENRSA): $(GENRSA).o $(LIBCRYPTO)
	$(CC) -o $(GENRSA) $(CFLAGS) $(GENRSA).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(GENDH): $(GENDH).o $(LIBCRYPTO)
	$(CC) -o $(GENDH) $(CFLAGS) $(GENDH).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(PEM2BIN): $(PEM2BIN).o $(LIBCRYPTO)
	$(CC) -o $(PEM2BIN) $(CFLAGS) $(PEM2BIN).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(BIN2PEM): $(BIN2PEM).o $(LIBCRYPTO)
	$(CC) -o $(BIN2PEM) $(CFLAGS) $(BIN2PEM).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(S_SERVER): $(S_SERVER).o s_cb.o s_socket.o $(LIBCRYPTO) $(LIBSSL)
	$(CC) -o $(S_SERVER) $(CFLAGS) $(S_SERVER).o s_cb.o s_socket.o $(PEX_LIBS) $(LIBSSL) $(LIBCRYPTO) $(EX_LIBS)

$(S_CLIENT): $(S_CLIENT).o s_cb.o s_socket.o $(LIBCRYPTO) $(LIBSSL)
	$(CC) -o $(S_CLIENT) $(CFLAGS) $(S_CLIENT).o s_cb.o s_socket.o $(PEX_LIBS) $(LIBSSL) $(LIBCRYPTO) $(EX_LIBS)

$(S_FILTER): $(S_FILTER).o s_cb.o s_socket.o $(LIBCRYPTO) $(LIBSSL)
	$(CC) -o $(S_FILTER) $(CFLAGS) $(S_FILTER).o s_cb.o s_socket.o $(PEX_LIBS) $(LIBSSL) $(LIBCRYPTO) $(EX_LIBS)

$(PEM_MAIL): $(PEM_MAIL).o $(LIBCRYTO)
	$(CC) -o $(PEM_MAIL) $(CFLAGS) $(PEM_MAIL).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(PKCS7): $(PKCS7).o $(APPSOBJ) $(LIBCRYTO)
	$(CC) -o $(PKCS7) $(CFLAGS) $(APPSOBJ) $(PKCS7).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(DH): $(DH).o $(APPSOBJ) $(LIBCRYTO)
	$(CC) -o $(DH) $(CFLAGS) $(APPSOBJ) $(DH).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(ERRSTR): $(ERRSTR).o $(APPSOBJ) $(LIBCRYTO)
	$(CC) -o $(ERRSTR) $(CFLAGS) $(APPSOBJ) $(ERRSTR).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(HASHDIR): $(HASHDIR).o $(APPSOBJ) $(LIBCRYTO)
	$(CC) -o $(HASHDIR) $(CFLAGS) $(APPSOBJ) $(HASHDIR).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

$(S_TIME): $(S_TIME).o $(APPSOBJ) $(LIBCRYTO)
	$(CC) -o $(S_TIME) $(CFLAGS) $(APPSOBJ) $(S_TIME).o $(PEX_LIBS) $(LIBCRYPTO) $(EX_LIBS)

e_ca.o: ca.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c ca.c

e_speed.o: speed.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c speed.c

e_verify.o: verify.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c verify.c

e_derpar.o: asn1parse.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c asn1parse.c

e_crl.o: crl.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c crl.c

e_rsa.o: rsa.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c rsa.c

e_req.o: req.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c req.c

e_x509.o: x509.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c x509.c

e_genrsa.o: genrsa.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c genrsa.c

e_pem2bin.o: pem2bin.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c pem2bin.c

e_bin2pem.o: bin2pem.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c bin2pem.c

e_server.o: s_server.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c s_server.c

e_client.o: s_client.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c s_client.c

e_filter.o: s_filter.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c s_filter.c

e_dgst.o: dgst.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c dgst.c

e_dh.o: dh.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c dh.c

e_errstr.o: errstr.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c errstr.c

e_hashdir.o: hashdir.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c hashdir.c

e_s_time.o: s_time.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c s_time.c

e_enc.o: enc.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c enc.c

e_gendh.o: gendh.c
	$(CC) -DMONOLITH $(CFLAGS) $(CPPFLAGS) -o $@ -c gendh.c

# DO NOT DELETE THIS LINE -- make depend depends on it.
