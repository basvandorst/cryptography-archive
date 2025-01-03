#
# You will usually NOT need to edit this file at all:  instead, edit the
# Make-common file.  See the LDAP INSTALL file for more information.
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
#       LDAP lightweight X.500 Directory access top level makefile
#
#-----------------------------------------------------------------------------
#
############################################################################
#                                                                          #
# Usually you will not need to edit anything in this file                  #
#                                                                          #
############################################################################
#
# Note that these definitions of standard Unix utilities are only used
# in this Makefile.  The Make-common (and .make-platform) files have a
# similar set of definitions that are used in all the other LDAP Makefiles.
#
RM=rm -f
MV=mv -f
CP=cp
CAT=cat
PWD=pwd
TAIL=tail
CHMOD=chmod
FIND=find
SED=sed
LN=ln -s
MKDIR=mkdir
GREP=grep
DIRNAME=dirname
BASENAME=basename
TAR=tar
COMPRESS=compress
CO=co
CI=ci
UNIX_BIN=unix
PGPLIBSDIR = ../../libs

SRCDIRS= liblber libldap

TESTDIR=

#
# rules to make the software
#
# makedb
all: makeconfig
	@echo "making all..."
	@for i in $(SRCDIRS); do \
	    echo; echo "  cd $$i; $(MAKE) $(MFLAGS) all"; \
	    ( cd $$i; $(MAKE) $(MFLAGS) all ); \
	done

world: all

#
# rules to install the software
#

install:	makeconfig
	@for i in $(SRCDIRS); do \
	    echo; echo "cd $$i; $(MAKE) $(MFLAGS) install"; \
	    ( cd $$i; $(MAKE) $(MFLAGS) install ); \
	done

inst-lib:	makeconfig
	@echo "cd libraries; $(MAKE) $(MFLAGS) install"
	@( cd libraries; $(MAKE) $(MFLAGS) install )


#
# rules to test the LDAP software
#
test:	all
	@echo " cd $(TESTDIR); $(MAKE) $(MFLAGS) all"; \
	( cd $(TESTDIR); $(MAKE) $(MFLAGS) all );

#
# rules to make clean
#

clean:	FORCE
	@if [ -f .makefiles ]; then \
	    for i in $(SRCDIRS) $(TESTDIR); do \
		echo; echo "cd $$i; $(MAKE) $(MFLAGS) clean"; \
		( cd $$i; $(MAKE) $(MFLAGS) clean ); \
	    done; \
	fi; \
	( for d in ./obj-*; do \
	    if [ $$d != "./obj-*" ]; then \
		( echo "making clean in $$d..."; \
		  cd $$d; $(MAKE) $(MFLAGS) clean; ) \
	    else \
		exit 0; \
	    fi; \
	done )

veryclean:	clean
	@echo; echo "cd build; $(MAKE) $(MFLAGS) -f Make-template veryclean"; \
	( cd build; $(MAKE) $(MFLAGS) -f Make-template veryclean ); \
	if [ -f .makefiles ]; then \
	    for i in $(SRCDIRS) $(TESTDIR); do \
		echo; echo "cd $$i; $(MAKE) $(MFLAGS) veryclean"; \
		( cd $$i; $(MAKE) $(MFLAGS) veryclean ); \
	    done; \
	    echo "finding and removing Makefiles..."; \
	    for i in `$(FIND) . -type d -print`; do \
		if [ -f $$i/Make-template ]; then \
		    echo "removing file $$i/Makefile"; \
		    $(RM) $$i/Makefile; \
		fi; \
	    done; \
	    echo "removing file .makefiles"; \
	    $(RM) .makefiles; \
	fi; \
	( for d in ./obj-*; do \
	    if [ $$d != "./obj-*" ]; then \
		echo "removing $$d..."; $(RM) -r $$d; \
	    else \
		exit 0; \
	    fi; \
	done ); \
	if [ -f .make-platform ]; then \
	    echo "removing link .make-platform"; \
	    $(RM) .make-platform; \
	else \
	    exit 0; \
	fi


#
# rules to make depend
#
#
depend:	makeconfig
	@echo "making depend everywhere"; \
	echo "  cd include; $(MAKE) $(MFLAGS) all"; \
		( cd include; $(MAKE) $(MFLAGS) all ); \
	for i in $(SRCDIRS); do \
	    echo; echo "cd $$i; $(MAKE) $(MFLAGS) depend"; \
	    ( cd $$i; $(MAKE) $(MFLAGS) depend ); \
	done; \
	$(MAKE) $(MFLAGS) makefiles

#
# rule to force check for change of platform
#
platform:	FORCE
	@if [ -f .make-platform ]; then \
	    echo "removing old link .make-platform"; \
	    $(RM) .make-platform; \
	fi; \
	$(MAKE) $(MFLAGS) .make-platform


makeconfig:	.makefiles buildtools

.make-platform:
	@if [ -f /usr/bin/swconfig ]; then \
	    UNAME=./build/uname.sh; \
	elif [ -f /bin/uname ]; then \
	    UNAME=/bin/uname; \
	elif [ -f /usr/bin/uname ]; then \
	    UNAME=/usr/bin/uname; \
	else \
	    UNAME=./build/uname.sh; \
	fi; \
	if [ -z "$$UNAME" ]; then \
	    echo "unknown platform (no $$UNAME or /usr/bin/uname)"; \
	    echo "see the file  build/PORTS  for more information."; \
	    exit 1; \
	else \
	    OS=`$$UNAME -s` ; OSRELEASE=`$$UNAME -r` ; \
	    OSVERSION=`$$UNAME -v` ; \
	    case $$OS in \
	    SunOS) \
		if [ $$OSRELEASE -gt "5" -o $$OSRELEASE -lt "4" ]; then \
		    echo "SunOS release $$OSRELEASE unknown..."; exit 1; \
		fi; \
		if [ $$OSRELEASE -ge "5" ]; then \
		    PLATFORM="sunos5"; \
		else \
		    PLATFORM="sunos4"; \
		fi; \
		;; \
	    ULTRIX) \
		PLATFORM="ultrix" \
		;; \
	    OSF1) \
		PLATFORM="osf1" \
		;; \
	    AIX) \
		PLATFORM="aix" \
		;; \
	    HP-UX) \
		PLATFORM="hpux" \
		;; \
	    Linux) \
		PLATFORM="linux" \
		;; \
	    NetBSD) \
		PLATFORM="netbsd" \
		;; \
	    OpenBSD) \
		PLATFORM="openbsd" \
		;; \
	    FreeBSD) \
		PLATFORM="freebsd" \
		;; \
	    NeXTSTEP) \
		PLATFORM="nextstep" \
		;; \
	    SCO) \
		PLATFORM="sco" \
		;; \
	    IRIX|IRIX64) \
		PLATFORM="irix" \
		;; \
	    *) echo "unknown platform ($$OS $$OSVERSION $$OSRELEASE)..."; \
	       echo "see the file  build/PORTS  for more information."; \
		exit 1; \
		;; \
	    esac; \
	fi; \
	CC=$(CC); \
	OLDIFS="$$IFS"; \
	IFS=":"; \
	for dir in $$PATH; do \
	    if [ -f $$dir/gcc ]; then \
		CC=gcc; \
		break; \
	    fi; \
	done; \
	IFS="$$OLDIFS"; \
	$(LN) ./build/platforms/$$PLATFORM-$$CC/Make-platform .make-platform; \
	echo ""; \
	echo "** Set platform to $$PLATFORM with compiler $$CC..."; \
	echo ""

#
# rule to build Makefiles by concatenating Make-template file in each
# subdirectory with global Make-common, .make-platform, and
# build/Make-append files
#
.makefiles:	Make-common .make-platform build/Make-append
	@echo "making Makefiles..."; \
	HDRFILE=/tmp/Makehdr.$$$$; \
	DEFSFILE=/tmp/Makedefs.$$$$; \
	$(CAT) build/Make-append ./.make-platform ./Make-common > $$DEFSFILE; \
	echo "# --------------------------------------------------------" >  $$HDRFILE; \
	echo "#  This file was automatically generated.  Do not edit it."  >> $$HDRFILE; \
	echo "#  Instead, edit the Make-common file (located in the root"  >> $$HDRFILE; \
	echo "#  (of the LDAP distribution).  See the LDAP INSTALL file"   >> $$HDRFILE; \
	echo "#  for more information." >> $$HDRFILE; \
	echo "# --------------------------------------------------------" >> $$HDRFILE; \
	echo "#" >> $$HDRFILE; \
	for i in `$(FIND) . -type d -print`; do \
	    if [ -f $$i/Make-template ]; then \
		echo "  creating $$i/Makefile"; \
		$(RM) $$i/Makefile; \
		$(CAT) $$HDRFILE $$i/Make-template $$DEFSFILE > $$i/Makefile; \
		$(CHMOD) 444 $$i/Makefile; \
	    fi; \
	done; \
	$(RM) .makefiles; \
	touch .makefiles; \
	$(RM) $$HDRFILE $$DEFSFILE

#
# rule to always build makefiles
#
makefiles:	FORCE
	$(RM) .makefiles
	$(MAKE) $(MFLAGS) .makefiles

#
# rule to create any tools we need to build everything else
#
buildtools:	FORCE
	@echo "making buildtools"
	@echo "  cd build; $(MAKE) $(MFLAGS)"
	@( cd build; $(MAKE) $(MFLAGS) )

FORCE:
