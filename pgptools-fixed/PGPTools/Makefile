# Generic Makefile for PGPTOOLS Demo
# -DHIGHFIRST on a big-endian machine
# Did I miss any dependencies?

CC=gcc -O

CPPFLAGS=-DSEEK_SET=0 -DSEEK_END=2 -DSEEK_CUR=1
CFLAGS=-DUNIX -DDYN_ALLOC -DNO_ASM $(CPPFLAGS) -DHIGHFIRST -DIDEA32 

ALLOBJS=        fifo.o pgptools.o pgpkmgt.o pgpkgen.o pgpktru.o pgpmem.o \
                pgparmor.o fifozip.o idea.o md5.o mpilib.o \
                zbits.o zdeflate.o zfile_io.o zglobals.o zinflate.o \
                zip.o zipup.o ztrees.o zunzip.o sparc.o

libpgptools.a:	$(ALLOBJS)
	-rm libpgptools.a
	ar rcu libpgptools.a $(ALLOBJS)
	ranlib libpgptools.a


ptd:		$(ALLOBJS) ptd.o
		$(CC) $(CFLAGS) ptd.o $(ALLOBJS) -o ptd
               
ptd.o:		ptd.c usuals.h md5.h mpilib.h pgpmem.h fifo.h \
                pgptools.h pgpkmgt.h pgpktru.h
		$(CC) $(CFLAGS) -c ptd.c -o ptd.o

fifo.o:		fifo.c fifo.h usuals.h pgpmem.h
		$(CC) $(CFLAGS) -c fifo.c -o fifo.o

pgptools.o:	pgptools.c pgptools.h fifo.h mpilib.h idea.h md5.h 
		$(CC) $(CFLAGS) -c pgptools.c -o pgptools.o

pgparmor.o:	pgparmor.c pgparmor.h usuals.h fifo.h
		$(CC) $(CFLAGS) -c pgparmor.c -o pgparmor.o

pgpkmgt.o:	pgpkmgt.c pgpkmgt.h pgptools.h fifo.h md5.h
		$(CC) $(CFLAGS) -c pgpkmgt.c -o pgpkmgt.o

pgpmem.o:	pgpmem.c pgpmem.h usuals.h md5.h mpilib.h fifo.h pgptools.h
		$(CC) $(CFLAGS) -c pgpmem.c -o pgpmem.o

pgpkgen.o:	pgpkgen.c pgpkgen.h pgpmem.h pgptools.h fifo.h mpilib.h \
                usuals.h md5.h
		$(CC) $(CFLAGS) -c pgpkgen.c -o pgpkgen.o

pgpktru.o:	pgpktru.c pgpktru.h usuals.h mpilib.h md5.h fifo.h \
		pgpmem.h pgptools.h pgpkmgt.h
		$(CC) $(CFLAGS) -c pgpktru.c -o pgpktru.o

fifozip.o:	fifozip.c fifo.h fifozip.h zipup.h zunzip.h
		$(CC) $(CFLAGS) -c fifozip.c -o fifozip.o

ptdemo.o:	ptdemo.c usuals.h md5.h mpilib.h fifo.h fifo.h pgptools.h
		$(CC) $(CFLAGS) -c ptdemo.c

idea.o:		idea.h idea.h
		$(CC) $(CFLAGS) -c idea.c -o idea.o

md5.o:		md5.c md5.h
		$(CC) $(CFLAGS) -c md5.c -o md5.o

mpilib.o:	mpilib.c mpilib.h
		$(CC) $(CFLAGS) -c mpilib.c -o mpilib.o

zbits.o:	zbits.c fifo.h zip.h
		$(CC) $(CFLAGS) -c zbits.c -o zbits.o

zdeflate.o:	zdeflate.c fifo.h zunzip.h zip.h
		$(CC) $(CFLAGS) -c zdeflate.c -o zdeflate.o

zfile_io.o:	zfile_io.c fifo.h zunzip.h
		$(CC) $(CFLAGS) -c zfile_io.c -o zfile_io.o

zglobals.o:	zglobals.c fifo.h zip.h
		$(CC) $(CFLAGS) -c zglobals.c -o zglobals.o

zinflate.o:	zinflate.c fifo.h zunzip.h
		$(CC) $(CFLAGS) -c zinflate.c -o zinflate.o

zip.o:		zip.c fifo.h usuals.h ziperr.h
		$(CC) $(CFLAGS) -c zip.c -o zip.o

zipup.o:	zipup.c fifo.h zip.h zrevisio.h
		$(CC) $(CFLAGS) -c zipup.c -o zipup.o

ztrees.o:	ztrees.c fifo.h zip.h zrevisio.h
		$(CC) $(CFLAGS) -c ztrees.c -o ztrees.o

zunzip.o:	zunzip.c fifo.h zunzip.h
		$(CC) $(CFLAGS) -c zunzip.c -o zunzip.o

