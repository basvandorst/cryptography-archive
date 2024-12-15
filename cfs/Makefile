# Makefile for CFS 1.4 and ESM 1.0
#*
#* The author of this software is Matt Blaze.
#*              Copyright (c) 1992, 1993, 1994, 1995, 1997 by AT&T.
#* Permission to use, copy, and modify this software without fee
#* is hereby granted, provided that this entire notice is included in
#* all copies of any software which is or includes a copy or
#* modification of this software and in all copies of the supporting
#* documentation for such software.
#*
#* This software is subject to United States export controls.
#*
#* THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
#* WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
#* REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
#* OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
#*

#==========================================================================
# See README* and the CFS release notes (notes.ms) for instructions.
#==========================================================================

#==========================================================================
# Edit the three "CONFIGURE:" sections below to customize for your platform.
# Note that I've tested only the SunOS and BSDI configurations.
# If you discover a problem and change configuration, ALWAYS run
# make clean after editing this file.
#==========================================================================

# NOTE TO LINUX USERS and others with a non-standard rpcgen:
# There are no less than THREE versions of rpcgen distributed with
# various flavors of Linux floating around.  You MUST make sure
# that you are using the compile options for the right one so that
# it that emits standard SunRPC-compatible C code with "old-style" names.
# The version of rpcgen distributed with some (but not all) versions of
# Linux does not do this by default.  If CFS won't compile "out of the box,"
# this is the most likely problem.  Change RPC options to include -b -k,
# and things should # work fine (do a "make clean" after re-editing the
# makefile).  As a last resort, if you absolutely can't make your rpcgen
# work, try using "./make_with_bad_rpcgen" in this directory instead of
# "make".  (You'll need to do a "make clean" first if you've already
# failed with the existing rpcgen.) Hack at it long enough, and it will work.

# ESM NOTE: You need RSAREF 2.0 (not included in the distribution) to
# compile ESM.  You can get RSAREF by ftp from rsa.com .
# To compille, edit the configuration section for your platform below.
# You'll probably need to change the RSALIB and CFLAGS to point tothe
# right places for your local copy of RSAREF

# WARNING:  ESM RUNS ON FEWER PLATFORMS THAN CFS.  COMPILE ESM AT
# YOUR OWN RISK IF YOU AREN'T USING BSDI OR SUNOS 4.x.
# I think ESM now runs under Solaris, but test carefully.

#CFS PORTS:
# HP/UX, Irix, Linux and AIX ported by Mark Henderson (markh@wimsey.com)
# Ultrix 4.2 ported by Ed Gould (ed@left.wing.org)
# Solaris 2.3 ported by Mark Stein (Mark.Stein@Eng.Sun.COM)
# More Solaris/Linux help by Christopher Oliver (oliver@fritz.co.traverse.com) 
#   and Greg Onufer <Greg.Onufer@Eng.Sun.COM>
# Blowfish code by Eric Young.
# Blowfish support by Jeremey Barrett <jeremey@bluemoney.com>


#==========================================================================
# (1/3) CONFIGURE: local customization
#==========================================================================
#
# configuration options for all platforms
#
# 1A, 1B: pathnames, compiler, etc:

#1A: compiler:
# for cc, use
#CC=cc
#COPT=-O -DNOT_ANSI_C -DPROTOTYPES=0
# for gcc, use
CC=gcc
COPT=-O2 -DPROTOTYPES=1

#1B: paths:
#some peple like /usr/local/sbin instead of /usr/local/etc
BINDIR=/usr/local/bin
ETCDIR=/usr/local/etc
PRINTCMD=enscript -Gr2
# you only need RSAREF for ESM
# these paths WILL have to be changed...
RSALIB=/usr/mab/rsaref/install/rsaref.a
RINCLUDES=/usr/mab/rsaref/source

# if you're a paranoid fascist, you might want to configure
# default timeouts on the attach command.  If you do,
# just add definitions for TMOUT and IDLE with the default number
# of minutes you want to the CFLAGS line.
# So the COPT line for the SUNOS CC configuration with a timeout
# of 12 hours and an idle timer of 2 hours would look like:
# COPT=-O -DTMOUT=720 -DIDLE=120
# If you leave them out the default timeouts are infinite.  You
# can override them, of course, on the cattach command line.


#=======================================================================
# (2/3) CONFIGURE: platform selection
#=======================================================================
# Uncomment the options for the your local platform.
# You'll need to figure out how to install man pages yourself.

## Use these for vanilla SUNOS 4.x .
#CFLAGS=$(COPT) -I$(RINCLUDES) -DSUN
#LIBS=
#COMPAT=
#RPCOPTS=

## Use these for recent versions of Linux with the new rpcgen
## and broken glibc2 header files:
## (Known to work on RedHat 4.x and 5.x and many other current
## Linux dists).
## See NOTE TO LINUX USERS above, and also README.linux,
## if you can't make things work.
#CFLAGS=$(COPT) -U__STDC__ -Dd_fileno=d_ino -I$(RINCLUDES)
#LIBS=
#COMPAT=
#RPCOPTS= -k -b

## A few Linux users have reported success with these
## options:
## See NOTE TO LINUX USERS above, and also README.linux,
## if you can't make things work.
#CFLAGS=$(COPT) -U__OPTIMIZE__ -traditional -Dd_fileno=d_ino -I$(RINCLUDES)
#LIBS=
#COMPAT=
#RPCOPTS= -k -b

## Users of older versions Linux (Slackware 1.1.2) may be able to
## use these options:
## See NOTE TO LINUX USERS above, and also README.linux,
## if you can't make things work.
#CFLAGS=$(COPT) -I$(RINCLUDES)
#LIBS=
#COMPAT=
#RPCOPTS=

## Irix 4.0 -- markh@wimsey.bc.ca
## Be sure to read README.irix
#CFLAGS=-cckr $(COPT) -Dirix -I$(RINCLUDES)
#LIBS=-lrpcsvc -lsun
#COMPAT=
#RPCOPTS=

## HPUX 8.0 -- markh@wimsey.bc.ca
## Also thanks to Charles Henrich (henrich@crh.cl.msu.edu)
## and Eric Ross (ericr@hpvclq.vcd.hp.com)
#CFLAGS=$(COPT) -Dhpux -DNORLIMITS -I$(RINCLUDES)
#COMPAT=
#RPCOPTS=
#LIBS=-lBSD
# On later hpux versions, use
#LIBS=

## AIX 3.2.0 -- markh@wimsey.bc.ca
#CFLAGS=$(COPT) -D_BSD -D_SUN -DAIX320EUIDBUG -I$(RINCLUDES)
#LIBS=
#COMPAT=
#RPCOPTS=

## Generic 4.4 systems with CFS on its own port
#CFLAGS=$(COPT) -DBSD44 -DANYPORT -I$(RINCLUDES)
#LIBS=-lrpc
#COMPAT=-lcompat
#RPCOPTS=

## Ultrix 4.2a
#CFLAGS=$(COPT) -DANYPORT -I$(RINCLUDES)
#LIBS=
#COMPAT=
#RPCOPTS=

## BSD386 systems with CFS on its own port 
## Use this for BSDI 2.1 or later
## BSDI support by mab
#CFLAGS=$(COPT) -DBSD44 -DANYPORT -DSHORTLINKS -I$(RINCLUDES)
#LIBS=-lrpc
#COMPAT=-lcompat
#RPCOPTS=

## Use these for 4.4/BSD386 systems with CFS on the NFS port because of no
## support for the port options in the mount syscall
## Use this for BSDI 2.0 or earlier.
## BSDI support by mab
## Also works under older freeBSD, though you may want to use -static on the
##  linker (dean@deanstoy.wa.com (Dean M. Phillips))
#CFLAGS=$(COPT) -DBSD44 -DANYPORT -DCFS_PORT=2049 -DSHORTLINKS -I$(RINCLUDES)
#LIBS=-lrpc
#COMPAT=-lcompat
#RPCOPTS=

## Use these for FreeBSD 2.2.x systems with CFS on it's own port (Dima Ruban)
#CFLAGS=$(COPT) -DBSD44 -DANYPORT -DSHORTLINKS -I$(RINCLUDES)
#LIBS=
#COMPAT=-lcompat
#RPCOPTS=
 
##Use these for NetBSD i386 1.0 (John Kohl)
## For mounting, you need to use a command like:
##	mount -o -P,-c localhost:/null /crypt
## Use -DSHORTLINKS to support the BSD 4.4 symbolic links (Dave Carrel)
#CFLAGS=$(COPT) -DBSD44 -DANYPORT -DCFS_PORT=2049 -DSHORTLINKS -I$(RINCLUDES)
#LIBS=
#COMPAT=-lcompat
#RPCOPTS=

## Solaris 2.3 / SUNOS 5.x
#CFLAGS=$(COPT) -DSOLARIS2X -DPORTMAP -I$(RINCLUDES) -DPTMX
#LIBS=-lsocket -lnsl
#COMPAT=
#RPCOPTS=

## not sure what to do for NeXT.  I think this works:
#CFLAGS=$(COPT) -posix -D_BSD -DANYPORT -I$(RINCLUDES)


#==========================================================================
# (3/3) CONFIGURE: one last thing
#==========================================================================
# finally, comment out the next line:
CC=you_forgot_to_edit_the_makefile

# now you're done with local configuration.


#==========================================================================
# CONFIGURE: you shouldn't touch anything below here
#==========================================================================

SRCS=Makefile admproto.x mount.x nfsproto.x cfs.c cfs_adm.c cfs_nfs.c cfs.h \
  cfs_fh.c cfs_des.c cfs_cipher.c mcg.c mcgsbox.c mcg.h shs.c shs.h cattach.c \
  getpass.c cdetach.c cmkdir.c adm.c cname.c ccat.c cpasswd.c truerand.c \
  cfs_bfenc.c cfs_bfsk.c cfs_bf.h cfs_bfl.h cfs_bfp.h \
  safer.c safer.h VERSION LEVELS i o cfssh make_with_bad_rpcgen cmkkey
ESRCS=esm.c esm_cipher.c esm_gen.c dhparams.c esm.h
MANS=cattach.1 cdetach.1 cmkdir.1 cfssh.1 cfsd.8 cname.8 ccat.8 cpasswd.1 \
  README README.install README.history notes.ms README.linux README.irix \
  README.esm esm.1 cmkkey.1
OBJS= cfs.o nfsproto_xdr.o nfsproto_svr.o admproto_xdr.o admproto_svr.o \
  cfs_adm.o cfs_nfs.o cfs_fh.o cfs_des.o cfs_cipher.o adm.o ver.o mcgsbox.o \
  cfs_bfenc.o cfs_bfsk.o mcg.o safer.o
EOBJS=dhparams.o truerand.o esm_gen.o esm.o esm_cipher.o
COBJS=admproto_clnt.o cfs_des.o cfs_cipher.o cattach.o getpass.o cmkdir.o \
  cdetach.o ver.o cname.o ccat.o mcgsbox.o mcgsbox.o mcg.o shs.o cpasswd.o \
  cfs_bfenc.o cfs_bfsk.o truerand.o safer.o
OTHERS = nfsproto.h nfsproto_svr.c nfsproto_xdr.c admproto.h admproto_svr.c \
  admproto_xdr.c admproto_clnt.c ver.c

default:
	@echo make "cfs", "esm", "install_cfs" or "install_esm"

cfs: cfsd cattach cmkdir cdetach cname ccat cpasswd
	@echo

cfsd: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o cfsd

cattach: cattach.o admproto_clnt.o admproto_xdr.o getpass.o cfs_des.o \
  cfs_cipher.o cfs_bfenc.o cfs_bfsk.o adm.o ver.o mcg.o mcgsbox.o shs.o safer.o
	$(CC) cattach.o admproto_clnt.o admproto_xdr.o cfs_des.o \
	   cfs_bfenc.o cfs_bfsk.o cfs_cipher.o getpass.o adm.o ver.o mcg.o \
	   mcgsbox.o shs.o safer.o $(COMPAT) $(LIBS) -o cattach

cdetach: cdetach.o admproto_clnt.o admproto_xdr.o adm.o ver.o
	$(CC) cdetach.o adm.o admproto_clnt.o admproto_xdr.o \
	   ver.o $(LIBS) -o cdetach

cmkdir: getpass.o adm.o cfs_des.o cfs_cipher.o cmkdir.o ver.o mcg.o \
   mcgsbox.o cfs_bfenc.o cfs_bfsk.o safer.o shs.o truerand.o
	$(CC) cmkdir.o cfs_des.o cfs_cipher.o getpass.o adm.o ver.o mcg.o \
	   cfs_bfenc.o cfs_bfsk.o mcgsbox.o safer.o shs.o truerand.o \
	   $(COMPAT) -o cmkdir

cpasswd: getpass.o cfs_des.o cfs_cipher.o cpasswd.o ver.o mcg.o \
   mcgsbox.o cfs_bfenc.o cfs_bfsk.o safer.o shs.o truerand.o
	$(CC) cpasswd.o cfs_des.o cfs_cipher.o getpass.o ver.o mcg.o \
	   mcgsbox.o cfs_bfenc.o cfs_bfsk.o safer.o shs.o \
	   truerand.o  $(COMPAT) -o cpasswd

cname: cname.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o \
   cfs_bfenc.o cfs_bfsk.o cfs_nfs.o ver.o mcg.o mcgsbox.o safer.o shs.o
	$(CC) cname.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o \
	   cfs_nfs.o ver.o mcg.o mcgsbox.o cfs_bfenc.o cfs_bfsk.o safer.o \
	   shs.o $(LIBS) $(COMPAT) -o cname

ccat: ccat.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o cfs_nfs.o \
   ver.o mcg.o mcgsbox.o shs.o safer.o
	$(CC) ccat.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o \
	   cfs_bfenc.o cfs_bfsk.o cfs_nfs.o ver.o mcg.o mcgsbox.o shs.o \
	   safer.o $(LIBS) $(COMPAT) -o ccat

$(OBJS): nfsproto.h admproto.h cfs.h mcg.h safer.h shs.h \
   cfs_bf.h cfs_bfl.h cfs_bfp.h

$(COBJS): nfsproto.h admproto.h cfs.h mcg.h safer.h shs.h \
   cfs_bf.h cfs_bfl.h cfs_bfp.h

# truerand is a special case, no -O
truerand.o:
	$(CC) -c truerand.c

ver.c: VERSION LEVELS
	echo "static char version[]=" > ver.c
	echo "  \"CFS `cat VERSION` (`cat LEVELS`)\";" >> ver.c

nfsproto_xdr.c: nfsproto.x
	rpcgen $(RPCOPTS) -c -o nfsproto_xdr.c nfsproto.x 

nfsproto_svr.c: nfsproto.x
	rpcgen $(RPCOPTS) -m -o nfsproto_svr.c nfsproto.x 

nfsproto.h: nfsproto.x
	rpcgen $(RPCOPTS) -h -o nfsproto.h nfsproto.x

admproto_xdr.c: admproto.x
	rpcgen $(RPCOPTS) -c -o admproto_xdr.c admproto.x 

admproto_svr.c: admproto.x
	rpcgen $(RPCOPTS) -m -o admproto_svr.c admproto.x 

admproto.h: admproto.x
	rpcgen $(RPCOPTS) -h -o admproto.h admproto.x

admproto_clnt.c: admproto.x
	rpcgen $(RPCOPTS) -l -o admproto_clnt.c admproto.x 

clean:
	rm -f $(OBJS) $(COBJS) $(OTHERS)
	rm -f cfsd cmkdir cattach cpasswd cdetach cname ccat
	rm -f $(EOBJS) esm

tarfile: cfs.tar.gz

sharfile: cfs.shar

cfs.tar.gz: $(SRCS) $(ESRCS) $(MANS)
	rm -f cfs.tar.gz "cfs.`cat VERSION`.tar.gz"
	tar cf cfs.tar $(SRCS) $(ESRCS) $(MANS)
	gzip cfs.tar
	ln cfs.tar.gz "cfs.`cat VERSION`.tar.gz"

cfs.shar: $(SRCS) $(ESRCS) $(MANS)
	rm -f cfs.shar "cfs.`cat VERSION`.shar"
	shar $(SRCS) $(ESRCS) $(MANS)  > cfs.shar
	ln cfs.shar "cfs.`cat VERSION`.shar"

printout: $(SRCS) cfs.h mcg.h safer.h admproto.h nfsproto.h
	$(PRINTCMD) $(SRCS) cfs.h mcg.h safer.h admproto.h nfsproto.h

install_cfs: cfsd cattach cdetach cmkdir
	install -m 0755 -c -o root cfsd $(ETCDIR)
	install -m 0755 -c -o root cattach cdetach cmkdir cpasswd cfssh \
                cname ccat cmkkey $(BINDIR)
#	install -m 0755 i o $(BINDIR)
	@echo "Kill any running cfsd prior to restarting."
	@echo "See the README file for more information."
	@echo "Don't forget to install the man pages (*.[18])."


$(EOBJS): esm.h

esm: esm.o cfs_des.o esm_cipher.o dhparams.o truerand.o shs.o
	$(CC) -o esm esm.o cfs_des.o esm_cipher.o dhparams.o truerand.o shs.o $(RSALIB)

# to generate your own dhparams, remove the existing dhparams.c and
# remake esm.  You shouldn't do this if you want to remain interoperable.
# esm_gen takes a long time.
esm_gen: esm_gen.o esm_cipher.o cfs_des.o truerand.o
	$(CC) -o esm_gen esm_gen.o esm_cipher.o cfs_des.o truerand.o $(RSALIB)

dhparams.c:
	make esm_gen
	esm_gen > dhparams.c

install_esm: esm
	install esm $(BINDIR)

esm.shar: 
	shar README.esm Makefile esm.c *.x cfs_des.c esm_cipher.c esm_gen.c \
	     dhparams.c truerand.c shs.c esm.h esm.1> esm.shar
