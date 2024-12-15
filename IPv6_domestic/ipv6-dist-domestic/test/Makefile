CFLAGS += -g -DINET6 -I../sys
LDFLAGS += -g -L/usr/inet6/lib -linet6
TARGETS = gendata socktest

all: $(TARGETS)

socktest: socktest.c
	cc $(CFLAGS) socktest.c $(LDFLAGS) -o socktest

clean:
	rm -f *~ *.o $(TARGETS)
