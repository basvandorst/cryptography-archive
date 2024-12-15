CC = gcc

CFLAGS = -O -g -Wall # -pg
CDEFS  =  -DNNTP -DUNIX_SOCKET -DHAVE_READDIR -DDO_HAVE_GETTIMEOFDAY\
	-DDONT_HAVE_TM_GMTOFF # -Dstrtoul=strtol

SRCS	= generic.c memory.c parsedate.y test.c unix.c util.c\
		nntp_machine.c nntp_lib.c nntp_util.c\
		file_lib.c file_util.c\
		rpc_lib.c

OBJS	= generic.o memory.o parsedate.o unix.o util.o\
		nntp_machine.o nntp_lib.o nntp_util.o\
		file_lib.o\
		file_util.o\
		rpc_lib.o

YACC	= bison -y

all: news

.c.o: $<
	$(CC) $(CFLAGS) $(CDEFS) -c $<

depend: $(SRCS)
	gcc -MM $(SRCS) > .depend

news: $(OBJS) libnews.a test.c
	$(CC) $(CFLAGS) -o news test.c -L./ -lnews

libnews.a: $(OBJS)
	ar r $@ $(OBJS)
	ranlib $@

dist: clean
	tar -zcvf ../newsapi.tar.gz *

zip: clean
	zip -l -r ../newsapi.zip .

clean:
	rm -f *~ news *.o libnews.a
	make -f tcl/Makefile clean
	make -f News/makefile clean

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif

