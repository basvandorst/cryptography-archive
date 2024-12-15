#	Ken Weber (kweber@mcs.kent.edu)		06/18/93
#	updated Ken Weber (kweber@mcs.kent.edu)		08/09/93


CC = gcc

#
#	For hp the -O2 makes the gcds much faster, but doesn't help for sparc2.
#	You'll have to figure out how to deal with this, I guess.
#

INCL_2_0 = ../.. 
OPT = -O -g -Wall
CFLAGS = $(OPT) -I$(INCL_2_0)
AR = ar
RANLIB=`if [ -r /bin/ranlib -o -r /usr/bin/ranlib ]; \
	then echo ranlib; else echo true; fi`
SHELL = /bin/sh

SRCS = $(MPZ_SRCS) $(MPN_SRCS) $(OPTL_MPN_SRCS)
OBJS = $(MPZ_OBJS) $(MPN_OBJS) $(OPTL_MPN_OBJS)

GMP_PLUS_OBJS = $(MPZ_OBJS) $(MPN_OBJS)

MPZ_SRCS = mpz/gcd.c mpz/divides.c
MPZ_OBJS = gcd.o divides.o

MPN_SRCS = mpn/accelgcd.c mpn/bingcd.c mpn/bmod.c mpn/sub_n_rsh.c\
	   mpn/compl_rsh.c
MPN_OBJS = accelgcd.o bingcd.o bmod.o sub_n_rsh.o compl_rsh.o 

all: libgmp+.a

check: libgmp+.a
	cd tests; $(MAKE) CC="$(CC)" SHELL="$(SHELL)" OPT="$(OPT)"

libgmp+.a: $(GMP_PLUS_OBJS)
	rm -f $@
	$(AR) cr $@ $(GMP_PLUS_OBJS)
	$(RANLIB) $@

LN = ln -s

clean:
	rm -f *.o libgmp+.a core
	-cd tests; $(MAKE) clean

realclean: clean

accelgcd.o : mpn/accelgcd.c 
	$(CC) $(CFLAGS) -c $? -o $@
bingcd.o : mpn/bingcd.c 
	$(CC) $(CFLAGS) -c $? -o $@
bmod.o : mpn/bmod.c 
	$(CC) $(CFLAGS) -c $? -o $@
sub_n_rsh.o : mpn/sub_n_rsh.c 
	$(CC) $(CFLAGS) -c $? -o $@
compl_rsh.o : mpn/compl_rsh.c 
	$(CC) $(CFLAGS) -c $? -o $@

gcd.o : mpz/gcd.c
	$(CC) $(CFLAGS) -c $? -o $@
divides.o : mpz/divides.c
	$(CC) $(CFLAGS) -c $? -o $@
