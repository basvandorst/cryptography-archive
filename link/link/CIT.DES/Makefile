#
#	Makefile - for DES routines
#
#	RCS ID: $Header: Makefile,v 2.1 90/07/18 10:59:42 lpb Exp $
#
#	major targets are:
#		make all	- compile binaries in this directory
#		make test	- run descert against the test suite
#		make install	- install the binaries & man entry
#		make clean	- remove all object files
#
#	Copyright 1988 Lawrence Brown and CITRAD
#
#	Compiler Defines:
#	-----------------
#	    if this program is compiled on a little-endian machine (eg Vax)
#		define '-DLITTLE_ENDIAN' in CFLAGS & LINTF
#		in order to enable the byte swapping  routines, 
#		which will incur a time penalty
#
#	    if compiled on a machine requiring strict alignment of words/longs
#		and cannot guarantee that the 8 char blocks passed are aligned
#		define '-DSTRICT_ALIGN' in CFLAGS & LINTF
#		this will incure about an 5% time penalty for each encryption
#
#	    if a detailed trace of DES function f is required for debugging
#		define '-DTRACE=1' or '-DTRACE=2' in CFLAGS & LINTF

CC = cc
CFLAGS=  ${GLOBCOPTS} -DDES_TABLES=\"${DES_TABLES}\" -DGEN_TABLES=\"${GEN_TABLES}\" -DVOID_SIGNAL

PROGS = des desmac descert benchmark desgen
PROGS2 = descalc desrnd mkcert uudecode uuencode
SRCS  = des.c desmac.c descert.c des64.c desgen.c benchmark.c getpass.c descalc.c desrnd.c mkcert.c tek.c uudecode.c uuencode.c
OBJS  = des.o desmac.o descert.o desgen.o benchmark.o getpass.o descalc.o desrnd.o mkcert.o tek.o uudecode.o uuencode.o
MANS  = benchmark.1 des.1 descert.1

DES_TABLES = des64.i
GEN_TABLES = gen64.i

all: ${PROGS}

des: des.o des64.o getpass.o
	cc ${CFLAGS} des.o des64.o getpass.o ${LIBS} -o des

desmac: desmac.o des64.o
	cc ${CFLAGS} desmac.o des64.o ${LIBS} -o desmac

benchmark: benchmark.o des64.o
	cc ${CFLAGS} benchmark.o des64.o ${LIBS} -o benchmark

descert: descert.o des64.o
	cc ${CFLAGS} descert.o des64.o ${LIBS} -o descert

radlogin: radlogin.o des64.o
	cc -o radlogin radlogin.o des64.o

descalc: descalc.o des64.o
	cc -o descalc descalc.o des64.o

descycle: descycle.o des64.o
	cc -o descycle descycle.o des64.o

desrnd: desrnd.o des64.o tek.o
	cc ${CFLAGS} desrnd.o des64.o tek.o ${LIBS} -o desrnd

mkcert: mkcert.o des64.o
	cc ${CFLAGS} mkcert.o des64.o ${LIBS} -o mkcert

desgen:	desgen.o
	cc ${CFLAGS} desgen.o ${LIBS} -o desgen

radlogin.o: radlogin.c
	cc -c -O -DUTMP_FILE=\"/etc/utmp\" radlogin.c

${OBJS}:	des.h

des64.o:	${GEN_TABLES} despriv.h

desgen.o:	${DES_TABLES}

des.o desmac.o:	desmode.h

${SRCS}: des.h

${GEN_TABLES}:	${DES_TABLES} desgen
	./desgen ${GEN_TABLES}

test: descert
	./descert testdata

install: ${PROGS} ${MANS}
	@mv -f ${PROGS} ${BINDIR}
	@cp  ${MANS} ${MANDIR}

clean:
	@rm -f *.o gen64.i a.out ${PROGS} ${PROGS2}

.c.o: 
	cc ${CFLAGS} -c $*.c

