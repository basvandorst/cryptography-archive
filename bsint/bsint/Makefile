# Note: how you make archives is OS dependent.  You may need to diddle
# the "ar" command (or even add new commands).

CC=gcc
CFLAGS=
LIBS=-lg++ -lm

all:	libbsint.a demos

libbsint.a:	asmmath.o bsi_ops.o bui_ops.o bsint_io.o
	ar -rc libbsint.a asmmath.o bsi_ops.o bui_ops.o bsint_io.o

asmmath.o:	asmmath.c asmmath.h 
	$(CC) $(CFLAGS) -c asmmath.c

bsi_ops.o:	bsi_ops.cc bsint.h bsi_ops.h 
	$(CC) $(CFLAGS) -c bsi_ops.cc

bui_ops.o:	bui_ops.cc bsint.h bsi_ops.h asmmath.h 
	$(CC) $(CFLAGS) -c bui_ops.cc

bsint_io.o:	bsint_io.cc bsint_io.h bsint.h bsi_ops.h 
	$(CC) $(CFLAGS) -c bsint_io.cc

demos:	n3_1 mersenne

n3_1:	n3_1.o libbsint.a
	$(CC) -o n3_1 n3_1.o libbsint.a $(LIBS)

n3_1.o:	n3_1.cc
	$(CC) $(CFLAGS) -c n3_1.cc

mersenne:	mersenne.o libbsint.a
	$(CC) -o mersenne mersenne.o libbsint.a $(LIBS)

mersenne.o:	mersenne.cc
	$(CC) $(CFLAGS) -c mersenne.cc

