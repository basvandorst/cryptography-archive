# This Makefile is for LINUX. This Makefile has been tested with a setup of:
#  o Linux kernel version 0.99.13
#  o GCC version 2.4.5
#  o Libc version 4.4.1
#
# This has also been tested to run with success on:
#  o FreeBSD kernel version BETA 1.0.0 
#  o GCC version 2.4.5
#
# I doubt you should have problems making this on, say, SunOS with
# regular cc, just change the CFLAGS and CC a litte... I haven't
# had a chance to test it out yet.
#
# Chris (chris@g386bsd)
# Dream Quest Communications
#
# This DES implementation is public domain. All rights worth shit.
#
# By default, the code is compiled for a "small endian" machine (VAX, Intel).
# To compile on a "large endian" machine (68K, Pyramid, Convex), take out the
# LITTLE_ENDIAN flag. If you get "ENCRYPT FAIL" messages out of the "certify"
# step, you've got the wrong byte order. (take out -DLITTLE_ENDIAN)

# Try CC=cc if you have no gcc
CC=gcc

# If you have no ranlib program, try RANLIB=echo
RANLIB=ranlib
AR=ar

# GCC is a highly optimizing compiler, as you can see
# If you're not running GCC, try 'CFLAGS=-O -DLITTLE_ENDIAN'
#
# These flags should work for any i486 based Unix with GCC version 2.4.5.
# Try taking out -m486 for other operating systems if you want to use
# GCC to compile this bastard.
CFLAGS=-O6 -ffast-math -fstrength-reduce -fomit-frame-pointer -m486 -DLITTLE_ENDIAN

# Define GETPASSO to getpass.o if your libraries don't contain getpass()
# GETPASSO=getpass.o
GETPASSO=

all: descalc descycle descert radlogin benchmark des certify libdes

des: main.o des.o $(GETPASSO)
	$(CC) $(CFLAGS) -o des main.o des.o $(GETPASSO)

benchmark: benchmark.o des.o
	$(CC) $(CFLAGS) -o benchmark benchmark.o des.o

radlogin: radlogin.o des.o
	$(CC) $(CFLAGS) -o radlogin radlogin.o des.o

descert: descert.o des.o
	$(CC) $(CFLAGS) -o descert descert.o des.o

descalc: descalc.o des.o
	$(CC) $(CFLAGS) -o descalc descalc.o des.o

descycle: descycle.o des.o
	$(CC) $(CFLAGS) -o descycle descycle.o des.o

certify: descert certify-data
	descert < certify-data > certify
	cat certify

radlogin.o: radlogin.c
	$(CC) $(CFLAGS) -c -DUTMP_FILE=\"/etc/utmp\" radlogin.c

libdes: des.o
	$(AR) rv libdes.a des.o
	$(RANLIB) libdes.a

install: radlogin descert descalc descycle des libdes benchmark
	install -c -s -o root -g root -m 4755 radlogin /bin/radlogin
	install -c -s -o root -g root -m 0755 descert /usr/bin/descert
	install -c -s -o root -g root -m 0755 descalc /usr/bin/descalc
	install -c -s -o root -g root -m 0755 descycle /usr/bin/descycle
	install -c -s -o root -g root -m 0755 des /usr/bin/des
	install -c -s -o root -g root -m 0755 benchmark /usr/bin/benchmark
	install -c -o root -g root -m 0644 libdes.a /usr/lib/libdes.a
	install -c -o root -g root -m 0644 des.1 /usr/man/man1/des.1
	install -c -o root -g root -m 0644 des.3 /usr/man/man3/des.3

dist: clean
	(cd ..;tar -cvf /tmp/des-linux.tar des-linux)
	gzip -9 /tmp/des-linux.tar
	mv /tmp/des-linux.tar.gz ../des-linux-1.0.tar.gz

clean:
	rm -f *~ core* *.o descert descalc descycle radlogin benchmark des libdes.a certify
