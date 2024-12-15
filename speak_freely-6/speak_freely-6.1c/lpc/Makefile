
lpc.o:	lpc.c lpc.h
	$(CC) -c lpc.c

lpctest: lpctest.c lpc.o
	$(CC) lpctest.c lpc.o ../ulaw.o -o lpctest -lm

clean:
	rm -f *.bak *.o lpctest core
