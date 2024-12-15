BINDIR=$(HOME)/bin
LIBDIR=$(HOME)/lib
MANDIR=$(HOME)/man
HDRDIR=$(HOME)/include

CC=gcc
CFLAGS=-O

HDR=gmp-ext.h gmp_fatal.h

SRC=\
gmp_fatal.c\
gmp_errstr.c\
mpz_2factor.c\
mpz_get_raw.c\
mpz_invert.c\
mpz_jacobi.c\
mpz_legendre.c\
mpz_set_raw.c\
mpz_size_raw.c\
mpz_sqrtm_amm.c\
mpz_sqrtm_berlekamp.c

libgmp_ext.a: $(SRC:c=o)
	ar cr libgmp_ext.a $(SRC:c=o)
	ranlib libgmp_ext.a

$(SRC):	$(HDR)

install:	libgmp_ext.a $(HDR)
	-rm -f $(LIBDIR)/libgmp_ext.a
	cp -p libgmp_ext.a $(LIBDIR)/libgmp_ext.a
	for f in $(HDR); do bn=`basename $$f`; rm -f $(HDRDIR)/$$bn; done
	for f in $(HDR); do bn=`basename $$f`; cp -p $$f $(HDRDIR)/$$bn; done

clean:
	-rm -f *.o libgmp_ext.a *~ core
