#
#  SecuDE Release 4.2 (GMD)
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


DD = ../


CPPFLAGS = -I$(OPENWINHOME)/include
LDFLAGS  = -L$(OPENWINHOME)/lib
LDLIBS   = -lxview -lolgx -lX

IFLAGS	 = $(CPPFLAGS)

PGM     = xpipe xpipe_slave xpipe_sel



all:	make

make:   $(PGM)
	cp $? $(BINDIR)
	touch make

xpipe.o: xpipe.c xpipe.glyph xpipe.h xpipe.icon xpipe.mask.icon
	$(CC) xpipe.c -c $(CFLAGS) $(CPPFLAGS)

xpipe: xpipe.o
	$(CC) xpipe.o -o $@ $(LDFLAGS) $(LDLIBS)

xpipe_slave.o: xpipe_slave.c xpipe_slave.h xpipe_slave.icon xpipe_slave.mask.icon
	$(CC) xpipe_slave.c -c $(CFLAGS) $(CPPFLAGS)

xpipe_slave: xpipe_slave.o
	$(CC) xpipe_slave.o -o $@ $(LDFLAGS) $(LDLIBS)

xpipe_sel.o: xpipe_sel.c
	$(CC) xpipe_sel.c -c $(CFLAGS) $(CPPFLAGS)

xpipe_sel: xpipe_sel.o
	$(CC) xpipe_sel.o -o $@ $(LDFLAGS) $(LDLIBS)
