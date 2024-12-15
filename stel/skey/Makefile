#
# $Author: vince $
# $Header: /users/vince/src/skey/RCS/Makefile,v 1.24 1996/04/26 16:29:34 vince Exp $
# $Date: 1996/04/26 16:29:34 $
# $Revision: 1.24 $
# $Locker:  $
#

CC=gcc
LBIN=/usr/local/bin
CDEBUGFLAGS = -g -O2 -fwritable-strings

##########################################
# add -DSKEYINITINSECURE at your own risk!
##########################################

DEFINES = -DSKEYD

LDFLAGS	= -L$(DESDIR)
DESDIR	= ../libdes
DESLIB	= -ldes
DESFLAGS= -I$(DESDIR)

# CUSTOMIZE THIS
# if this machine has /usr/include/stdlib.h, uncomment the first line,
# otherwise, uncomment the second
HASSTDLIB = -DHASSTDLIB
#HASSTDLIB =

LIB=	libskey.a
LIBOBJS =	skeylogin.o skeysubr.o md4.o put.o skey.o deskeyc.o \
		skeyaccess.o skey_getpass.o skey_crypt.o

RANLIB = ranlib

CFLAGS = $(DEFINES) $(CDEBUGFLAGS) $(HASSTDLIB) $(DESFLAGS)

what:
	@echo Please choose one of the following:
	@echo "hpux, sunos4, solaris24, solaris25, irix, linux, ultrix"
	@echo "freebsd, aix, bsdi"


hpux sunos4 ultrix freebsd:
	make all DEFINES="-DSKEYD -DSYSV"

bsdi:
	make all DEFINES="-DSKEYD -DSYSV -DBSDI -DFLOCK"

aix:
	make all DEFINES="-DSKEYD -DSYSV -DAIX"

solaris24:
	make all \
	DEFINES="-DSKEYD -DSYSV -DSOLARIS24" \
	LDFLAGS="-L$(DESDIR) -lsocket -lnsl" \
	RANLIB="touch"

solaris25:
	make all \
	DEFINES="-DSKEYD -DSYSV" \
	LDFLAGS="-L$(DESDIR) -lsocket -lnsl" \
	RANLIB="touch"

linux:
	make all DEFINES="-DSKEYD -DSYSV -DFLOCK"

irix:
	make all DEFINES="-DSKEYD -DSYSV" RANLIB="echo"

#all: key keylogin skey.init keysu $(LIB)
all: key skey.init keysu skeyd try $(LIB)

install: all man
	chmod 755 key keylogin skey.init keysu keyinfo keyinit
	chown root keylogin skey.init keysu keyinit
	cp keyinit skey.init key keylogin keysu keyinfo $(LBIN)
	chmod 4755 $(LBIN)/keylogin $(LBIN)/skey.init $(LBIN)/keysu
# this assumes things are set up it /etc
	cp /etc/login /etc/login.$$
	cp  keylogin /etc/login
	chmod 4755 /etc/login
#	cp /bin/login /bin/Ologin.$$
#	rm /bin/login
#	ln -s $(LBIN)/keylogin /bin/login
#	touch /etc/skeykeys
#	chmod 644 /etc/skeykeys

man:	
	cp *.1 /usr/local/man/man1

$(LIB):  $(LIBOBJS)
	ar rc $(LIB) $(LIBOBJS)
	$(RANLIB) $(LIB)

clean:
	-rm -f *.o $(LIB) key keylogin skey.init keysu skeyd try

suid: all
	chown root keylogin skey.init keysu
	chmod 4755 keylogin skey.init keysu

ci	:
	ci -q -l -mautomatic *.c *.h Makefile skeyd.8

key: skey.o $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o key skey.o $(LIB) $(DESLIB)

keylogin: login.o skeylogin.o skeysubr.o md4.o put.o setenv.o getenv.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o keylogin \
		login.o getenv.o setenv.o skeylogin.o $(LIB)

skey.init: skeyinit.o skeylogin.o skeysubr.o md4.o put.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o skey.init \
	skeyinit.o $(LIB) $(DESLIB)

keysu: su.o skeylogin.o skeysubr.o md4.o put.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o keysu su.o $(LIB) $(DESLIB)

skeyd: skeyd.o $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ skeyd.o $(LIB) $(DESLIB)

try: try.o $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ try.o $(LIB) $(DESLIB)

shar:
	shar *.c *.h Makefile skeydconf > skey.shar

login.o: login.c skey.h
md4.o: md4.c md4.h
skey.o: skey.c md4.h skey.h
skeylogin.o: skeylogin.c skey.h
skeysubr.o: skeysubr.c md4.h skey.h
put.o: put.c skey.h
