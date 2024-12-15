#
# Unix makefile for Mandelsteg and Gifextract V1.0 by Henry Hastur.
#
# For MS-DOS, execute comp.bat instead. Sorry, but I'm too lazy to
# create a proper DOS makefile.
#

OFILES = gif_comp.o mandsteg.o 

LDFLAGS= 
CPPFLAGS=

#
# Use the following if you have plenty of memory to spare
#
#CFLAGS=-O

#
# Use the following for minimal memory use (some reduction in speed)
#

CFLAGS=-O -DLOW_MEM

#
# Use one of the following for SVR4
#
#CFLAGS=-O -DLOW_MEM -DSYSV
#CFLAGS=-O -DSYSV

CC=cc

all:		 mandelsteg gifextract

mandelsteg:	$(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -o mandelsteg $(LDFLAGS)

gifextract:	gifextr.o
	$(CC) $(CFLAGS) gifextr.o -o gifextract $(LDFLAGS)
	
.c.o:	
		$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)


