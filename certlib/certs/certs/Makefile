
# pragma ident "@(#)Makefile	1.7 96/01/29 Sun Microsystems"
#

include ../Makefile.defs
DIRS_SUNOS5	= lib hashcert utils
DIRS_SUNOS4	= lib hashcert utils 

DIR_X509:sh=			\
(				\
	if [ -d x509cert ];	\
	then echo "x509cert";	\
	fi;			\
)

SUBDIRS	= $(DIRS_SUNOS$(MAJOR_OS)) $(DIR_X509)


all: $(SUBDIRS)

$(SUBDIRS): Makefile
	@(cd $(@); echo "Building $(@)"; make)

clean:
	@for comp in $(SUBDIRS); \
	do (cd $${comp}; echo "Cleaning $${comp}"; make clean); \
	done;
	@-$(SCCSCLEAN)

cleanall:
	@for comp in $(SUBDIRS); \
	do \
	if [ -d $${comp} ]; \
	then \
		(cd $${comp}; echo "Cleaning all $${comp}"; make cleanall); \
	fi; \
	done;
	@-$(SCCSCLEAN)
