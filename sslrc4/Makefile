CC=gcc
SSL=/usr/local/ssl
CFLAGS= -O -Wall -I $(SSL)/include
DFLAGS= -L$(SSL)/lib -lcrypto

rc4test: rc4test.o
	$(CC) -o $@ rc4test.o $(DFLAGS)

clean:
	rm -f *.o
