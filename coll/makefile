CC = gcc
CFLAGS = -g -I/u/dawagner/crypto/libdes
LIBS = -L/u/dawagner/crypto/libdes -ldes

all: des-serial crypt-2-serial crypt-master crypt-slave

des-serial: des-serial.c
	$(CC) $(CFLAGS) -o des-serial des-serial.c $(LIBS)

crypt-2-serial: crypt-2-serial.c
	$(CC) $(CFLAGS) -o crypt-2-serial crypt-2-serial.c $(LIBS)

crypt-master: crypt-master.c
	$(CC) $(CFLAGS) -DDEBUG -o crypt-master crypt-master.c $(LIBS)

crypt-slave: crypt-slave.c
	$(CC) $(CFLAGS) -o crypt-slave crypt-slave.c $(LIBS)

crypt-imposter: crypt-imposter.c
	$(CC) $(CFLAGS) -o crypt-imposter crypt-imposter.c
