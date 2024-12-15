CFLAGS=-O

all:	des.o

# all: des descert certify

des: main.o des.o getpass.o misc.o
	$(CC) $(CFLAGS) des main.o des.o getpass.o misc.o

descert: descert.o des.o
	$(CC) $(CFLAGS) descert descert.o des.o

certify: descert testdata
	descert < testdata
	touch certify

clean:
	rm -f *.bak *.o *.out core des descert certify
