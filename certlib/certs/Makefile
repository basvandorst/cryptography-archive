include Makefile.defs

SUBDIRS = certs 
ALLDIRS	= bnlib certs 

all: bnlib $(SUBDIRS)

bnlib: Makefile
	@(cd $(@); echo "Building $(@)"; ./configure; make)

$(SUBDIRS): Makefile
	@(cd $(@); echo "Building $(@)"; make)

clean:
	@for comp in $(SUBDIRS) ; \
	do (cd $${comp}; echo "Cleaning $${comp}"; make clean); \
	done;
#	@-$(SCCSCLEAN)


cleanall:
	@for comp in $(ALLDIRS) ; \
	do (cd $${comp}; echo "Cleaning all $${comp}"; make cleanall); \
	done;
#	@-$(SCCSCLEAN)
