#
# $Author: vince $
# $Header: /users/vince/src/stel-dist/stel/RCS/Makefile,v 1.86 1996/04/26 16:29:23 vince Exp vince $
# $Date: 1996/04/26 16:29:23 $
# $Revision: 1.86 $
# $Locker: vince $
#

# AUTHENTICATION
# -DSECURID and $(SDIDIR)=/usr/ace/system to add smart card
#  support code (unbundled)
# if user is securid registered securid auth is required
# if user is skey registered skey auth is required
# also, skey auth is required if /etc/skey.access says so
# last-resort authentication are unix passwords

# MORE CHECKS
# /etc/login.access is checked when auth succeeded

# -DGENERICLOGIN if you want to take advantage of STEL's internal login
# routine. If GENERICLOGIN is not defined, then /bin/login -f is run
# upon successful authentication. -DGENERICLOGIN is useful when system
# login is broken (i.e., it does not support the -f, -l or -p options).
# This option is turned ON by default.

# -DDESCORE if you want to take advantage of desCore DES library.
# Dana L. How DesCore's is a major speedup in respect to Eric Young's
# Libdes; speedups greatly depend on the architecture STEL runs on;
# they sometimes vary from 20% to 80% and over.
# This option is turned OFF by default.

# jrhett@Navigist.Com
# -DSOCKS if you want to SOCKSify the stel binary.
# SOCKSLIB must be defined to where your SOCKS library is.
# Only SOCKS version 4 is supported - v5 shouldn't need to be compiled in.
# This option is turned OFF by default.

# -DNOREMEXE if you want to disallow the command remote execution feature.
# If NOREMEXE is defined it is not possible to specify commands to be
# execute remotely, and only user's standard shell is executed at
# connection time.  This feature in intended for use with restricting
# shells, such as chroot-based jail systems.  As a consequence, file
# transfer commands are disabled when using this option is set.

# You should define OFFSET_T to be the right type.
# OFFSET_T=long for most architectures
# OFFSET_T=off_t for POSIX systems

CC	= gcc
DEFS	= -DGENERICLOGIN -DCOMBOFLAG $(SYSDEFS) ### -DSECURID
INCDIRS	= -I$(DESDIR) -I$(DESCOREDIR) -I$(SKEYDIR) -I$(GMPDIR) $(OTHERINCDIR) ### -I$(SDIDIR) 

DESDIR		= ../libdes
DESCOREDIR	= ../descore
#DESCORELIB	= -ldescore

SKEYDIR	= ../skey
GMPDIR	= ../gmp-1.3.2
GNUREGEX = ../regex-0.12

# SOCKS support
#SOCKSLIB	= -L/usr/local/lib/libsocks.a


# securid support
###SDIDIR	= /usr/ace/sun
###SDIDIR	= /usr/ace/hp
###SDILIB	= $(SDIDIR)/sdiclient.a
###SDILIB	= $(SDIDIR)/sdiclient.a

CFLAGS	= -g $(OPTIMIZE) -fwritable-strings $(DEFS) $(INCDIRS)

LDFLAGS	= -g $(OPTIMIZE) -L$(DESDIR) -L$(DESCOREDIR) \
	-L$(GMPDIR) -L$(SKEYDIR) $(SOCKSLIB) $(OTHERLIBDIR) -lskey -ldes \
	-lgmp $(SYSLIBS) -lm -lsocks


S5R3OBJ = utmp_init.o utmp_login.o utmp_logout.o updwtmp.o
S5R4OBJ = utmpx_init.o utmpx_login.o utmpx_logout.o
BSD4OBJ = login.o logout.o logwtmp.o


MASTER	= steld.o sub.o makeio.o md5c.o idea.o login_access.o sdihook.o \
	  crc32.o truerand.o rc4.o  # lip.o
SLAVE	= stel.o sub.o makeio.o md5c.o idea.o \
	  crc32.o truerand.o rc4.o  # lip.o

ALLOBJ	= $(MASTER) $(SLAVE) $(BSD4OBJ) $(S5R4OBJ) $(S5R3OBJ)

SRC	= steld.c stel.c sub.c makeio.c md5c.c idea.c  \
	  utmp_init.c utmp_login.c utmp_logout.c updwtmp.c \
	  utmpx_init.c utmpx_login.c utmpx_logout.c \
	  login.c logout.c logwtmp.c login_access.c sdihook.c \
	  crc32.c truerand.c rc4.c  # lip.c

ETC	= Makefile securid.conf skey.access skey.access.5 \
	  stel.1 steld.8 login.access securid.conf skeydconf

HDR	= defs.h idea.h md5.h global.h sys_defs.h usuals.h rc4.h # lip.h

what:
	@echo Please choose one of the following:
	@echo "hpux, sunos4, solaris24, solaris25, irix, linux, ultrix"
	@echo "freebsd, aix, bsdi"

hpux	: 
	make all ALLMASTER="$(S5R3OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_INFO -DOFFSET_T=long" \
	SYSLIBS="$(DESCORELIB)"

sunos4:
	make all ALLMASTER="$(BSD4OBJ) $(MASTER) $(GNUREGEX)/regex.o" \
	ALLSLAVE="$(SLAVE) $(GNUREGEX)/regex.o" \
	OTHERINCDIR="-I$(GNUREGEX)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DSUNOS4 -DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_NOTICE \
	-DOFFSET_T=off_t " \
	SYSLIBS="$(DESCORELIB)"

linux:
	make all ALLMASTER="$(S5R3OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	SYSDEFS="-DSMALL_CACHE -DLOGPRI=LOG_INFO -DOFFSET_T=off_t \
	-DNO_IP_PROTO" \
	SYSLIBS="$(DESCORELIB)"

irix	:
	make all ALLMASTER="$(S5R4OBJ) $(MASTER) $(GNUREGEX)/regex.o" \
	ALLSLAVE="$(SLAVE) $(GNUREGEX)/regex.o" \
	OTHERINCDIR="-I$(GNUREGEX)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DIRIX -DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_INFO \
	-DOFFSET_T=off_t -DHAS_UTMPX" \
	SYSLIBS="$(DESCORELIB)"

solaris24:
	make all ALLMASTER="$(S5R4OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE) $(GNUREGEX)/regex.o" \
	OTHERINCDIR="-I$(GNUREGEX)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DSOLARIS -DSOLARIS24 -DSYSV4 -DIDEA32 -DHIGHFIRST \
	-DLOGPRI=LOG_INFO -DOFFSET_T=off_t" \
	SYSLIBS="$(DESCORELIB) -lsocket -lnsl"

solaris25:
	make all ALLMASTER="$(S5R4OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DSOLARIS -DSYSV4 -DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_INFO \
	-DOFFSET_T=off_t" \
	SYSLIBS="$(DESCORELIB) -lsocket -lnsl"

freebsd:
	make all ALLMASTER="$(BSD4OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DIDEA32 -DLOGPRI=LOG_INFO -DOFFSET_T=off_t \
	-DHAS_SYS_PARAM_H" \
	SYSLIBS="$(DESCORELIB) -lutil"

bsdi:
	make all ALLMASTER="$(BSD4OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DBSDI -DIDEA32 -DLOGPRI=LOG_INFO -DOFFSET_T=off_t \
	-DHAS_SYS_PARAM_H" \
	SYSLIBS="$(DESCORELIB)"

ultrix:
	make all ALLMASTER="$(BSD4OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_INFO -DOFFSET_T=off_t \
	-DNO_IP_PROTO" \
	SYSLIBS="$(DESCORELIB)"

aix:
	make all ALLMASTER="$(BSD4OBJ) $(MASTER)" \
	ALLSLAVE="$(SLAVE)" \
	OPTIMIZE="-O2" \
	SYSDEFS="-DAIX -DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_INFO -DOFFSET_T=off_t"\
	SYSLIBS="$(DESCORELIB)"



all:	steld stel

steld	: $(ALLMASTER)
	$(CC) $(ALLMASTER) $(SDILIB) -o $@ $(LDFLAGS)

stel	: $(ALLSLAVE)
	$(CC) $(ALLSLAVE) -o $@ $(LDFLAGS)
	chmod 711 $@

ci	:
	ci -msafety -q -l $(SRC) $(ETC)

shar	:
	shar $(SRC) $(HDR) $(ETC) > stel.shar

lint	:
	lint \
	-DIDEA32 -DHIGHFIRST -DLOGPRI=LOG_INFO -DOFFSET_T=long \
	-I$(DESDIR) -I$(DESCOREDIR) -I$(SKEYDIR) -I$(GMPDIR)  $(DEFS) $(SRC)

wc	:
	wc -l $(SRC) $(HDR)

clean	:
	/bin/rm -f $(ALLOBJ) core steld stel

signatures	:
	echo > signatures
	echo Wrapped and signed on this date `date` >> signatures
	echo >> signatures
	md5 $(SRC) $(HDR) $(ETC) >> signatures
	echo >> signatures
	echo vince >> signatures
	pgp -sa signatures
	rm signatures

tag	:
	ctags -w $(SRC)
