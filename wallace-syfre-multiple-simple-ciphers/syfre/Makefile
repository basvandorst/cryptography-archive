BINDIR	= /usr/local/bin
MANDIR	= /usr/man
MANSECT	= l
VERSION	= 1.1

CC	= g++
CFLAGS	= -O2
SHELL	= /bin/sh

AR	= /usr/bin/ar
CHMOD	= /usr/bin/chmod
CP	= /usr/bin/cp
RANLIB	= /usr/bin/ranlib
RM	= /bin/rm
STRIP	= /usr/bin/strip

.SUFFIXES : .os


# SSRCS   : source code files for "syfre".
# TSRCS   : source code files for "test" (for debugging).
# LSRCS   : source code files for cypher library (class definitions).
# LCYPS   : source code files for cyphers.

SSRCS	= syfre.c text.c version.c
TSRCS	= test.c version.c
LSRCS	= cypher.c data.c random.c
LCYPS	= cyphers/recurxor.c  cyphers/quadadd.c  cyphers/cubicxor.c \
	  cyphers/blockxor.c  cyphers/square.c   cyphers/sixteen.c  \
	  cyphers/rotate.c    cyphers/dualxor.c


# NOTE:     the following rules may not work with you version of make.
#           if not simply expand the rules out manually.  $(A:.b=.c)
#           means "take every entry in A, and replace '.b' suffixes
#           with '.c'".
#
# SOBJS   : object files for "syfre".
# TOBJS   : object files for "test" (for debugging).
# LOBJS   : object files for cypher library (class definitions).
# LCYPS_S : object files for cyphers (-DSECURE).
# LCYPS_D : object files for cyphers (with debug info).

SOBJS	= $(SSRCS:.c=.o)
TOBJS	= $(TSRCS:.c=.o)
LOBJS	= $(LSRCS:.c=.o)
LCYPS_S	= $(LCYPS:.c=.os)
LCYPS_D	= $(LCYPS:.c=.o)

SLIB	= libcypher.s.$(VERSION).a
DLIB	= libcypher.d.$(VERSION).a


all: test syfre

full: clean syfre syfretest

syfre:  $(SLIB) $(SOBJS)
	$(CC) $(CFLAGS) -o syfre $(SOBJS) $(SLIB)

test: $(DLIB) $(TOBJS)
	$(CC) $(CFLAGS) -o test $(TOBJS) $(DLIB)

clean:
	$(RM) -f core *.o $(SLIB) $(DLIB)
	$(RM) -f cyphers/*.o cyphers/*.os

install: syfre syfretest
	$(STRIP) syfre
	$(CP) syfre $(BINDIR)/syfre
	$(CHMOD) 555 $(BINDIR)/syfre

installman: syfre.man
	$(CP) syfre.man $(MANDIR)/man$(MANSECT)/syfre.$(MANSECT)
	$(CHMOD) 444 $(MANDIR)/man$(MANSECT)/syfre.$(MANSECT)


$(SLIB): $(LOBJS) $(LCYPS_S)
	$(AR) r $@ $?
	$(RANLIB) $@

$(DLIB): $(LOBJS) $(LCYPS_D)
	$(AR) r $@ $?
	$(RANLIB) $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.c.os:
	$(CC) $(CFLAGS) -DSECURE -c $< -o $@


# dependencies
#
cypher.o:   cypher.c  cypher.h data.h
data.o:     data.c             data.h
random.o:   random.c
syf.o:      syf.c     cypher.h data.h include.h config.h
syfre.o:    syfre.c   cypher.h data.h include.h config.h
test.o:     test.c    cypher.h data.h include.h config.h
text.o:     text.c             data.h include.h config.h
version.o:  version.c cypher.h data.h include.h config.h
