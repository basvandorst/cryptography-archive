#
# Makefile for des
#

#
# define HAVE_ECB_BUFFER if the routine des_ecb_buffer() is in the library
#
DEFS= -DHAVE_ECB_BUFFER

#
# If des.h isn't in your system include directories, point this at a
# directory where it can be found.
#
INCL= -I../des

#
# The library you want to link this against.  -ldes if you like the
# system library.
#
LIBS= ../des/libdes.a

CFLAGS= $(DEFS) -O $(INCL)
LINTFLAGS = $(DEFS) $(INCL) -ha

SRCS=	des.c
OBJS=	des.o

des: $(OBJS)
	$(CC) $(CFLAGS) -o des $(OBJS) $(LIBS)

lint:
	lint $(LINTFLAGS) $(SRCS) $(LIBS)

clean:
	rm -f *.o des make.out

install:
	install -c -s des /local/bin

depend:
	mkdep $(CFLAGS) $(SRCS)

