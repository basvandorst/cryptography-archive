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

ASMOBJS =  add.o sub.o cadd.o mult.o div.o dmult.o shift.o
OBJS    =  arintf.o
 
DD     = ../../
DFLAGS  = $(DWLNG) $(DARITHTEST) $(ENDIAN)      # Wortlaenge  32
IFLAGS = -I.. -I$(INCL)

all: $(LIBSECUDE)
 
arintf.o: $(ASMOBJS)
	${LD} ${LDFLAGS} -r ${ASMOBJS} -o arintf.o 

add.o:   add.c
sub.o:   sub.c
div.o:   div.c
mult.o:  mult.c
shift.o: shift.c
dmult.o: dmult.c

