#
# @(#) sysmng		By Marco	Milano 10/89
#


#
# Site Customizations
#
#	PINST	: Install directory
#	BFIND	: "find" path-name
#
PINST	= /usr/local/bin
BFIND	= -DBFIND=\"/usr/local/bin/bfind\"
CONFDIR	= /usr/etc/security

#
# Makefile
#
# For BSD Machine :
#EXECS	= scheck logchk pwdchk getfinger
#
# Bsd haven't the /usr/adm/sulog file
#
# For System V Machine :
EXECS	= scheck suchk logchk pwdchk getfinger
#
INSTALL	= /usr/src/X11R4/mit/util/scripts/install.sh -c
SHELL	= /bin/sh

#DEFINES= -DNOTERM $(BFIND)

DEFINES	= $(BFIND) -DNOTERM
CFLAGS	= -O $(DEFINES)
LDFLAGS	= -s
LIBS	= alert.o noterm.o fast_pwent.o
OBJS	= scheck.o hash.o
INCS	= hash.h config.h logconf.h alertconfig.h fastpwent.h
MISC	= Makefile README INSTALLATION logconf alert.list
SRCS	= hash.c logchk.c logconfload.c pwdchk.c scheck.c suchk.c	\
	  alert.c noterm.c getfinger.c fast_pwent.c
DEP	= makedepend
DEPFLAGS= $(DEFINES)

all	: $(EXECS)

clean	::
	rm -f *.o $(EXECS)

clobber	::
	rm -f $(EXECS)

ci	::
	ci -q $(INCS) $(SRCS)

co	::
	co -q $(INCS) $(SRCS)

rcs	::
	ci -l -q -m"BackUp" $(INCS) $(SRCS)

install	: $(CONFDIR) $(PINST)/scheck $(PINST)/suchk $(PINST)/logchk  \
	  $(PINST)/pwdchk $(PINST)/getfinger
	[ -f $(CONFDIR)/alert.list ] ||	\
			$(INSTALL) -m 711 -o root alert.list $(CONFDIR)
	[ -f $(CONFDIR)/logconf ] ||	\
			$(INSTALL) -m 0400 -o root logconf $(CONFDIR)

$(CONFDIR) :
	mkdir $(CONFDIR)

scheck	: $(OBJS) $(LIBS)
	cc $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

suchk	: suchk.o $(LIBS)
	cc $(LDFLAGS) $@.o -o $@ $(LIBS)

getfinger	: getfinger.o $(LIBS)
	cc $(LDFLAGS) $@.o -o $@ $(LIBS)

logchk	: logchk.o logconfload.o $(LIBS)
	cc $(LDFLAGS) $@.o logconfload.o -o $@ $(LIBS)

pwdchk	: pwdchk.o $(LIBS)
	cc $(LDFLAGS) $@.o -o $@ $(LIBS)

$(PINST)/suchk : suchk
	$(INSTALL) -m 711 -o root suchk $(PINST)

$(PINST)/getfinger : getfinger
	$(INSTALL) -m 711 -o root getfinger $(PINST)

$(PINST)/scheck	: scheck
	$(INSTALL) -m 711 -o root scheck $(PINST)

$(PINST)/logchk	: logchk
	$(INSTALL) -m 711 -o root logchk $(PINST)

$(PINST)/pwdchk	: pwdchk
	$(INSTALL) -m 711 -o root pwdchk $(PINST)

shar	:
	makekit -o MANIFEST MANIFEST $(MISC) $(SRCS) $(INCS)

AFILE	= /usr/ftp/pub/security-2.0.tar.Z
ftp	:
	tar -cvf - $(MISC) $(SRCS) $(INCS) | compress -v > $(AFILE)
	chmod 444 $(AFILE)

wc	:
	wc -v $(SRCS) $(INCS)

depend	: $(SRCS)
	$(DEP) $(DEPFLAGS) $(SRCS)

tags	: $(SRCS)
	ctags -w $(SRCS)

lint	: $(SRCS)
	lint $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

hash.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h hash.h config.h
logchk.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
logchk.o: /usr/include/sys/param.h /usr/include/machine/param.h
logchk.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
logchk.o: /usr/include/sys/stat.h /usr/include/utmp.h /usr/include/stdio.h
logchk.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h /usr/include/errno.h
logchk.o: /usr/include/sys/errno.h /usr/include/pwd.h /usr/include/ctype.h
logchk.o: /usr/include/sys/socket.h /usr/include/sys/ioctl.h
logchk.o: /usr/include/netinet/in.h /usr/include/netdb.h /usr/include/sgtty.h
logchk.o: logconf.h config.h
logconfload.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
logconfload.o: /usr/include/sys/socket.h /usr/include/sys/ioctl.h
logconfload.o: /usr/include/netinet/in.h /usr/include/netdb.h
logconfload.o: /usr/include/stdio.h /usr/include/pwd.h /usr/include/ctype.h
logconfload.o: logconf.h config.h
pwdchk.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h
pwdchk.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/sgtty.h
pwdchk.o: /usr/include/sys/ioctl.h /usr/include/signal.h
pwdchk.o: /usr/include/sys/signal.h /usr/include/sys/types.h
pwdchk.o: /usr/include/sys/syscall.h /usr/include/pwd.h config.h fastpwent.h
scheck.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h
scheck.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h
scheck.o: /usr/include/sys/types.h /usr/include/sys/stat.h hash.h config.h
suchk.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
suchk.o: /usr/include/sys/param.h /usr/include/machine/param.h
suchk.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
suchk.o: /usr/include/sys/stat.h /usr/include/errno.h
suchk.o: /usr/include/sys/errno.h /usr/include/stdio.h /usr/include/pwd.h
suchk.o: /usr/include/sgtty.h /usr/include/sys/ioctl.h config.h
alert.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
alert.o: /usr/include/sys/stat.h /usr/include/signal.h
alert.o: /usr/include/sys/signal.h /usr/include/sys/syscall.h
alert.o: /usr/include/utmp.h /usr/include/stdio.h /usr/include/fcntl.h
alert.o: /usr/include/sys/fcntl.h alertconfig.h config.h
noterm.o: /usr/include/fcntl.h /usr/include/sys/stdsyms.h
noterm.o: /usr/include/sys/fcntl.h /usr/include/sys/types.h
noterm.o: /usr/include/signal.h /usr/include/sys/signal.h
noterm.o: /usr/include/sys/syscall.h
getfinger.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
getfinger.o: /usr/include/sys/param.h /usr/include/machine/param.h
getfinger.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
getfinger.o: /usr/include/sys/stat.h /usr/include/errno.h
getfinger.o: /usr/include/sys/errno.h /usr/include/stdio.h /usr/include/pwd.h
getfinger.o: /usr/include/sgtty.h /usr/include/sys/ioctl.h config.h
fast_pwent.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h
fast_pwent.o: /usr/include/pwd.h /usr/include/errno.h
fast_pwent.o: /usr/include/sys/errno.h fastpwent.h
