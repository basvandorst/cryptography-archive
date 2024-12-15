# makefile for librand
# tested on Sparc-20 (SunOS 4.x) and P100 (BSDI) only.
# You're on your own elsewhere.  Read the comments for scary warnings.
# nt_truerand.c is included but untested.  Figure out how to use it
# youself, and don't say we didn't warn you.
#
# Usage: unsigned long trand8(), trand16(), trand32();
#
#* The authors of this software are Don Mitchell, Matt Blaze & Jack Lacy.
#*              Copyright (c) 1995 by AT&T.
#* Permission to use, copy, and modify this software without fee
#* is hereby granted, provided that this entire notice is included in
#* all copies of any software which is or includes a copy or
#* modification of this software and in all copies of the supporting
#* documentation for such software.
#*
#* THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
#* WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
#* REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY

SRCS=randbyte.c unix_truerand.c nt_truerand.c qshs.c
OBJS=randbyte.o unix_truerand.o qshs.o
CC=gcc
CFLAGS=
# No -O in CFLAGS! On some compilers, this optimizes out the counter...

librand.a: $(OBJS)
	ar rcv librand.a $(OBJS)
	ranlib librand.a

librand.shar: README makefile $(SRCS) 
	shar README makefile $(SRCS) > librand.shar

clean:
	rm -f $(OBJS) librand.a
