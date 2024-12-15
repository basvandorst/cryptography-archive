# Copyright 1993 by Scott Gustafson (scott@cadence.com)
#
# Symbols defined are SECURE, DEBUG, SYSLOG, TERMIO
# SECURE will give you a 32 byte string to type in to csu and respond,
#	otherwise the string is 8 bytes.  It will also give you a wider range
#	of characters to type in.  But you can always change that in encode.c
# TERMIO will hide the password from being displayed on the terminal you
#	are typing from if you have termios.
# SYSLOG will log messages to the system log files and the console

# This is the GID owner of the programs and the GID name
GID	=	9
GIDNAME	=	audit

# This is the name of the password file.  Note that this file needs to be
# protected in every known way.
PASSWD	=	csu.local

# This is the directory to put the binaries in
BINDIR	=	/bin

# This is the directory to put the password file in
ETCDIR	=	/etc

# My ANSI C compiler
CC	=	/usr/lang/acc
CFLAGS	=	-Xa -DGID=$(GID) -DCSUNAME=\"$(ETCDIR)/$(PASSWD)\" -DTERMIO -DSYSLOG
LDFLAGS	=	

all:	csu respond new-account

csu:	csu.o log.o encode.o genstr.o
	$(CC) $(CFLAGS) -o csu csu.o log.o encode.o genstr.o md5.o $(LDFLAGS)

respond: respond.o log.o encode.o
	$(CC) $(CFLAGS) -o respond respond.o log.o encode.o md5.o $(LDFLAGS)

new-account:	new-account.o log.o encode.o md5.o
	$(CC) $(CFLAGS) -o new-account new-account.o log.o encode.o md5.o $(LDFLAGS)

install: csu respond new-account
	cp csu $(BINDIR)
	chmod 6750 $(BINDIR)/csu
	chown root $(BINDIR)/csu
	chgrp $(GIDNAME) $(BINDIR)/csu
	cp respond $(BINDIR)
	chmod 711 $(BINDIR)/respond
	chown root $(BINDIR)/respond
	chgrp $(GIDNAME) $(BINDIR)/respond
	cp new-account $(BINDIR)
	chmod 6700 $(BINDIR)/new-account
	chown root $(BINDIR)/new-account
	chgrp $(GIDNAME) $(BINDIR)/new-account
	touch $(ETCDIR)/csu.local
	chmod 600 $(ETCDIR)/csu.local
	chown root $(ETCDIR)/csu.local
	chgrp $(GIDNAME) $(ETCDIR)/csu.local

server: csu new-account
	cp csu $(BINDIR)
	chmod 6750 $(BINDIR)/csu
	chown root $(BINDIR)/csu
	chgrp $(GIDNAME) $(BINDIR)/csu
	cp new-account $(BINDIR)
	chmod 6700 $(BINDIR)/new-account
	chown root $(BINDIR)/new-account
	chgrp $(GIDNAME) $(BINDIR)/new-account
	touch $(ETCDIR)/csu.local
	chmod 600 $(ETCDIR)/csu.local
	chown root $(ETCDIR)/csu.local
	chgrp $(GIDNAME) $(ETCDIR)/csu.local

client: respond
	cp respond $(BINDIR)
	chmod 711 $(BINDIR)/respond
	chown root $(BINDIR)/respond
	chgrp $(GIDNAME) $(BINDIR)/respond

clean:
	rm -f csu respond new-account *.o a.out core tmp

# Be very very very careful if you want to call this
clobber: clean
	rm -f $(BINDIR)/csu $(BINDIR)/respond $(BINDIR)/new-account
	rm -f $(ETCDIR)/csu.local


#Some dependencies
csu.o: csu.h genstr.h log.h encode.o Makefile
respond.o: log.h csu.h encode.o Makefile
new-account.o: log.h csu.h encode.o Makefile
log.o: log.c log.h Makefile
encode.o: encode.c md5.o csu.h Makefile
genstr.o: genstr.c genstr.h csu.h Makefile
md5.o: md5.c md5.h global.h Makefile
