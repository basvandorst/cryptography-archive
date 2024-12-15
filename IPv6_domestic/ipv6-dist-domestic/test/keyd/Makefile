CFLAGS = -g -DINET6 
LDFLAGS = -g -L/usr/inet6/lib -linet6
TARGETS = keyd

all: $(TARGETS)


keyd: keyd.c
	cc $(CFLAGS) keyd.c $(LDFLAGS) -o keyd

key: key.c
	cc $(CFLAGS) key.c $(LDFLAGS) -o key

key2: key2.c
	cc $(CFLAGS) key2.c $(LDFLAGS) -o key2

clean:
	rm -f *~ *.o $(TARGETS)
