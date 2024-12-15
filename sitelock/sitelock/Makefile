# This makefile compiles the source code for sitelock
# Copyright (in parts) (C) 1994 by me.
# MD5 Message-Digest Algorithm" as found in RFC 1321.  That code is copyrighted:
#
#	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991.
#	All rights reserved.
#
# See the source code for copyright restrictions.
#
# No "makefile" is provided in the RFC.
# This one was written by Jim Ellis (jte@cert.org) for convenience.

# Note: a bug in mddriver causes "MD" to default to an incorrect value,
# so we set it to "5" here.
CFLAGS = -g -DMD=5 

all: trial make_key

make_key: md5c.o make_key.o
	cc -o make_key md5c.o make_key.o

trial: md5c.o sitelock.o trial.o
	cc -o trial md5c.o sitelock.o trial.o

make_key.o:
	cc -c $(CFLAGS) make_key.c

sitelock.o: global.h md5.h
	cc -c $(CFLAGS) sitelock.c

md5c.o: global.h md5.h
	cc -c $(CFLAGS) md5c.c

clean:
	rm -f *.o trial make_key
