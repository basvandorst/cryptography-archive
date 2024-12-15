# This is where to install this package
BINDIR=/u/elkins/bin/sunos4

# If you need to hardcode paths, this is the place to do it
PGP=pgp
MIMENCODE=mimencode

# set this to 'true' if you machine generates CRLF instead of just LF
CRLF=false

# System configuration:
#   If your system does not have rename(2), then you should change this to
#   -DRENAME=0
#
#   If your compiler has a "const" type, you should use -Dconst=const,
#   otherwise use the default.
#
SYSDEFS=-DRENAME=1 -Dconst=''

CC=gcc
CFLAGS=-g $(XCFLAGS) -DPGP_PATH=\"$(PGP)\" $(SYSDEFS) -Wunused

### No changes required beyond this line! ###

BINS=showencrypted showsigned pgpsign pgpencrypt tocanon

all: $(BINS)

tocanon:
	$(CC) $(CFLAGS) -o tocanon tocanon.c

showencrypted:
	$(CC) $(CFLAGS) -o showencrypted showencrypted.c

showsigned: showencrypted
	rm -f showsigned
	ln -s showencrypted showsigned

pgpsign: pgpsign.in
	rm -f pgpsign
	sed -e "s;%PGP%;$(PGP);" \
            -e "s;%MIMENCODE%;$(MIMENCODE);" \
	    -e "s;%CRLF%;$(CRLF);" \
            pgpsign.in > pgpsign
	chmod +x pgpsign

pgpencrypt: pgpencrypt.in
	rm -f pgpencrypt
	sed -e "s;%PGP%;$(PGP);" \
            -e "s;%MIMENCODE%;$(MIMENCODE);" \
	    -e "s;%CRLF%;$(CRLF);" \
            pgpencrypt.in > pgpencrypt
	chmod +x pgpencrypt

install: all
	for i in $(BINS); do cp $$i $(BINDIR); chmod +x $(BINDIR)/$$i; done

clean:
	rm -f *.o core *~ $(BINS)
