GMPDIR=../gmp-1.3.1/
INSTDIR=/usr/local/bin/
# use these for debugging
#
#CFLAGS=-g -I$(GMPDIR) -DDEBUG -v
#LFLAGS=-g

CFLAGS=-O -I$(GMPDIR)
LFLAGS=-s
CC=gcc
LOBJS=lucas.o prandom.o md5.o d3des.o misc.o idea.o

all: vsign sign decrypt encrypt genkey md5 chpwd prv2pub

chpwd: chpwd.o $(LOBJS)
	$(CC) $(LFLAGS) -o chpwd chpwd.o $(LOBJS) $(GMPDIR)/libgmp.a
md5: md5.o md5driver.o $(LOBJS)
	$(CC) $(LFLAGS) -o md5 md5driver.o $(LOBJS) $(GMPDIR)/libgmp.a
vsign: vsign.o $(LOBJS)
	$(CC) $(LFLAGS) -o vsign vsign.o  $(LOBJS) $(GMPDIR)/libgmp.a
sign: sign.o $(LOBJS)
	$(CC) $(LFLAGS) -o sign sign.o  $(LOBJS) $(GMPDIR)/libgmp.a
decrypt: decrypt.o $(LOBJS)
	$(CC) $(LFLAGS) -o decrypt decrypt.o $(LOBJS) $(GMPDIR)/libgmp.a
encrypt: encrypt.o $(LOBJS)
	$(CC) $(LFLAGS) -o encrypt encrypt.o $(LOBJS) $(GMPDIR)/libgmp.a
genkey: genkey.o $(LOBJS)
	$(CC) $(LFLAGS) -o genkey genkey.o $(LOBJS) $(GMPDIR)/libgmp.a
prv2pub: prv2pub.o $(LOBJS)
	$(CC) $(LFLAGS) -o prv2pub prv2pub.o $(LOBJS) $(GMPDIR)/libgmp.a
rtest: prandom.c $(LOBJS)
	$(CC) -DTEST $(LFLAGS) -o rtest prandom.c $(LOBJS) $(GMPDIR)/libgmp.a

clean:
	rm -f vsign sign decrypt encrypt genkey *.o core cscope.out test.pub test.prv md5 prv2pub chpwd rtest

signdist: 
	rm -f /tmp/MANIFEST
	find . -type f -print | sort | xargs md5 >/tmp/MANIFEST
	echo 'this file contains the md5 hash of the files in L3 0.3.6' >>/tmp/MANIFEST
	date >>/tmp/MANIFEST
	ripem -e -m mic-clear -u markh@wimsey.bc.ca </tmp/MANIFEST >SIGNED_MANIFEST
	rm -f /tmp/MANIFEST

install: all
	install -c -m 0755 encrypt $(INSTDIR)
	install -c -m 0755 decrypt $(INSTDIR)
	install -c -m 0755 vsign $(INSTDIR)
	install -c -m 0755 sign $(INSTDIR)
	install -c -m 0755 genkey $(INSTDIR)
	install -c -m 0755 md5 $(INSTDIR)
	install -c -m 0755 chpwd $(INSTDIR)
	install -c -m 0755 prv2pub $(INSTDIR)

lucas.o: lucas.c lucas.h
prandom.o: prandom.c
md5.o: md5.c
d3des.o: d3des.c
genkey.o: genkey.c
idea.o: idea.c
