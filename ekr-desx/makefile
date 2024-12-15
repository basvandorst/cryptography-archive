#This makefile is for UNIX.
# By default, the code is compiled for a "big endian" machine (68K,
# Pyramid, Convex). To compile on a "little endian" machine (VAX, Intel)
# set the LITTLE_ENDIAN flag. If you get "ENCRYPT FAIL" messages out of
# the "certify" step, you've got the wrong byte order.

CFLAGS=-O
#CFLAGS=-O -DLITTLE_ENDIAN

all: desxtest 

desxtest: desxtest.o desx.o des.o
	cc -o desxtest desxtest.o desx.o des.o

