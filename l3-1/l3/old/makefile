GMPDIR=../../gmp-1.3.1
#CC=gcc
CC=CC

luc: luc.cc
	$(CC) -g -DTEST -D__STDC__ -o luc luc.cc -I$(GMPDIR) $(GMPDIR)/libgmp.a

cluc: luc-0.2.1.c
	$(CC) -g -DTEST -D__STDC__ -o cluc luc-0.2.1.c -I$(GMPDIR) $(GMPDIR)/libgmp.a

clean:
	rm -f *.o cluc luc cscope.out
