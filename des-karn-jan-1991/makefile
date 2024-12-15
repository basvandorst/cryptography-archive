#This makefile is for UNIX.
# By default, the code is compiled for a "big endian" machine (68K,
# Pyramid, Convex). To compile on a "little endian" machine (VAX, Intel)
# set the LITTLE_ENDIAN flag. If you get "ENCRYPT FAIL" messages out of
# the "certify" step, you've got the wrong byte order.

CFLAGS=-O
#CFLAGS=-O -DLITTLE_ENDIAN

all: descalc descycle descert certify radlogin benchmark des uuencode uudecode

des: main.o des.o getpass.o
	cc -o des main.o des.o getpass.o

benchmark: benchmark.o des.o
	cc -o benchmark benchmark.o des.o

radlogin: radlogin.o des.o
	cc -o radlogin radlogin.o des.o

descert: descert.o des.o
	cc -o descert descert.o des.o

descalc: descalc.o des.o
	cc -o descalc descalc.o des.o

descycle: descycle.o des.o
	cc -o descycle descycle.o des.o

certify: descert testdata
	descert < testdata > certify
	cat certify

radlogin.o: radlogin.c
	cc -c -O -DUTMP_FILE=\"/etc/utmp\" radlogin.c

uuencode: uuencode.o
	cc -o uuencode uuencode.o

uudecode: uudecode.o
	cc -o uudecode uudecode.o
