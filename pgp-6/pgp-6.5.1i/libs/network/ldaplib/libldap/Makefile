# --------------------------------------------------------
#  This file was automatically generated.  Do not edit it.
#  Instead, edit the Make-common file (located in the root
#  (of the LDAP distribution).  See the LDAP INSTALL file
#  for more information.
# --------------------------------------------------------
#
#-----------------------------------------------------------------------------
# Copyright (c) 1994 Regents of the University of Michigan.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that this notice is preserved and that due credit is given
# to the University of Michigan at Ann Arbor. The name of the University
# may not be used to endorse or promote products derived from this
# software without specific prior written permission. This software
# is provided ``as is'' without express or implied warranty.
#
#       LDAP library makefile
#
#-----------------------------------------------------------------------------

LDAPSRC	= ..
PGPLIBS = ../../..
OUTDIR  = $(LDAPSRC)/unix
PGPCDKDIR = $(PGPLIBS)/pgpcdk
PGPPFLDIR = $(LDAPSRC)/../../pfl
PGPCDKINCLUDES = -I$(PGPCDKDIR)/pub/include -I$(PGPCDKDIR)/unix
PGPPFLINCLUDES = -I$(PGPPFLDIR)/common -I$(PGPPFLDIR)/unix

SRCS	= bind.c open.c result.c error.c compare.c search.c \
	  modify.c add.c modrdn.c delete.c abandon.c ufn.c cache.c \
	  getfilter.c regex.c sbind.c kbind.c unbind.c friendly.c cldap.c \
	  free.c disptmpl.c srchpref.c dsparse.c tmplout.c sort.c \
	  getdn.c getentry.c getattr.c getvalues.c addentry.c \
	  request.c getdxbyname.c os-ip.c url.c charset.c

OBJS	= bind.o open.o result.o error.o compare.o search.o \
	  modify.o add.o modrdn.o delete.o abandon.o ufn.o cache.o \
	  getfilter.o regex.o sbind.o kbind.o unbind.o friendly.o cldap.o \
	  free.o disptmpl.o srchpref.o dsparse.o tmplout.o sort.o \
	  getdn.o getentry.o getattr.o getvalues.o addentry.o \
	  request.o getdxbyname.o os-ip.o url.o charset.o

HDIR	= -I$(LDAPSRC)/include -I$(LDAPSRC)/pgpsockets

INCLUDES= $(HDIR) $(PGPCDKINCLUDES) $(PGPPFLINCLUDES)

DEFINES	= $(DEFS) -DFILTERFILE="\"$(RUNTIMEETCDIR)/ldapfilter.conf\"" \
	  -DTEMPLATEFILE="\"$(RUNTIMEETCDIR)/ldaptemplates.conf\"" \
	  -DPGPSOCKETSLDAP=1

CFLAGS	= $(INCLUDES) $(DEFINES) $(ACFLAGS)
LDFLAGS = -L$(OUTDIR)
LIBS	= -lldap -llber

all:	$(OUTDIR)/libldap.a DONE

$(OUTDIR)/libldap.a: $(OBJS)
	$(AR) ruv $@ $(OBJS)
	@if [ ! -z "$(RANLIB)" ]; then \
		$(RANLIB) $@; \
	fi;

DONE:
	@set -x; echo $(OBJS) > $@

ltest:	libldap.a test.o ../liblber/liblber.a
	$(CC) $(ALDFLAGS) -o $@ test.o $(LIBS) $(ALIBS)

ttest:	libldap.a tmpltest.o ../liblber/liblber.a
	$(CC) $(ALDFLAGS) -o $@ tmpltest.o $(LIBS) $(ALIBS)

version.c:	$(OBJS)
	$(RM) $@
	(u=$${USER-root} v=`$(CAT) ../../build/version` d=`$(PWD)` \
	h=`$(HOSTNAME)` t=`$(DATE)`; $(SED) -e "s|%WHEN%|$${t}|" \
	-e "s|%WHOANDWHERE%|$${u}@$${h}:$${d}|" \
	-e "s|%VERSION%|$${v}|" \
	< Version.c > $@)

install:	libldap.a ldapfilter.conf ldapfriendly ldaptemplates.conf ldapsearchprefs.conf FORCE
	-$(MKDIR) -p $(LIBDIR)
	$(INSTALL) $(INSTALLFLAGS) -m 644 libldap.a $(LIBDIR)
	@if [ ! -z "$(RANLIB)" ]; then \
		(cd /tmp; $(RANLIB) $(LIBDIR)/libldap.a) \
	fi
	-$(MKDIR) -p $(ETCDIR)
	-$(MV) $(ETCDIR)/ldapfriendly $(ETCDIR)/ldapfriendly-
	$(INSTALL) $(INSTALLFLAGS) -m 644 ldapfriendly $(ETCDIR)
	-$(MV) $(ETCDIR)/ldapfilter.conf $(ETCDIR)/ldapfilter.conf-
	$(INSTALL) $(INSTALLFLAGS) -m 644 ldapfilter.conf $(ETCDIR)
	-$(MV) $(ETCDIR)/ldaptemplates.conf $(ETCDIR)/ldaptemplates.conf-
	$(INSTALL) $(INSTALLFLAGS) -m 644 ldaptemplates.conf $(ETCDIR)
	-$(MV) $(ETCDIR)/ldapsearchprefs.conf $(ETCDIR)/ldapsearchprefs.conf-
	$(INSTALL) $(INSTALLFLAGS) -m 644 ldapsearchprefs.conf $(ETCDIR)

lint:	FORCE
	$(LINT) $(INCLUDES) $(DEFINES) $(SRCS)

5lint:	FORCE
	$(5LINT) $(INCLUDES) $(DEFINES) $(SRCS)

clean:	FORCE
	$(RM) $(OUTDIR)/libldap.a ltest ttest *.o DONE core a.out *.log version.c

depend:	FORCE
	$(MKDEP) $(INCLUDES) $(DEFINES) $(SRCS)

links:
	@$(LN) .src/*.[ch] .src/ldapfriendly .src/ldapfilter.conf \
		.src/ldaptemplates.conf .src/ldapsearchprefs.conf .

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

bind.o: bind.c ../pgpsockets/pgpSocketsLDAP.h
bind.o: ../../../pgpcdk/pub/include/pgpSockets.h
bind.o: ../../../pgpcdk/pub/include/pgpOptionList.h
bind.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
bind.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
bind.o: ../../../pgpcdk/pub/include/pgpTLS.h
bind.o: ../../../pgpcdk/pub/include/pgpErrors.h
bind.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
bind.o: ../include/ldap.h
open.o: open.c ../pgpsockets/pgpSocketsLDAP.h
open.o: ../../../pgpcdk/pub/include/pgpSockets.h
open.o: ../../../pgpcdk/pub/include/pgpOptionList.h
open.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
open.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
open.o: ../../../pgpcdk/pub/include/pgpTLS.h
open.o: ../../../pgpcdk/pub/include/pgpErrors.h
open.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
open.o: ../include/ldap.h
result.o: result.c ../include/lber.h
result.o: ../../../pgpcdk/pub/include/pgpOptionList.h
result.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
result.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
result.o: ../../../pgpcdk/pub/include/pgpTLS.h
result.o: ../../../pgpcdk/pub/include/pgpErrors.h
result.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
result.o: ../include/ldap.h
error.o: error.c ../pgpsockets/pgpSocketsLDAP.h
error.o: ../../../pgpcdk/pub/include/pgpSockets.h
error.o: ../../../pgpcdk/pub/include/pgpOptionList.h
error.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
error.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
error.o: ../../../pgpcdk/pub/include/pgpTLS.h
error.o: ../../../pgpcdk/pub/include/pgpErrors.h
error.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
error.o: ../include/ldap.h
compare.o: compare.c ../include/lber.h
compare.o: ../../../pgpcdk/pub/include/pgpOptionList.h
compare.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
compare.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
compare.o: ../../../pgpcdk/pub/include/pgpTLS.h
compare.o: ../../../pgpcdk/pub/include/pgpErrors.h
compare.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
compare.o: ../include/ldap.h
search.o: search.c ../pgpsockets/pgpSocketsLDAP.h
search.o: ../../../pgpcdk/pub/include/pgpSockets.h
search.o: ../../../pgpcdk/pub/include/pgpOptionList.h
search.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
search.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
search.o: ../../../pgpcdk/pub/include/pgpTLS.h
search.o: ../../../pgpcdk/pub/include/pgpErrors.h
search.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
search.o: ../include/ldap.h
modify.o: modify.c ../include/lber.h
modify.o: ../../../pgpcdk/pub/include/pgpOptionList.h
modify.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
modify.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
modify.o: ../../../pgpcdk/pub/include/pgpTLS.h
modify.o: ../../../pgpcdk/pub/include/pgpErrors.h
modify.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
modify.o: ../include/ldap.h
add.o: add.c ../pgpsockets/pgpSocketsLDAP.h
add.o: ../../../pgpcdk/pub/include/pgpSockets.h
add.o: ../../../pgpcdk/pub/include/pgpOptionList.h
add.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
add.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
add.o: ../../../pgpcdk/pub/include/pgpTLS.h
add.o: ../../../pgpcdk/pub/include/pgpErrors.h
add.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
add.o: ../include/ldap.h
modrdn.o: modrdn.c ../include/lber.h
modrdn.o: ../../../pgpcdk/pub/include/pgpOptionList.h
modrdn.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
modrdn.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
modrdn.o: ../../../pgpcdk/pub/include/pgpTLS.h
modrdn.o: ../../../pgpcdk/pub/include/pgpErrors.h
modrdn.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
modrdn.o: ../include/ldap.h
delete.o: delete.c ../include/lber.h
delete.o: ../../../pgpcdk/pub/include/pgpOptionList.h
delete.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
delete.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
delete.o: ../../../pgpcdk/pub/include/pgpTLS.h
delete.o: ../../../pgpcdk/pub/include/pgpErrors.h
delete.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
delete.o: ../include/ldap.h
abandon.o: abandon.c ../include/lber.h
abandon.o: ../../../pgpcdk/pub/include/pgpOptionList.h
abandon.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
abandon.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
abandon.o: ../../../pgpcdk/pub/include/pgpTLS.h
abandon.o: ../../../pgpcdk/pub/include/pgpErrors.h
abandon.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
abandon.o: ../include/ldap.h
ufn.o: ufn.c ../pgpsockets/pgpSocketsLDAP.h
ufn.o: ../../../pgpcdk/pub/include/pgpSockets.h
ufn.o: ../../../pgpcdk/pub/include/pgpOptionList.h
ufn.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
ufn.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
ufn.o: ../../../pgpcdk/pub/include/pgpTLS.h
ufn.o: ../../../pgpcdk/pub/include/pgpErrors.h
ufn.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
ufn.o: ../include/ldap.h
cache.o: cache.c ../../../pgpcdk/pub/include/pgpSockets.h
cache.o: ../../../pgpcdk/pub/include/pgpOptionList.h
cache.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
cache.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
cache.o: ../../../pgpcdk/pub/include/pgpTLS.h
cache.o: ../../../pgpcdk/pub/include/pgpErrors.h
cache.o: ../../../pfl/common/pgpPFLErrors.h ../include/proto-lber.h ldap-int.h
getfilter.o: getfilter.c ../pgpsockets/pgpSocketsLDAP.h
getfilter.o: ../../../pgpcdk/pub/include/pgpSockets.h
getfilter.o: ../../../pgpcdk/pub/include/pgpOptionList.h
getfilter.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
getfilter.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
getfilter.o: ../../../pgpcdk/pub/include/pgpTLS.h
getfilter.o: ../../../pgpcdk/pub/include/pgpErrors.h
getfilter.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
getfilter.o: ../include/ldap.h
regex.o: regex.c
sbind.o: sbind.c ../pgpsockets/pgpSocketsLDAP.h
sbind.o: ../../../pgpcdk/pub/include/pgpSockets.h
sbind.o: ../../../pgpcdk/pub/include/pgpOptionList.h
sbind.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
sbind.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
sbind.o: ../../../pgpcdk/pub/include/pgpTLS.h
sbind.o: ../../../pgpcdk/pub/include/pgpErrors.h
sbind.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
sbind.o: ../include/ldap.h
kbind.o: kbind.c
unbind.o: unbind.c ../../../pgpcdk/pub/include/pgpTLS.h
unbind.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
unbind.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
unbind.o: ../../../pgpcdk/pub/include/pgpSockets.h
unbind.o: ../../../pgpcdk/pub/include/pgpOptionList.h
unbind.o: ../../../pgpcdk/pub/include/pgpErrors.h
unbind.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
unbind.o: ../include/ldap.h
friendly.o: friendly.c ../pgpsockets/pgpSocketsLDAP.h
friendly.o: ../../../pgpcdk/pub/include/pgpSockets.h
friendly.o: ../../../pgpcdk/pub/include/pgpOptionList.h
friendly.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
friendly.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
friendly.o: ../../../pgpcdk/pub/include/pgpTLS.h
friendly.o: ../../../pgpcdk/pub/include/pgpErrors.h
friendly.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
friendly.o: ../include/ldap.h
cldap.o: cldap.c
free.o: free.c ../pgpsockets/pgpSocketsLDAP.h
free.o: ../../../pgpcdk/pub/include/pgpSockets.h
free.o: ../../../pgpcdk/pub/include/pgpOptionList.h
free.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
free.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
free.o: ../../../pgpcdk/pub/include/pgpTLS.h
free.o: ../../../pgpcdk/pub/include/pgpErrors.h
free.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
free.o: ../include/ldap.h
disptmpl.o: disptmpl.c ../../../pgpcdk/pub/include/pgpSockets.h
disptmpl.o: ../../../pgpcdk/pub/include/pgpOptionList.h
disptmpl.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
disptmpl.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
disptmpl.o: ../../../pgpcdk/pub/include/pgpTLS.h
disptmpl.o: ../../../pgpcdk/pub/include/pgpErrors.h
disptmpl.o: ../../../pfl/common/pgpPFLErrors.h ../include/ldap.h
srchpref.o: srchpref.c ../../../pgpcdk/pub/include/pgpSockets.h
srchpref.o: ../../../pgpcdk/pub/include/pgpOptionList.h
srchpref.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
srchpref.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
srchpref.o: ../../../pgpcdk/pub/include/pgpTLS.h
srchpref.o: ../../../pgpcdk/pub/include/pgpErrors.h
srchpref.o: ../../../pfl/common/pgpPFLErrors.h ../include/ldap.h
dsparse.o: dsparse.c ../pgpsockets/pgpSocketsLDAP.h
dsparse.o: ../../../pgpcdk/pub/include/pgpSockets.h
dsparse.o: ../../../pgpcdk/pub/include/pgpOptionList.h
dsparse.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
dsparse.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
dsparse.o: ../../../pgpcdk/pub/include/pgpTLS.h
dsparse.o: ../../../pgpcdk/pub/include/pgpErrors.h
dsparse.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
dsparse.o: ../include/ldap.h
tmplout.o: tmplout.c ../../../pgpcdk/pub/include/pgpSockets.h
tmplout.o: ../../../pgpcdk/pub/include/pgpOptionList.h
tmplout.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
tmplout.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
tmplout.o: ../../../pgpcdk/pub/include/pgpTLS.h
tmplout.o: ../../../pgpcdk/pub/include/pgpErrors.h
tmplout.o: ../../../pfl/common/pgpPFLErrors.h ../include/ldap.h
sort.o: sort.c ../include/lber.h ../../../pgpcdk/pub/include/pgpOptionList.h
sort.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
sort.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
sort.o: ../../../pgpcdk/pub/include/pgpTLS.h
sort.o: ../../../pgpcdk/pub/include/pgpErrors.h
sort.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
sort.o: ../include/proto-ldap.h
getdn.o: getdn.c ../pgpsockets/pgpSocketsLDAP.h
getdn.o: ../../../pgpcdk/pub/include/pgpSockets.h
getdn.o: ../../../pgpcdk/pub/include/pgpOptionList.h
getdn.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
getdn.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
getdn.o: ../../../pgpcdk/pub/include/pgpTLS.h
getdn.o: ../../../pgpcdk/pub/include/pgpErrors.h
getdn.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
getdn.o: ../include/ldap.h
getentry.o: getentry.c ../pgpsockets/pgpSocketsLDAP.h
getentry.o: ../../../pgpcdk/pub/include/pgpSockets.h
getentry.o: ../../../pgpcdk/pub/include/pgpOptionList.h
getentry.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
getentry.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
getentry.o: ../../../pgpcdk/pub/include/pgpTLS.h
getentry.o: ../../../pgpcdk/pub/include/pgpErrors.h
getentry.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
getentry.o: ../include/ldap.h
getattr.o: getattr.c ../pgpsockets/pgpSocketsLDAP.h
getattr.o: ../../../pgpcdk/pub/include/pgpSockets.h
getattr.o: ../../../pgpcdk/pub/include/pgpOptionList.h
getattr.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
getattr.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
getattr.o: ../../../pgpcdk/pub/include/pgpTLS.h
getattr.o: ../../../pgpcdk/pub/include/pgpErrors.h
getattr.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
getattr.o: ../include/ldap.h
getvalues.o: getvalues.c ../pgpsockets/pgpSocketsLDAP.h
getvalues.o: ../../../pgpcdk/pub/include/pgpSockets.h
getvalues.o: ../../../pgpcdk/pub/include/pgpOptionList.h
getvalues.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
getvalues.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
getvalues.o: ../../../pgpcdk/pub/include/pgpTLS.h
getvalues.o: ../../../pgpcdk/pub/include/pgpErrors.h
getvalues.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
getvalues.o: ../include/ldap.h
addentry.o: addentry.c ../pgpsockets/pgpSocketsLDAP.h
addentry.o: ../../../pgpcdk/pub/include/pgpSockets.h
addentry.o: ../../../pgpcdk/pub/include/pgpOptionList.h
addentry.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
addentry.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
addentry.o: ../../../pgpcdk/pub/include/pgpTLS.h
addentry.o: ../../../pgpcdk/pub/include/pgpErrors.h
addentry.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
addentry.o: ../include/ldap.h
request.o: request.c ../include/lber.h
request.o: ../../../pgpcdk/pub/include/pgpOptionList.h
request.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
request.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
request.o: ../../../pgpcdk/pub/include/pgpTLS.h
request.o: ../../../pgpcdk/pub/include/pgpErrors.h
request.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
request.o: ../include/ldap.h
getdxbyname.o: getdxbyname.c
os-ip.o: os-ip.c ../../../pgpcdk/pub/include/pgpSockets.h
os-ip.o: ../../../pgpcdk/pub/include/pgpOptionList.h
os-ip.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
os-ip.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
os-ip.o: ../../../pgpcdk/pub/include/pgpTLS.h
os-ip.o: ../../../pgpcdk/pub/include/pgpErrors.h
os-ip.o: ../../../pfl/common/pgpPFLErrors.h ../include/proto-ldap.h
url.o: url.c ../pgpsockets/pgpSocketsLDAP.h
url.o: ../../../pgpcdk/pub/include/pgpSockets.h
url.o: ../../../pgpcdk/pub/include/pgpOptionList.h
url.o: ../../../pgpcdk/pub/include/pgpPubTypes.h
url.o: ../../../pgpcdk/unix/pgpConfig.h ../../../pfl/common/pgpBase.h
url.o: ../../../pgpcdk/pub/include/pgpTLS.h
url.o: ../../../pgpcdk/pub/include/pgpErrors.h
url.o: ../../../pfl/common/pgpPFLErrors.h ../include/pgpSrvStd.h
url.o: ../include/ldap.h
charset.o: charset.c

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
#
# Do NOT edit this file -- it is automatically appended to all Makefiles
# except the LDAP top-level Makefile. See the LDAP INSTALL file for more
# information.
#
#-----------------------------------------------------------------------------
# Copyright (c) 1994 Regents of the University of Michigan.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that this notice is preserved and that due credit is given
# to the University of Michigan at Ann Arbor. The name of the University
# may not be used to endorse or promote products derived from this
# software without specific prior written permission. This software
# is provided ``as is'' without express or implied warranty.
#
#       LDAP standard Make defines (appended to all but top-level Makefile)
#
#-----------------------------------------------------------------------------

# DEFS are included in CFLAGS
DEFS	= $(PLATFORMCFLAGS) $(LDAP_DEBUG) $(KERBEROS) $(AFSKERBEROS) \
		$(UOFM) $(UOFA) $(NO_USERINTERFACE) $(CLDAP) $(NO_CACHE) \
		$(LDAP_REFERRALS) $(LDAP_DNS) $(STR_TRANSLATION) \
		$(LIBLDAP_CHARSETS) $(LIBLDAP_DEF_CHARSET) \
		$(SLAPD_BACKENDS) $(LDBMBACKEND) $(LDBMINCLUDE) $(PHONETIC)

# SERVERDEFS are added to server builds CFLAGS (in addition to DEFS)
SERVERDEFS	= $(ISODEPACKAGE) $(ICRELEASE) $(LDAP_USE_PP) \
			$(NO_SETPROCTITLE) $(PEPSY_DUMP)

#
# ISODELIBS are used in server/ldapd builds
#
ISODELIBS	= $(ISODEBASELIBS) -lm $(ISODETURBOLIBS)

# ACFLAGS are added to CFLAGS but not passed to mkdep, lint, etc
ACFLAGS		= $(EXTRACFLAGS) $(UNPROTOCFLAGS)

# ALDFLAGS are always placed near the beginning of all linker (cc -o) commands
ALDFLAGS	= $(EXTRALDFLAGS) $(PLATFORMLDFLAGS)

# ALIBS are always placed at the end of all linker (cc -o) commands
ALIBS		= $(PLATFORMLIBS) $(THREADSLIB)

#
# default definitions for Unix utilities (may be over-ridden in Make-platform)
CC	= cc
MAKE	= make
RANLIB	= ranlib
AR	= ar
RM	= rm -f
MV	= mv -f
CP	= cp
CHMOD	= chmod
CAT	= cat
LN	= ln -s
HARDLN	= ln
TAIL	= tail
SED	= sed
LINT	= lint
5LINT	= lint
MKDIR	= mkdir
INSTALL	= install
INSTALLFLAGS = -c
BASENAME= basename
DIRNAME	= dirname
MKDEP	= $(LDAPSRC)/build/mkdep -c gcc -s -f Make-template
PWD	= pwd
DATE	= date
HOSTNAME= hostname


#
# empty target used to force rules to be invoked
#
FORCE:

#
# LDAP SunOS5 GNU C Make-platform file
#

#
# add any platform-specific overrides below here
#


#
# -------------------------------------------------------------------------
# you will probably not need to edit anything below this point
# -------------------------------------------------------------------------
CC	= gcc

# ranlib not needed under SunOS5
RANLIB	= ""

# give full path to hostname since it may not be in user's path
HOSTNAME=/usr/ucb/hostname

# don't count on /usr/ucb/install being present or first in path
INSTALL=$(LDAPSRC)/build/install.sh

PLATFORMCFLAGS=	-Dsunos5 -D_REENTRANT
PLATFORMLIBS= -lsocket -lnsl -lgen
THREADS= -DTHREAD_SUNOS5_LWP
THREADSLIB=-lthread
#-----------------------------------------------------------------------------
# Copyright (c) 1997 PGP inc.
# All rights reserved.
#
#-----------------------------------------------------------------------------
#############################################################################
## Edit the following variables to have appropriate values for your system ##
#############################################################################

# the following causes problems for Sun make (elkins)

#vpath %.a $(LDIR):$(PGPCDKDIR)/unix:$(DBLDIR):$(PGPCDKDIR)/priv/keyserver: \
#	  $(PGPCDKDIR)/../network/pgpsockets/unix/src

#############################################################################
## LDAP install paths                                                      ##
#############################################################################
#
# by default, everything is installed below INSTROOT
# servers, config files, etc. are put in ETCDIR
# include files get put in INCLUDEDIR
# libraries are put in LIBDIR
# man pages are put under MANDIR
# programs end-users will run are put in BINDIR
#
# Moved INSTROOT declaration to top-level Makefile
# useful for automation.

ETCDIR= $(INSTROOT)/etc
INCLUDEDIR= $(INSTROOT)/include
LIBDIR= $(INSTROOT)/unix
MANDIR= $(INSTROOT)/doc
BINDIR= $(INSTROOT)/unix

#
# if you want things to run in a different directory from where they
# are installed, set this accordingly (this path gets compiled into a
# few binaries). otherwise, leave it alone.
RUNTIMEETCDIR= $(ETCDIR)

#############################################################################
## General compiler options                                                ##
#############################################################################
# Passed to every compile (cc or gcc).  This is where you put -O or -g, etc.
# Added -D_REENTRANT per ldap doc. for Solaris builds <galileo>
OPT=-g
DEBUG=1
UNFINISHED=1
EXTRACFLAGS=-D_REENTRANT $(OPT) -fwritable-strings \
            -DPGP_DEBUG=$(DEBUG) -DUNFINISHED_CODE_ALLOWED=$(UNFINISHED) \
	    -DPGPSOCKETSLDAP=1

# Passed to every link (ld).  Include -g here if you did in EXTRACFLAGS.

EXTRALDFLAGS=$(OPT)

#############################################################################
## ISODE is required ONLY to build the ldap <-> X.500 server (ldapd)       ##
## If you don't want to build it, you can skip this section.               ##
#############################################################################
#
# To build the ldap server, uncomment the HAVEISODE line,
# and the section describing build settings for your version of isode.
#
HAVEISODE = yes

#############################################################################
## If you don't want to run slapd, skip this section.                      ##
#############################################################################
#
# To build slapd (the stand-alone ldap daemon), uncomment the MAKESLAPD
# line and select the SLAPD_BACKENDS you want to use. If you enable the
# LDBM backend, also select one of the LDBM backends.
MAKESLAPD= yes
#
# remove the defines for backends you don't want to enable 
SLAPD_BACKENDS= -DLDAP_LDBM 
#
# If you have included -DLDAP_LDBM in the SLAPD_BACKENDS line you need
# to specify which low-level database package to use.  There are
# four choices: Berkeley db b-tree, Berkeley db hash, GNU dbm, or ndbm.
#
# berkeley db btree package
LDBMBACKEND=-DLDBM_USE_DBBTREE21
LDBMINCLUDE=
LDBMLIB=-ldb
# berkeley db hash package
#LDBMBACKEND=-DLDBM_USE_DBHASH
#LDBMINCLUDE=-I/usr/local/db/include
#LDBMLIB=-ldb
# gnu dbm (gdbm)
#LDBMBACKEND=-DLDBM_USE_GDBM
#LDBMINCLUDE=-I$(INSTROOT)/servers/slapd/db
#LDBMLIB=-lgdbm
# standard unix ndbm
#LDBMBACKEND=-DLDBM_USE_NDBM
#
# if you want to use a non-default threads package change these lines
#THREADS=-DNO_THREADS
#THREADSLIB=

#############################################################################
## General options                                                         ##
#############################################################################
# uncomment this line to enable debugging code (a good idea)
LDAP_DEBUG=-DLDAP_DEBUG

# uncomment this line to turn on a few U of Michigan specific things
#UOFM=-DUOFM

# uncomment this line to delete a few printfs in the lber and ldap libraries.
#NO_USERINTERFACE=-DNO_USERINTERFACE

# uncomment this line to include Connectionless LDAP support
#CLDAP=-DCLDAP

# uncomment this line to eliminate local caching support in the libldap
#NO_CACHE=-DNO_CACHE

# uncomment this line to enable support for LDAP referrals in libldap
#LDAP_REFERRALS=-DLDAP_REFERRALS

# uncomment this line to use soundex for approximate matches in slapd.
# the default is to use the metaphone algorithm.
#PHONETIC=-DSOUNDEX
