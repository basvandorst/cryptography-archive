# makefile for pgpd (and pg_inetd)
#
# You must set the following variables.
# PGPBIN  points at your PGP binary.
# PGPPATH will be passed to PGP in the environment; it should
#     point to the directory in which your PGP keyring lives.
#     Also in this directory is your "config.txt" file, which
#     will affect the operation of PGP, and hence this program.
# PGPD    points at your PGPD binary (for pg_inetd).
# KTPPORT is the port number on which pg_inetd will listen for connections.

PGPBIN=\"/afs/andrew.cmu.edu/inst/ini/bin/pgp\"
PGPPATH=\"/afs/cs.cmu.edu/user/thoth\"
PGPD=\"/afs/cs.cmu.edu/user/thoth/Project/Misc/pgpd/pgpd\"
KTPPORT=13013

PFLAGS=-DPGPBIN=$(PGPBIN) -DPGPPATH=$(PGPPATH) -DPGPD=$(PGPD) -DKTPPORT=$(KTPPORT)

# CFLAGS options:
#
# -DNO_STRERROR     For systems without strerror(3).
# -DNO_MKSTEMP      For systems without mkstemp(3).
# -DDEBUG           If you want debugging on by default (you don't).

# Try these on for size unless yours is one of the systems below.
CC=gcc
CFLAGS=-O2 -ansi

# Uncomment the following line if your make doesn't know $(MAKE).
#MAKE=make

PROGS=pgpd pg_inetd

all: $(PROGS)

pgpd: pgpd.c ktp.h
	$(CC) $(CFLAGS) $(PFLAGS) -o pgpd pgpd.c

pg_inetd: pg_inetd.c
	$(CC) $(CFLAGS) $(PFLAGS) -o pg_inetd pg_inetd.c

clean:
	-rm -f $(PROGS)

# Has been tested with the following three architectures:

# HP-UX 9.01
hpux:
	$(MAKE) all CC=cc CFLAGS="-O -Aa -D_HPUX_SOURCE"

# Ultrix 4.2 on a DECstation 5000
mips-ultrix:
	$(MAKE) all CC=gcc CFLAGS="-O2 -ansi -DNO_MKSTEMP"

# SunOS 4.1.3 on a SPARCstation 10
sunos:
	$(MAKE) all CC=gcc CFLAGS="-O2 -ansi -DNO_STRERROR"
