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

DFLAGS      =


OBJS    = dh.o
DAFDB =
DD      = ../
DFLAGS  = $(DWLNG) $(DISODE80) $(DICR1) # -DTEST

all: $(LIBSECUDE)

trace:	; ./make "OPT=$(OPT) -DTRACE"


dh.o: dh.c
	$(CC) dh.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DTIME)
dhtest.o: dhtest.c
	$(CC) dhtest.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DTIME)
dhtest: dhtest.o dh.o
	$(CC) dhtest.o dh.o -o $@ $(LFLAGS) $(LIB)
