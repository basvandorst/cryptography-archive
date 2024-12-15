
#   Make file for Speak Freely for Unix

#   Uncomment the appropriate LFLAGS statement below according
#   to your machine type.

#   Linker flags

# Silicon Graphics
# In order to build Speak Freely, you need to have the packages
# dmedia_dev, dmedia_dev.sw, dmedia_dev.sw.audio, and dmedia_dev.sw.common
# installed.  In IRIX 5.3 and later, they are included with the IRIX
# Development Option (IDO), but may not be installed by a default
# installation of IDO.	The command "versions dmedia_dev.sw"
# will tell you whether these components are present on your system.
# If they aren't, you need to install them before building Speak
# Freely.
#CC = cc -float -signed
#LFLAGS = -laudio -lcurses -lm

# SunOS 4.1.x
#CC = cc -fsingle -DOLDCC
#LFLAGS = -lcurses -ltermcap -lm

# Solaris 2.x courtesy of Hans Werner Strube
# (-fsingle is needed for pre-4.0 compilers and is ignored by 4.0 in 
# ANSI mode.)  Defining THREADS enables multi-threaded operation in
# sflwld (and has no effect on any other component of Speak Freely).
# THREADS has been tested only on Solaris 2.5 SPARC and requires
# POSIX thread support.  If you cannot build with THREADS defined,
# simply remove it from the CCFLAGS line below.  If you disable
# THREADS, you can also remove the "-lpthread" library specification
# from the LFLAGS line, which may cause an error if the system does
# not include the POSIX threads library.
CC = cc -fsingle # for Sun Compiler
CCFLAGS = -DSolaris -DTHREADS
LFLAGS = -lcurses -lsocket -lnsl -lm -lpthread

# FreeBSD 2.2 courtesy of Andrey A. Chernov
#
# ** FreeBSD users please note: many FreeBSD audio drivers are
#    half-duplex, even through your sound card may actually
#    have full-duplex hardware.  If you have trouble running
#    sfmike and sfspeaker at the same time, try uncommenting
#    the definition DUPLEX = -DHALF_DUPLEX later in this file.
#    Depending on how your driver handles non-blocking I/O,
#    you may also have to add -DAUDIO_BLOCKING to the
#    CCFLAGS line.
#CCFLAGS = -DM_LITTLE_ENDIAN
#LFLAGS = -lcurses -ltermcap -lcompat -lm

# ** Linux users please note: many Linux audio drivers are
#    half-duplex, even through your sound card may actually
#    have full-duplex hardware.  If you have trouble running
#    sfmike and sfspeaker at the same time, try uncommenting
#    the definition DUPLEX = -DHALF_DUPLEX later in this file.
#    Depending on how your driver handles non-blocking I/O,
#    you may also have to add -DAUDIO_BLOCKING to the
#    CCFLAGS line.  In addition, if you're using an older
#    C library (libc), you may also need to add -DLINUX_FPU_FIX
#    to compile in code which forces the FPU into default IEEE
#    behaviour rather than interrupting on intentional and
#    harmless floating point underflows in lpc/lpc.c.
#
#    Also, on some versions of Linux you will need to change
#    the LFLAGS declaration to: LFLAGS = -lcurses -lm
#CC = gcc -O3 # for GNU's gcc compiler
#CCFLAGS = -DLINUX -DM_LITTLE_ENDIAN
#LFLAGS = -lncurses -lm
# If the above LFLAGS doesn't work, try the one below.
#LFLAGS = -lcurses -lm

# Hewlett-Packard courtesy of Marc Kilian
#	   PRELIMINARY--NOT FULLY TESTED
#CC = cc
#CCFLAGS = -DHEWLETT_PACKARD -DOLDCC
#LFLAGS = -lAlib -lcurses -ltermcap -lm

# Where Perl is located on your system.  This is used to make
# a directly-executable version of sfvod.
PERL = /usr/local/bin/perl

# If your audio hardware is half duplex, uncomment the next line
#DUPLEX = -DHALF_DUPLEX

# If your getdomainname() does not return the DNS domainname, define:
#DOMAIN=-DMYDOMAIN=\"somedomain.net\"

#   Everything will probably work OK without any changes below
#   this line.

#   Default Internet socket port used by sfmike and sfspeaker.	If you
#   change this, you will not be able to exchange sound with users
#   who've built Speak Freely with different values.  This default can
#   be overridden by the "-Pport" option on sfspeaker and the ":port"
#   hostname suffix in sfmike.	The ports used by Speak Freely are as
#   follows:
#
#	    INTERNET_PORT     UDP   Sound packets
#	    INTERNET_PORT+1   UDP   Control messages (RTCP)
#	    INTERNET_PORT+2   TCP   Communications with LWL server
#
#   If you don't publish your information or query an LWL server,
#   INTERNET_PORT+2 is never used.

INTERNET_PORT = 2074

CARGS = -DInternet_Port=$(INTERNET_PORT)

#DEBUG = -g -DHEXDUMP
DEBUG = -O

#   Compiler flags

CFLAGS = $(DEBUG) -Iadpcm -Ilpc -Igsm/inc -Imd5 -Iidea -Ilibdes $(CARGS) $(DUPLEX) $(CCFLAGS) $(DOMAIN)

PROGRAMS = sfspeaker sfmike sflwld sflwl sfecho sfvod

all:	$(PROGRAMS)

SPKROBJS = speaker.o codecs.o deskey.o g711.o rate.o rtpacket.o soundbyte.o ulaw.o usleep.o vatpkt.o vox.o hp_audio.o

sfspeaker: $(SPKROBJS) adpcmlib.o lpclib.o lpc10lib.o gsmlib.o deslib.o md5lib.o idealib.o libdes.o xdsub.o
	$(CC) $(SPKROBJS) adpcm/adpcm-u.o des/des.o md5/md5.o idea/idea.o lpc10/liblpc10.a gsm/lib/libgsm.a lpc/lpc.o xdsub.o libdes/libdes.a $(LFLAGS) -o sfspeaker

MIKEOBJS = mike.o codecs.o deskey.o g711.o rate.o rtpacket.o soundbyte.o ulaw.o usleep.o vatpkt.o xdsub.o hp_audio.o

sfmike: $(MIKEOBJS) adpcmlib.o lpclib.o lpc10lib.o gsmlib.o deslib.o md5lib.o idealib.o libdes.o
	$(CC) $(MIKEOBJS) adpcm/adpcm-u.o des/des.o md5/md5.o idea/idea.o -lm lpc10/liblpc10.a gsm/lib/libgsm.a lpc/lpc.o libdes/libdes.a $(LFLAGS) -o sfmike

LWLDOBJS = lwld.o xdsub.o

sflwld: $(LWLDOBJS)
	$(CC) $(LWLDOBJS) $(LFLAGS) -o sflwld

LWLOBJS = lwl.o

sflwl:	$(LWLOBJS)
	$(CC) $(LWLOBJS) $(LFLAGS) -o sflwl

ECHOOBJS = echo.o codecs.o g711.o rtpacket.o ulaw.o xdsub.o

sfecho: $(ECHOOBJS) md5lib.o
	$(CC) $(ECHOOBJS) md5/md5.o adpcm/adpcm-u.o lpc/lpc.o $(LFLAGS) -o sfecho

sfvod:	sfvod.pl
	echo \#\! $(PERL) >sfvod
	echo \$$version = `tail -1 version.h`\; >>sfvod
	echo '#include <stdio.h>' >sfvod-t.c
	echo '#include <sys/types.h>' >>sfvod-t.c
	echo '#include <sys/socket.h>' >>sfvod-t.c
	echo 'main(){printf("$$AF_INET = %d; $$SOCK_DGRAM = %d;%c", AF_INET, SOCK_DGRAM, 10);return 0;}' >>sfvod-t.c
	$(CC) sfvod-t.c -o sfvod-t
	./sfvod-t >>sfvod
	rm sfvod-t.c sfvod-t
	cat sfvod.pl >>sfvod
	chmod 755 sfvod

adpcmlib.o:
	( echo "Building ADPCM library."; cd adpcm ; make CC="$(CC)" )
	echo "ADPCM" >adpcmlib.o

libdes.o:
	( echo "Building LIBDES library."; cd libdes ; make -f Makefile.sf CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "LIBDES" >libdes.o

lpclib.o:
	( echo "Building LPC library."; cd lpc ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "LPC" >lpclib.o

lpc10lib.o:
	( echo "Building LPC10 library."; cd lpc10 ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "LPC" >lpc10lib.o

gsmlib.o:
	( echo "Building GSM library."; cd gsm ; make CC="$(CC)" )
	echo "GSM" >gsmlib.o

deslib.o:
	( echo "Building DES library."; cd des ; make CC="$(CC) $(CCFLAGS)" )
	echo "DES" >deslib.o

md5lib.o:
	( echo "Building MD5 library."; cd md5 ; make CC="$(CC)" )
	echo "MD5" >md5lib.o

idealib.o:
	( echo "Building IDEA library."; cd idea ; make CC="$(CC)" )
	echo "IDEA" >idealib.o

codecs.o:   codecs.c speakfree.h

speaker.o: speaker.c speakfree.h

mike.o: mike.c speakfree.h

lwl.o:	lwl.c speakfree.h

lwld.o: lwld.c speakfree.h

echo.o: echo.c speakfree.h

rtpacket.o: rtpacket.c speakfree.h rtp.h

soundbyte.o: soundbyte.c speakfree.h

vatpkt.o:   vatpkt.c speakfree.h vat.h

speakfree.h:	audio_descr.h rtp.h ulaw2linear.h types.h

testgsm:    testgsm.o gsmlib.o
	$(CC) testgsm.o -lm gsm/lib/libgsm.a $(LFLAGS) -o testgsm

manpage:
	nroff -man sfmike.1 | $(PAGER)
	nroff -man sfspeaker.1 | $(PAGER)
	nroff -man sflwl.1 | $(PAGER)
	nroff -man sflwld.1 | $(PAGER)
	nroff -man sfecho.1 | $(PAGER)
	nroff -man sfvod.1 | $(PAGER)

mantext:
	nroff -man sfmike.1 | col -b >/tmp/sfmike.man
	nroff -man sfspeaker.1 | col -b >/tmp/sfspeaker.man
	nroff -man sflwl.1 | col -b >/tmp/sflwl.man
	nroff -man sflwld.1 | col -b >/tmp/sflwld.man
	nroff -man sfecho.1 | col -b >/tmp/sfecho.man
	nroff -man sfvod.1 | col -b >/tmp/sfvod.man

printman:
	ptroff -man sfmike.1
	ptroff -man sfspeaker.1
	ptroff -man sflwl.1
	ptroff -man sflwld.1
	ptroff -man sfecho.1
	ptroff -man sfvod.1

clean:
	rm -f core *.out *.o *.bak $(PROGRAMS) *.shar sfvod-t*
	( cd adpcm; make clean )
	( cd libdes; make clean )
	( cd lpc; make clean )
	( cd lpc10; make clean )
	( cd gsm; make clean )
	( cd des; make clean )
	( cd md5; make clean )
	( cd idea; make clean )

#	Clean only the main directory, not the libraries
dusty:
	rm -f core *.out *.o *.bak $(PROGRAMS) *.shar sfvod-t*
