CC = gcc
CFLAGS = -O3 -fomit-frame-pointer -Wall
LDFLAGS = -s

OBJS = main.o rmd160.o

rmd160: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

main.o: main.c
rmd160.o: rmd160.c

clean:
	rm -f rmd160 *.o *~ core
