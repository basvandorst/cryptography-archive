#
# Makefile
#
# Makefile for the in.identd daemon
#
# Modify it below to suit your particular system
#
DESTROOT=/usr/local
DESTDIR=$(DESTROOT)/sbin
MANROOT=$(DESTROOT)/man
MANDIR=$(MANROOT)/man8

#
# You *may* have to add the "-DHAVE_FORKCLOSE_BUG" if you're
# getting strange behaviour when using the "-w" or "-b" runtime
# options. See the source for more information.
#

#
# Use this if Sun SunOS 4.*
#
#LIBS=-lkvm
#DEFS=-DSUN -DHAVE_KVM

#
# Use this if Sequent Dynix 2.*
#
# LIBS=
# DEFS=-DSEQUENT

#
# Use this if Mips RISC/OS 4.*
#
# LIBS=-lmld
# DEFS=-DMIPS

#
# Use this if Hewlett-Packard HP-UX 7.*
#
#LIBS=
#DEFS=-DHPUX7

#
# Use this if Hewlett-Packard HP-UX 8.*, DEC Ultrix 4.* and others
#
LIBS=
DEFS= -D__convex__


CFLAGS= -O $(DEFS)

in.identd: 	identd.o kvm.o
	$(CC) -o in.identd identd.o kvm.o $(LIBS)

identd.o:	identd.c Makefile
kvm.o:		kvm.c kvm.h paths.h Makefile

install: in.identd
	cp in.identd $(DESTDIR)/in.identd
	cp identd.8  $(MANDIR)/identd.8

tests:
	(cd testdir ; make)

clean:
	rm -f in.identd *~ core *.o \#*
	(cd testdir ; make clean)

