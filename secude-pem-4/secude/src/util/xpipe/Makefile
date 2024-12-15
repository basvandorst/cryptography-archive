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


CPPFLAGS += -I$(OPENWINHOME)/include
LDFLAGS  += -L$(OPENWINHOME)/lib
LDLIBS   += -lxview -lolgx -lX

IFLAGS	 += $(CPPFLAGS)

PGM     = xpipe xpipe_slave xpipe_sel



all:	$(BINDIR)

xpipe.o: xpipe.c xpipe.glyph xpipe.h xpipe.icon xpipe.mask.icon
	$(CC) -g xpipe.c -c $(CPPFLAGS)

xpipe: xpipe.o
	$(CC) -g xpipe.o -o $@ $(LFLAGS) $(LIB) $(LDFLAGS) $(LDLIBS)

xpipe_slave.o: xpipe_slave.c xpipe_slave.h xpipe_slave.icon xpipe_slave.mask.icon
	$(CC) -g xpipe_slave.c -c $(CPPFLAGS)

xpipe_slave: xpipe_slave.o
	$(CC) -g xpipe_slave.o -o $@ $(LFLAGS) $(LIB) $(LDFLAGS) $(LDLIBS)

xpipe_sel.o: xpipe_sel.c
	$(CC) -g xpipe_sel.c -c $(CPPFLAGS)

xpipe_sel: xpipe_sel.o
	$(CC) -g xpipe_sel.o -o $@ $(LFLAGS) $(LIB) $(LDFLAGS) $(LDLIBS)

$(BINDIR): $(PGM)
	cp $? $(BINDIR)

clean:
	rm -f *.o *.bak *% core $(PGM)
