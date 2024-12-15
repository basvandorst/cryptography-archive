
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


DD = ../


CPPFLAGS =   -I${OPENWINHOME}/include -DDEF_FILESEARCHPATH=\"/%T/%N%S\" -DSTARSIZE=2
XLIB = -lXext -lX11


OBJS = secxlock.o hsbramp.o usleep.o resource.o \
    hopalong.o qix.o life.o image.o blank.o \
    swarm.o sign_verify.o

PGM = secxlock

.c.o:;		$(CC) -c $*.c $(CFLAGS) $(DFLAGS) $(IFLAGS) 

all: make

make: $(PGM)
	cp $(PGM) $(BINDIR)
	touch make

$(PGM): $(OBJS)
	$(CC) -o $@ $(LDL) $(OBJS) $(LDOPTIONS) $(LIB) $(XLIB)  

