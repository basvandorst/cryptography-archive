CFLAGS=-g -DIN6 -DINET6 -Wall -I../sys
LFLAGS=-L. -lin6
CC=gcc
LDCC=gcc
LIBIN6_OBJS=addr2ascii.o ascii2addr.o addr2hostname.o hostname2addr.o \
		_hostent_com.o

libinet6.a: $(LIBIN6_OBJS)
	ar rulv libinet6.a $(LIBIN6_OBJS)
	ranlib libinet6.a

clean: 
	rm -f *~ *.a *.o
