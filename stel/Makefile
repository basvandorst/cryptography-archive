#DIRS	= descore gmp-1.3.2 regex-0.12 libdes skey stel
DIRS	= gmp-1.3.2 regex-0.12 libdes skey stel
ETC	= COPYRIGHT LICENSE Makefile README stel-paper.ps man

all	:
	@echo Automatic building: make system-type. System types are:
	@echo
	@echo "   hpux sunos4 solaris24 solaris25 irix linux aix"
	@echo
	@echo "        and"
	@echo
	@echo "   ultrix freebsd bsdi"
	@echo
	@echo "Please note: the former systems have been tested by the authors"
	@echo "while the latter three systems have not been tested."
	@echo
	@echo Manual building: enter the following directories:
	@echo
	@echo "   $(DIRS)"
	@echo
	@echo examine Makefiles, configure Makefiles accordingly, and
	@echo do things by hand.  Only use this method if something goes
	@echo wrong with the automatic building.  Please report your fixes
	@echo to stel-authors@idea.sec.dsi.unimi.it.  Good luck\!

hpux sunos4 solaris24 solaris25 irix linux ultrix freebsd bsdi aix:
	[ -r "regex-0.12/Makefile" ] || \
	(cd regex-0.12; ./configure; make $@ )
	for i in gmp-1.3.2 libdes skey stel; \
	do \
		(cd $$i; make $@ ); \
	done

tar	:
	tar cfh stel-dist.tar $(ETC) $(DIRS)
	gzip -f stel-dist.tar

clean	:
	for i in $(DIRS); \
	do \
		( cd $$i; make $@); \
	done
