# Makefile for mpn/tests

srcdir=../..
CC = gcc
CFLAGS = -g
OBJS = cputime.o test-cvt.o
INCLUDES = -I$(srcdir) -I$(srcdir)/mpn/$(TARGET) -I$(srcdir)/mpn/generic

%.o : %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $<

test-cvt: test-cvt.o cputime.o
	$(CC) -o tmp-$@ test-cvt.o cputime.o -L.. -lmpn
	if [ -x tmp-$@ ]; then mv tmp-$@ $@; fi

test-cvt.o: $(srcdir)/mpn/tests/test-cvt.c $(srcdir)/gmp.h $(srcdir)/gmp-impl.h
