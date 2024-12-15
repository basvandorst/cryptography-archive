#
# SSLtelnet    - this is an implementation of ftp and ftpd using SSLeay
#
#
# - You may want to change SSLTOP depending on where you have SSL installed
# - You will probably have to choose which of the OS sections to
#   activate so that the right compiler switches and libraries are linked
# - if you don't want SOCKS support then unset sockslib and remove
#   the -DUSE_SOCKS from the CC line
#
# - if you want to use sralogin rather than login (perhaps your login
#   doesn't handle the -f option ... then define USE_SRALOGIN otherwise
#   telnetd will use /bin/login (see telnetd/pathnames.h). Support for
#   the -f option is *required* if you want to use "-z certsok" 
#
# Then all that should be required is a straight:
# make install
#
# 06-Jun-95 tjh    original extraction from my SRA source tree into
# ................ this separate package
#
#

VERSION = 0.5
DIRECTORY=SSLtelnet-$(VERSION)
TARFILE=$(DIRECTORY).tar

GENERAL=Makefile README VERSION TODO

#
#/* 
# * The modifications to support SSLeay were done by Tim Hudson
# * tjh@mincom.oz.au
# *
# * You can do whatever you like with these patches except pretend that
# * you wrote them. 
# *
# * Email ssl-users-request@mincom.oz.au to get instructions on how to
# * join the mailing list that discusses SSLeay and also these patches.
# *
# */
#

# the location where SSLeay is installed ... 
# - expect a include and lib directory under here 
SSLTOP=/usr/local/ssl

INSTALLTOP=$(SSLTOP)

SUBDIRS= lib telnet

# SunOX 4.x (Solaris 1.x)
CC= gcc -DSUNOS -DTERMCAP $(socksflags)
LDADD= $(sockslib) ../lib/libbsd/libbsd.a -ltermcap

# SunOS 5.x (Solaris 2.x)
#CC = cc -g -DSUNOS -DSOLARIS2 -DUSE_SHADOW -DEAY_DES $(socksflags)
# the extra libbsd crud is only there for inet_aton for GCC and
# I really do not want to think too hard about it --tjh
#LDADD= $(sockslib) -lresolv -lsocket -lnsl ../lib/libbsd/libbsd.a -ltermcap

# IRIX 5.x
#CC = cc -DIRIX -DEAY_DES $(socksflags)
#LDADD=$(sockslib) -ltermcap

# HPUX 9.x
#CC = cc -DHPUX -DEAY_DES $(socksflags) -Dconst=
#LDADD=$(sockslib) -ltermcap

# DGUX 5.x
#CC = cc -DDGUX -DEAY_DES $(socksflags) -Dconst=
#LDADD=$(sockslib) -ltermcap -lcurses
  
# uncomment the next two lines for linux
#CC = gcc -DLINUX -DTERMIOS $(socksflags)
#LDADD = $(sockslib) -lbsd -lncurses

##OLD Linux stuff
##CC = gcc -DLINUX -DUSE_SHADOW $(socksflags)
##LDADD = $(sockslib) -lshadow
  
all:
	@mkdir bin 2>/dev/null
	@for i in $(SUBDIRS); \
	    do (cd $$i && echo $$i && $(MAKE) CC="$(CC)" SSLTOP="$(SSLTOP)" LDADD="$(LDADD)") || exit; \
	done

lib: /dev/null
	@for i in "lib" ; \
	do (cd $$i && echo $$i && $(MAKE) CC="$(CC)" SSLTOP="$(SSLTOP)" LDADD="$(LDADD)") || exit; \
	done

telnet: lib /dev/null
	@for i in "telnet" ; \
	do (cd $$i && echo $$i && $(MAKE) CC="$(CC)" SSLTOP="$(SSLTOP)" LDADD="$(LDADD)") || exit; \
	done

clean: /dev/null
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) clean) || exit; \
		if [ -f bin/$$i ]; then \
		    rm bin/$$i ;\
		fi; \
	done

tar:    clean
	(cd ..; tar cf $(TARFILE) $(DIRECTORY) )

install: telnet telnetd 
	@-mkdir -p $(INSTALLTOP)/bin 2>/dev/null
	cp bin/telnet $(INSTALLTOP)/bin/telnet
