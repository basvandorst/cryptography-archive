#
#	Makefile for tests for GMP+MP
#
#	tst-gcd is from GMP 1.3, tst-div tests the new mpz_gcd and mpz_divides
#	routines.
#
#	Ken Weber (kweber@mcs.kent.edu)		06/18/93
#

CC = gcc
LD = ld
LDFLAGS = -r

TEST_LIBS = ../libgmp+.a  ../../../libgmp.a

OPT = -O -g
CFLAGS = $(OPT) -I . -I../../../mpn/generic -I../../..

TEST_SRCS = tst-gcd.c tst-div.c
TEST_OBJS = tst-gcd.o tst-div.o
TESTS = tst-gcd tst-div

tests: $(TESTS)
	for i in $(TESTS); do echo $$i; ./$$i; done

tst-gcd: tst-gcd.o $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $@.o $(TEST_LIBS) 

tst-div: tst-div.o $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $@.o $(TEST_LIBS) 

$(TEST_PREFIX)clean:
	rm -f *.o $(TESTS) $(TEST_OBJS) core

tst-gcd.o : tst-gcd.c urandom.h 
	$(CC) $(CFLAGS) -DSIZE=16 -c tst-gcd.c

tst-div.o : tst-div.c urandom.h 
