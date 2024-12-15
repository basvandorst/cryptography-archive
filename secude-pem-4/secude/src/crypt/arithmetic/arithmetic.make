#
#  SecuDE Release 4.3 (GMD)
#
#
#    Copyright (C) 1991, GMD Darmstadt
#
#                         NOTICE
#
#    Acquisition, use, and distribution of this module 
#    and related materials are subject to restrictions 
#    mentioned in each volume of the documentation.
#

#
# Makefile:     arithmetic for long numbers
#

OBJS =  lnutil.o \
	bool.o  \
	modarit.o \
        comp.o \
	genprime.o random.o primes.o
#       trace.o

.c.o:;		$(CC) $*.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS)

DD      = ../
DFLAGS  = $(DWLNG) $(DARITHTEST) $(ENDIAN) $(DISODE80) $(DICR1) # Arithmetic routines need word length
  
all: $(LIBSECUDE)
	cd $(ARCH); ./make

$(LIBSECUDE): $(OBJS)
	ar $(ARFLAGS) $@ $?

lib: $(OBJS)
	ar $(ARFLAGS) $(LIBSECUDE) $(OBJS)
	cd $(ARCH); ./make lib

clean:
	rm -f *.o *.bak *%
	cd $(ARCH); ./make clean

trace:	trace.o
	./make	"CC=$(CC)" "OPT=$(OPT) -DTRACE"
