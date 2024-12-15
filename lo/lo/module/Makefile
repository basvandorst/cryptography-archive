CC=gcc
CFLAGS=-O2 -Wall
MV=mv

all: loop.o

loop.o: loop.c loop.h des.c des.h blk.h version.h
	$(CC) $(CFLAGS) -D__KERNEL__ -c loop.c
	$(LD) -r $@
	$(MV) a.out $@

version.h:
	@echo char kernel_version[] = \"`uname -r`\"\; > version.h

clean:
	rm -f *.o version.h 
