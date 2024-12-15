# Makefile for cfs 1.1.x
#*
#* The author of this software is Matt Blaze.
#*              Copyright (c) 1992, 1993, 1994 by AT&T.
#* Permission to use, copy, and modify this software without fee
#* is hereby granted, provided that this entire notice is included in
#* all copies of any software which is or includes a copy or
#* modification of this software and in all copies of the supporting
#* documentation for such software.
#*
#* This software is subject to United States export controls.  You may
#* not export it, in whole or in part, or cause or allow such export,
#* through act or omission, without prior authorization from the United
#* States government and written permission from AT&T.  In particular,
#* you may not make any part of this software available for general or
#* unrestricted distribution to others, nor may you disclose this software
#* to persons other than citizens and permanent residents of the United
#* States and Canada. 
#*
#* THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
#* WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
#* REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
#* OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
#*

# HP/UX, Irix, Linux and AIX ported by Mark Henderson (markh@wimsey.com)
# Ultrix 4.2 ported by Ed Gould (ed@left.wing.org)
# Solaris 2.3 ported by Mark Stein (Mark.Stein@Eng.Sun.COM)

# edit these to customize for your local site.
# note that I've tested only the 3 configurations listed
# Good luck if you're using something weird like Solaris...
BINDIR=/usr/local/bin
ETCDIR=/usr/local/etc
PRINTCMD=en

## use these for vanilla SUNOS
CFLAGS=-O
LIBS=
COMPAT=
#
## use these for Linux (Slackware 1.1.2)
#CFLAGS=-O
#LIBS=
#COMPAT=

# Irix 4.0 -- markh@wimsey.bc.ca
#CFLAGS=-cckr -O -Dirix
#LIBS=-lrpcsvc -lsun
#COMPAT=

# hpux 8.0 -- markh@wimsey.bc.ca
# also thanks to Charles Henrich (henrich@crh.cl.msu.edu)
# and Eric Ross (ericr@hpvclq.vcd.hp.com)
#CFLAGS=-O -Dhpux -DNORLIMITS
#LIBS=-lBSD
#COMPAT=

## use these for AIX 3.2.0 -- markh@wimsey.bc.ca
#CFLAGS=-O -D_BSD -D_SUN -DAIX320EUIDBUG
#LIBS=
#COMPAT=

## use these for 4.4/BSD386 systems with CFS on its own port
#CFLAGS=-O -DBSD44 -DANYPORT
#LIBS=-lrpc
#COMPAT=-lcompat

# Ultrix 4.2a
#CFLAGS=-O -DANYPORT
#LIBS=
#COMPAT=

# use these for 4.4/BSD386 systems with CFS on the NFS port because of no
# support for the port options in the mount syscall
# BSDI support by mab
# Also works under freeBSD, though you may want to use -static on the
#  linker (dean@deanstoy.wa.com (Dean M. Phillips))
#CFLAGS=-O -DBSD44 -DANYPORT -DCFS_PORT=2049
#LIBS=-lrpc
#COMPAT=-lcompat

## use these flags on Solaris 2.3
#CFLAGS=-O -DSOLARIS2X -DPORTMAP
#LIBS=-lsocket -lnsl
#COMPAT=

#not sure what to do for NeXT.  I think this works:
#CFLAGS=-O -posix -D_BSD -DANYPORT


# you shouldn't have to touch much below here

SRCS=Makefile admproto.x mount.x nfsproto.x cfs.c cfs_adm.c cfs_nfs.c cfs.h cfs_fh.c cfs_des.c cfs_cipher.c cattach.c getpass.c cdetach.c cmkdir.c adm.c cname.c ccat.c ver.c i o ssh
MANS=cattach.1 cdetach.1 cmkdir.1 ssh.1 cfsd.8 cname.8 ccat.8 README README.install README.history notes.ms
OBJS= cfs.o nfsproto_xdr.o nfsproto_svr.o admproto_xdr.o admproto_svr.o cfs_adm.o cfs_nfs.o cfs_fh.o cfs_des.o cfs_cipher.o adm.o ver.o
COBJS=admproto_clnt.o cfs_des.o cfs_cipher.o cattach.o getpass.o cmkdir.o cdetach.o ver.o cname.o ccat.o
OTHERS = nfsproto.h nfsproto_svr.c nfsproto_xdr.c admproto.h admproto_svr.c admproto_xdr.c admproto_clnt.c

all: cfsd cattach cmkdir cdetach cname ccat

cfsd: $(OBJS)
	cc $(OBJS) $(LIBS) -o cfsd

cattach: cattach.o admproto_clnt.o admproto_xdr.o getpass.o cfs_des.o cfs_cipher.o adm.o ver.o
	cc cattach.o admproto_clnt.o admproto_xdr.o cfs_des.o cfs_cipher.o getpass.o adm.o ver.o $(COMPAT) $(LIBS) -o cattach

cdetach: cdetach.o admproto_clnt.o admproto_xdr.o adm.o ver.o
	cc cdetach.o adm.o admproto_clnt.o admproto_xdr.o ver.o $(LIBS) -o cdetach

cmkdir: getpass.o cfs_des.o cfs_cipher.o cmkdir.o adm.o ver.o
	cc cmkdir.o cfs_des.o cfs_cipher.o getpass.o adm.o ver.o $(COMPAT) -o cmkdir

cname: cname.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o cfs_nfs.o ver.o
	cc cname.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o cfs_nfs.o ver.o $(LIBS) $(COMPAT) -o cname

ccat: ccat.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o cfs_nfs.o ver.o
	cc ccat.o getpass.o cfs_des.o cfs_cipher.o cfs_adm.o cfs_fh.o cfs_nfs.o ver.o $(LIBS) $(COMPAT) -o ccat



$(OBJS): nfsproto.h admproto.h cfs.h

$(COBJS): nfsproto.h admproto.h cfs.h

nfsproto_xdr.c: nfsproto.x
	rpcgen -c -o nfsproto_xdr.c nfsproto.x 

nfsproto_svr.c: nfsproto.x
	rpcgen -m -o nfsproto_svr.c nfsproto.x 

nfsproto.h: nfsproto.x
	rpcgen -h -o nfsproto.h nfsproto.x

admproto_xdr.c: admproto.x
	rpcgen -c -o admproto_xdr.c admproto.x 

admproto_svr.c: admproto.x
	rpcgen -m -o admproto_svr.c admproto.x 

admproto.h: admproto.x
	rpcgen -h -o admproto.h admproto.x

admproto_clnt.c: admproto.x
	rpcgen -l -o admproto_clnt.c admproto.x 

clean:
	rm -f $(OBJS) $(COBJS) $(OTHERS) cfsd cmkdir cattach cdetach

tarfile: $(SRCS) $(MANS)
	tar -cvf cfs.tar $(SRCS) $(MANS)

printout: $(SRCS) cfs.h admproto.h nfsproto.h
	$(PRINTCMD) $(SRCS) cfs.h admproto.h nfsproto.h

labprint: $(SRCS) nfsproto.h cfs.h
	pr $(SRCS) cfs.h nfsproto.h | ipr -Pip7 -O -2 #-F
	stroff -man $(MANS)

install: all
	install -c -o root cfsd $(ETCDIR)
	install -c -o root cattach cdetach cmkdir ssh cname ccat $(BINDIR)
#	install i o $(BINDIR)
	@echo "Kill any running cfsd prior to restarting."
	@echo "See the README file for more information."

