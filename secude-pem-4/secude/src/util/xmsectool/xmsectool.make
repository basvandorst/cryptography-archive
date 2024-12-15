#
#  SecuDE Release 4.3 (GMD)
#
#
#    Copyright (C) 1990-1994, GMD Darmstadt
#
#                         NOTICE
#
#    Acquisition, use, and distribution of this module 
#    and related materials are subject to restrictions 
#    mentioned in each volume of the documentation.
#
#
#
#	Makefile for xmst 1.0
#


DD         = ../


ST_IFLAGS   = -I$(MOTIF)/include 
ST_LDFLAGS  = $(LDFLAGS) -L$(MOTIF)/lib  
ST_LDLIBS   = $(LDLIBS) $(LIB) -lMrm -lXm -lXt -lX11
DFLAGS      = $(DX500) $(DSTRONG) $(DCOSINE) $(DAFDB) $(DSCA) $(DTEST) $(SPECIALS) $(DISODE80) $(DICR1)

UIL	 =  uil
UILFLAGS =


SECTOOLUILMAIN	= main.uil
SECTOOLUIL	= main.uil alias.uil dialog.uil values.uil args.uil callb.uil icons.uil
SECTOOLOBJS	= xmst.o init.o util.o maint.o dialog.o st_cb.o st_util.o at_cb.o at_util.o
SECTOOLINCLUDES = xmst.h

PGM	= xmst
UID	= xmst.uid

.h:		touch $*.c
.c.o:;		$(CC) -c $*.c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(ST_IFLAGS)
.c.ln:;		$(LINT) $(LINTFLAGS) $*.c
.uil.uid:;	$(UIL) -o $(UILFLAGS)$*.uid $*.uil


all:    	$(PGM) $(UID)

xmst.uid:	$(SECTOOLUIL)
		$(UIL) -o $@ $(UILFLAGS) $(SECTOOLUILMAIN)
		cp xmst.uid $(XAPPLRESDIR)/uid

xmst:		$(SECTOOLOBJS)
		$(CC) -o $@ $(ST_IFLAGS) $(SECTOOLOBJS) $(LFLAGS) $(ST_LDFLAGS) $(ST_LDLIBS)
		cp xmst $(BINDIR)
	
clean:
	rm -f *.o *.bak *.uid *% core $(PGM)
