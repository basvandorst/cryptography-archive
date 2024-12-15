ARCH = none

TARGETS = xrunecpp xcheckcertif
BINDIR  = bin

MANFILES = runecpp.l checkcertif.l
MANDIR = /usr/man/manl

BNCFLAGS = 

BMOFLAGS = -O
BMDFLAGS =
BMFLAGS = OFLAGS=$(BMOFLAGS) DFLAGS=$(BMDFLAGS)

ECPPOFLAGS = -O
ECPPDFLAGS = 
ECPPDLIB = 
ECPPFLAGS = OFLAGS=$(ECPPOFLAGS) DFLAGS=$(ECPPDFLAGS) DLIB=$(ECPPDLIB)

# Modify this if needed
MAILCMD = mail ecpp@fleurie.inria.fr
EDITOR = /usr/local/bin/emacs -nw
TMP = /tmp/mail2morain

default:
	@case "x$(TYPE)" in \
		x) make helpark;; \
		*) make ARCH=$(ARCH) TYPE=$(TYPE) all;; \
	esac

helpark:
	@echo "You must type:"
	@echo "   % make TYPE=YourType ARCH=YourName"
	@echo "where TYPE is the type of code for the BigNum package, among:"
	@echo ""
	@echo "  68K    : Sun3, etc."
	@echo "  sony68K"
	@echo "  sparc  : Sun 4, etc."
	@echo "  mips   : Dec Stations, etc."
	@echo "  vax"
	@echo "  pyramid"
	@echo "  ns     : Sequent machines"
	@echo "  i960   : Intel 960"
	@echo "  C32    : for none of the above, but using 32 bits"
	@echo ""
	@echo "For more informations, see the Makefile in ./Bignum/distrib"
	@echo ""
	@echo "ARCH is a name used to differentiate between machines"
	@echo "with the same processor."
	@echo "For example, on a Sun 3, type:"
	@echo "   % make TYPE=68K ARCH=sun3"
	@echo "and on a Dec3100"
	@echo "   % make TYPE=mips ARCH=ds3100"
	@echo ""
	@echo "For more informations, read the long instructions in the"
	@echo "file README."

all: bignum bigmod ecpp bin

bignum:
	- cd Bignum; mkdir $(ARCH);\
	make ARCH=$(ARCH) TYPE=$(TYPE) all

bigmod:
	- cd Bigmod; mkdir $(ARCH);\
	make ARCH=$(ARCH) $(BMFLAGS) abigmod

ecpp:
	- cd Ecpp/C; mkdir $(ARCH) ;\
	make ARCH=$(ARCH) $(ECPPFLAGS) arunecpp;\
	make ARCH=$(ARCH) $(ECPPFLAGS) acheckcertif
	
bin:
	@echo "Building scripts xrunecpp and xcheckcertif using the path:"
	@echo "     `pwd`/Ecpp/"
	@echo "Edit xrunecpp and xcheckcertif if needed"
	@echo "#!/bin/sh -" > xrunecpp
	@echo "arch=$(ARCH)" >> xrunecpp
	@echo "path=`pwd`/Ecpp" >> xrunecpp
	@echo "\$$path/C/\$$arch/runecpp -l\$$path/ \$$*" >> xrunecpp
	@chmod +x xrunecpp
	@echo "#!/bin/sh -" > xcheckcertif
	@echo "arch=$(ARCH)" >> xcheckcertif
	@echo "path=`pwd`/Ecpp" >> xcheckcertif
	@echo "\$$path/C/\$$arch/checkcertif \$$*" >> xcheckcertif
	@chmod +x xcheckcertif

install:
	- cd Ecpp/C/man;\
	for i in $(MANFILES) ;\
	do \
	  install -c $$i $(MANDIR) ;\
	done
	-mkdir $(BINDIR)
	for i in ${TARGETS} ;\
	do \
	  install -c $$i ${BINDIR} ;\
	done

clean:
	- cd Bignum; make ARCH=$(ARCH) aclean
	- cd Bigmod; make ARCH=$(ARCH) aclean
	- cd Ecpp/C; make ARCH=$(ARCH) aclean
	- /bin/rm $(TARGETS)

checkdirlist:
	ls -R > foo; diff foo dirlist

test:
	@echo "Running test on n1-50.input"
	@-xrunecpp -Q -f Ecpp/C/Ex/n1-50.input > foo ;\
	if [ "x`diff foo Ecpp/C/Ex/n1-50.res`" != "x" ] ;\
	then \
		echo "Error in test" ;\
		diff foo Ecpp/C/Ex/n1-50.res ;\
	else true; fi ;\
	/bin/rm foo

report:
	@echo "Implementation report for `cat VERSION.ecpp`" > $(TMP)
	@echo "Machine: " >> $(TMP)
	@echo "TYPE = " >> $(TMP)
	@echo "ARCH = " >> $(TMP)
	@echo "" >> $(TMP)
	@echo "Time for xrunecpp -q -f Ecpp/C/Test/test.data:" >> $(TMP)
	$(EDITOR) $(TMP)
	$(MAILCMD) < $(TMP)
	/bin/rm $(TMP)

bugreport:
	@echo "Bug in Version `cat VERSION.ecpp`" > $(TMP)
	@echo "Machine: " >> $(TMP)
	@echo "TYPE = " >> $(TMP)
	@echo "ARCH = " >> $(TMP)
	@echo "" >> $(TMP)
	@echo "Description of the bug" >> $(TMP)
	$(EDITOR) $(TMP)
	$(MAILCMD) < $(TMP)
	/bin/rm $(TMP)
