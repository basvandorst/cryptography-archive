!include <ntwin32.MAK>

TOP=		..
PGENDIR=	$(TOP)\Parser
PGEN=		$(PGENDIR)\pgen.exe
DESTH=		$(TOP)\Include\graminit.h
DESTC=		$(TOP)\Python\graminit.c

all:		install

install:	$(DESTH) $(DESTC)

$(DESTH):	graminit.h
		copy graminit.h $(DESTH)

$(DESTC):	graminit.c
		copy graminit.c $(DESTC)

graminit.c graminit.h: $(PGEN) Grammar
		$(PGEN) Grammar

$(PGEN):
		cd $(PGENDIR); make pgen

clean:
		-rm -f graminit.[ch]

