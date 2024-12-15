#GMPDIR=../gmp-1.3.2
#GMPLIB=$(GMPDIR)/libgmp-sun4.a

#GMPLIB=gmp.o

GMPDIR=/usr/include
GMPLIB=-lgmp
INSTDIR=/usr/local/bin
# use these for debugging
#
#CFLAGS=-g -I$(GMPDIR)  -DDEBUG -v
#LFLAGS=-g

CFLAGS=-O -I$(GMPDIR) -I/usr/include # -DSHORTNAMES # -DPCBC
LFLAGS=-s -static
CC=gcc
LOBJS=lucas.o prandom.o md5.o d3des.o misc.o idea.o tran.o rnd.o haval.o bf.o
RANLIB=ranlib

all: vsign sign decrypt encrypt genkey md5 chpwd prv2pub ssk vfk mg prvdump

libl3.a: $(LOBJS)
	rm -f libl3.a;	ar q libl3.a $(LOBJS); $(RANLIB) libl3.a

mg: mg.o libl3.a
	$(CC) $(LFLAGS) -o mg mg.o libl3.a $(GMPLIB)
vfk: vfk.o libl3.a
	$(CC) $(LFLAGS) -o vfk vfk.o libl3.a $(GMPLIB)
prvdump: prvdump.o libl3.a
	$(CC) $(LFLAGS) -o prvdump prvdump.o libl3.a $(GMPLIB)
ssk: ssk.o libl3.a
	$(CC) $(LFLAGS) -o ssk ssk.o libl3.a $(GMPLIB)
chpwd: chpwd.o libl3.a
	$(CC) $(LFLAGS) -o chpwd chpwd.o libl3.a $(GMPLIB)
md5: md5.o md5driver.o libl3.a
	$(CC) $(LFLAGS) -o md5 md5driver.o libl3.a $(GMPLIB)
vsign: vsign.o libl3.a
	$(CC) $(LFLAGS) -o vsign vsign.o  libl3.a $(GMPLIB)
sign: sign.o libl3.a
	$(CC) $(LFLAGS) -o sign sign.o  libl3.a $(GMPLIB)
decrypt: decrypt.o libl3.a
	$(CC) $(LFLAGS) -o decrypt decrypt.o libl3.a $(GMPLIB)
encrypt: encrypt.o libl3.a
	$(CC) $(LFLAGS) -o encrypt encrypt.o libl3.a $(GMPLIB)
genkey: genkey.o libl3.a
	$(CC) $(LFLAGS) -o genkey genkey.o libl3.a $(GMPLIB)
prv2pub: prv2pub.o libl3.a
	$(CC) $(LFLAGS) -o prv2pub prv2pub.o libl3.a $(GMPLIB)
rtest: prandom.c libl3.a
	$(CC) -DTEST $(LFLAGS) -o rtest prandom.c libl3.a $(GMPLIB)
luctest: lucas.c lucas.h
	$(CC) -I$(GMPDIR) -DTEST -g -o luctest lucas.c $(GMPLIB)
luct2: lucas.c lucas.h
	$(CC) -I$(GMPDIR) -DTEST2 -g -o luct2 lucas.c $(GMPLIB)
luct3: lucas.c lucas.h
	$(CC) -I$(GMPDIR) -DTEST3 -g -o luct3 lucas.c $(GMPLIB)

clean:
	(cd utils && make clean)
	(cd prencode && make clean)
	rm -f vsign sign decrypt encrypt genkey *.o core cscope.out md5 prv2pub chpwd rtest vfk ssk luctest luct2 luct3 mg libl3.a prvdump sample.dec foo ssk.L3

signdist: 
	@rm -f ./MANIFEST
	@find . -type f -print | sort | xargs md5 >./MANIFEST
	@what lucas.c >>./MANIFEST
	@echo 'this file contains the md5 hash of the files in L3' >>./MANIFEST
	@echo 'the file MANIFEST.lsig should contain a valid signature' >>./MANIFEST
	@echo ' which can be verified via' >> ./MANIFEST 
	@echo ' vsign markh@wimsey.bc.ca MANIFEST.lsig <MANIFEST ' >>./MANIFEST
	@date >>./MANIFEST
	sign markh@wimsey.bc.ca <./MANIFEST >MANIFEST.lsig
	pgp -sbf -u 0xC58015E3 <./MANIFEST >MANIFEST.sig

install: all
	(cd utils && make install INSTDIR=$(INSTDIR) )
	(cd prencode && make install INSTDIR=$(INSTDIR) )
	install -c -m 0755 encrypt $(INSTDIR)
	install -c -m 0755 decrypt $(INSTDIR)
	install -c -m 0755 vsign $(INSTDIR)
	install -c -m 0755 sign $(INSTDIR)
	install -c -m 0755 genkey $(INSTDIR)
	install -c -m 0755 md5 $(INSTDIR)
	install -c -m 0755 chpwd $(INSTDIR)
	install -c -m 0755 prv2pub $(INSTDIR)
	install -c -m 0755 ssk $(INSTDIR)
	install -c -m 0755 vfk $(INSTDIR)
	install -c -m 0755 mg $(INSTDIR)
	install -c -m 0755 prvdump $(INSTDIR)
	cd $(INSTDIR) ; rm -f unmg ucat ; ln mg unmg ; ln mg ucat

lucas.o: lucas.c lucas.h
prandom.o: prandom.c
md5.o: md5.c
d3des.o: d3des.c
genkey.o: genkey.c
idea.o: idea.c
misc.o: misc.c lucas.h
