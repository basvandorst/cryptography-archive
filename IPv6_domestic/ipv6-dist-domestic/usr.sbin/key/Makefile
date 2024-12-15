CFLAGS = -g -DINET6 -I../../sys -I.
LDFLAGS = -g -L/usr/inet6/lib -linet6
TARGETS = key

all: $(TARGETS)

key: key.c
	cc $(CFLAGS) key.c $(LDFLAGS) -o key

clean:
	rm -f *~ *.o $(TARGETS)
