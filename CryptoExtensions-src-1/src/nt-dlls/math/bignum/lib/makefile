# for use with watcom wmake under windows NT only.

#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

CC=wcc386
INCLUDES= -i=C:\apps\watcom\h;C:\apps\watcom\h\nt;c:\apps\java\include;c:\apps\java\include\win32
CFLAG= -mf -3s -bt=nt

CFLAGS= $(INCLUDES) $(CFLAG)

LIBSRC=bn.c bn_mul.c bn_regs.c
LIBOBJ=bn.obj bn_mul.obj bn_regs.obj

SRC= $(LIBSRC)

EXHEADER= bn.h 
HEADER=	bn_prime.h $(EXHEADER)

ALL= $(GENERAL) $(SRC) $(HEADER)

.c.obj :
	$(CC) $(CFLAGS) $<

all:	bignum.lib

bignum.lib:	$(LIBOBJ)
	wlib bignum +bn +bn_mul +bn_regs

