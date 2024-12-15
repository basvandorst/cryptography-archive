#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

FILES=bn.o bn_mul.o bn_regs.o

CC = gcc
AR = /usr/ccs/bin/ar

all: bignum.a

.c.o :
	$(CC) -c $< 

bignum.a : $(FILES)
	$(AR) -r bignum.a $(FILES)

clean:
	rm *.o *.a
