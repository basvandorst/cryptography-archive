#
# Makefile for generating the "public" versions of the documentation
#
# 06-Mar-95 tjh    original "coding"
#
# Tim Hudson
# tjh@mincom.oz.au
#

# initial version Eric called SSLeay.doc.tar.gz
# then called the next on SSLeay.doc-1.1.tar.gz
# some I'm stuck with that sort of naming now --tjh

# 1.2 was the "second" version that went out mid June
# 1.3 is the "new" version timed with 0.4.4 and application patches
#     at the 0.4 level (18-Jul-95)

VERSION = 1.5
TARFILE = SSLeay.doc-$(VERSION).tar

COMPRESS = compress
COMPRESS = gzip

INSTALLTOP=/usr/local/www/document/ssl

# for the moment just test in my home area
INSTALLTOP=/home/tjh/www/ssl

DOCS=ssl.sdf mosaic.sdf telnet.sdf httpd.sdf certs.sdf \
     ramblings.sdf changes.sdf faq.sdf

OTHERS=makefile README README.apps COPYRIGHT

#PSDOCS=$(DOCS:.sdf=.ps)

PSDOCS=ssl.ps faq.ps

HTMLDOCS=$(DOCS:.sdf=.html)
RTXDOCS=$(DOCS:.sdf=.rtx)
HLPDOCS=$(DOCS:.sdf=.hlp)
MIFDOCS=$(DOCS:.sdf=.mif)
FVODOCS=$(DOCS:.sdf=.fvo)

.SUFFIXES: .sdf .html .ps .rtx .hlp .mif .fvo

#EXTRAS="-ssslstyle"

EXTRAS=

.rtx.hlp:
	cp $< $(HOME)/hlp/compile.rtx
	touch $(HOME)/.runsoftpc
	rsh garnet >/dev/null 2>&1
	cp $(HOME)/hlp/compile.hlp $(<:.rtx=.hlp)

.sdf.rtx:
	sdf2rtx $(EXTRAS) $<

.sdf.ps:
	sdf2ps -ftoc=3 $(EXTRAS) $<

.sdf.html:
	sdf2html $(EXTRAS) $<

.mif.rtx:
	mif2rtx $<

.sdf.fvo:
	sdf2fvo $(EXTRAS) $<

.sdf.mif:
	sdf2mif $(EXTRAS) $<

.mif.hlp:
	mif2rtx $<
	cp $< $(HOME)/hlp/compile.rtx
	touch $(HOME)/.runsoftpc
	rsh garnet >/dev/null 2>&1
	cp $(HOME)/hlp/compile.hlp $(<:.rtx=.hlp)

# all: $(PSDOCS) $(HTMLDOCS)

all: ps html

html: $(HTMLDOCS)

rtx: $(RTXDOCS)

hlp: $(HLPDOCS)

ps: $(PSDOCS)

fvo: $(FVODOCS)

mif: $(MIFDOCS)

install: installps installhtml 

installps: $(PSDOCS) 
	cp $? $(INSTALLTOP)
	(cd $(INSTALLTOP); chmod 664 $?)

installhtml: $(HTMLDOCS)
	cp $? $(INSTALLTOP)
	(cd $(INSTALLTOP); chmod 664 $?)

clean: 
	/bin/rm -f $(PSDOCS) $(HTMLDOCS) $(MIFDOCS) $(RTXDOCS) $(HLPDOCS)
	/bin/rm -f $(TARFILE) $(TARFILE).Z $(TARFILE).gz
	/bin/rm -f *.backup 

tar:
	/bin/rm -f $(TARFILE) $(TARFILE).Z $(TARFILE).gz
	tar cf $(TARFILE) $(DOCS) $(PSDOCS) $(HTMLDOCS) $(OTHERS)
	$(COMPRESS) $(TARFILE)

#ssl.sdf: certs.sdf httpd.sdf mosaic.sdf telnet.sdf

#all.sdf: certs.sdf httpd.sdf mosaic.sdf ssl.sdf telnet.sdf


