#  Copyright     Digital Equipment Corporation & INRIA     1988, 1989
#  Last modified_on Thu Feb 14 17:08:11 GMT+1:00 1991 by herve 
#       modified_on Fri Jan 25 23:39:09 GMT+1:00 1991 by shand 
#       modified_on Thu Nov  2 14:23:14 GMT+1:00 1989 by gangnet 
#       modified_on Wed Jul  5 10:23:54 GMT+2:00 1989 by bertin 

CC = cc
LIB = BigNum.a
OBJECT = o/KerN.o o/bnInit.o o/bnDivide.o o/bnCmp.o o/bzf.o o/bz.o 
KERNH = h/BigNum.h 
CFLAGS = -c -I./h -O #-g -pg
LKFLAGS = -O #-g

# Original Makefile would like this
# TARGETS = testKerN bztest
# but I prefer that
TARGETS = BigNum.a

# extra entries:
# all     - make all the stuff
# tidy    - cleanup directories
# scratch - start from scratch

default:
	@echo "Usage: make <version>"
	@echo "see README for valid versions."
	@sh -c 'exit 1'

all: $(TARGETS)
	echo All is done !

tidy:
	-rm -f ,* .,* *~ #*# .emacs_[0-9]* *.BAK *.CKP core a.out
	-rm -f */,* */.,* */*~ */#*# */.emacs_[0-9]* */*.BAK */*.CKP

scratch:tidy 
	rm -f o/*.o

# build the BigNum library
$(LIB): $(OBJECT)
	-rm -f $(LIB)
	ar cr $(LIB) $(OBJECT)
	ranlib $(LIB)

# How to choose the machine dependante version. All produce KerN.o
o/KerN.o: c/KerN.c Makefile
	echo "The Default is KerN written in C with digits on 32 bits"
	make C32 CC=$(CC) CFLAGS=$(CFLAGS) 

C16: scratch
	$(CC) $(CFLAGS) -DDIGITon16BITS c/KerN.c
	mv KerN.o o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon16BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all

C32: scratch
	$(CC) $(CFLAGS) -DDIGITon32BITS c/KerN.c
	mv KerN.o o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all

68K: scratch
	as s/68KerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all

sony68K: scratch
	as s/68KerN_sony.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all

vax: scratch
	as s/vaxKerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" all

ns: scratch
	as s/nsKerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all

mips: scratch
	as -w s/mipsKerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" all

pyramid: scratch
	as s/pyramidKerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all

i960: scratch
	as s/i960KerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all
sparc: scratch
	as s/sparcKerN.s -o o/KerN.o
	make CC=$(CC) CFLAGS="$(CFLAGS) -DDIGITon32BITS" \
	     OBJECT="$(OBJECT) o/bnMult.o" all


# Construct VMS assembler from UNIX version

s/vaxKerN.mar: s/vaxKerN.s
	sed -f s/unix2vms.sed < s/vaxKerN.s > $@

# Level N
o/bnInit.o: c/bn/bnInit.c $(KERNH)
	$(CC) $(CFLAGS) c/bn/bnInit.c
	mv bnInit.o o

o/bnMult.o: c/bn/bnMult.c $(KERNH)
	$(CC) $(CFLAGS) c/bn/bnMult.c
	mv bnMult.o o

o/bnDivide.o: c/bn/bnDivide.c $(KERNH)
	$(CC) $(CFLAGS) c/bn/bnDivide.c
	mv bnDivide.o o

o/bnCmp.o: c/bn/bnCmp.c $(KERNH)
	$(CC) $(CFLAGS) c/bn/bnCmp.c
	mv bnCmp.o o

# Level Z
o/bz.o: c/bz.c h/BigZ.h $(KERNH)
	$(CC) $(CFLAGS) c/bz.c
	mv bz.o o

# Some functions build with BigZ
o/bzf.o: c/bzf.c h/BigZ.h $(KERNH)
	$(CC) $(CFLAGS) c/bzf.c
	mv bzf.o o

# Tests Of KerN
testKerN: Makefile o/testKerN.o $(LIB)
	$(CC) o/testKerN.o $(LIB) $(LKFLAGS) -o testKerN

o/testKerN.o: c/testKerN.c $(KERNH) h/BntoBnn.h
	$(CC) $(CFLAGS) c/testKerN.c
	mv testKerN.o o

# Tests Of BigZ
bztest: o/bztest.o $(LIB)
	$(CC) o/bztest.o $(LIB) $(LKFLAGS) -o bztest

o/bztest.o: c/bztest.c h/BigZ.h $(KERNH)
	$(CC) $(CFLAGS) c/bztest.c
	mv bztest.o o

# documentation
doc: doc/bn.ps doc/bnf.ps 
docprl: doc/bnprl.ps

doc/bn.dvi: doc/bn.tex doc/bnbody.tex
	cd doc;\
	latex bn;\
	makeindex bn;\
	sed -e "s/\\item Bz/\\newpage \\Bz/g" < bn.ind > bn.index;\
	mv bn.index bn.ind;\
	latex bn;\
	cd ..

doc/bn.ps: doc/bn.dvi
	cd doc;\
	dvips -o bn.ps bn;\
	cd ..

doc/bnf.dvi: doc/bnf.tex
	cd doc;\
	latex bnf;\
	cd ..

doc/bnf.ps: doc/bnf.dvi
	cd doc;\
	dvips -o bnf.ps bnf;\
	cd ..

doc/bnprl.dvi: doc/bnprl.tex doc/bnbody.tex
	cd doc;\
	latex bnprl;\
	makeindex bnprl;\
	sed -e "s/\\item Bz/\\newpage \\Bz/g" < bnprl.ind > bnprl.index;\
	mv bnprl.index bnprl.ind;\
	latex bnprl;\
	cd ..

doc/bnprl.ps: doc/bnprl.dvi
	cd doc;\
	dvips -o bnprl.ps bnprl;\
	cd ..


# build shell archives
PACKET_SIZE=90
DIRS= c c/bn h s o doc

# If you modify the list of files in the package kit, don't forget 
# to update README.

KIT=  README Makefile VMSmakefile MSDOSmakefile\
      doc/bn.tex doc/bnbody.tex doc/bnf.tex doc/intro\
      c/KerN.c c/bn/bnInit.c c/bn/bnMult.c c/bn/bnDivide.c\
      c/bn/bnCmp.c c/bz.c c/bzf.c \
      c/bztest.c c/testKerN.c \
      h/BigNum.h h/BigZ.h h/BntoBnn.h \
      s/vaxKerN.s s/68KerN.s s/nsKerN.s s/68KerN_mot.s \
      s/mipsKerN.s s/pyramidKerN.s s/vaxKerN.mar s/unix2vms.sed \
      s/i960KerN.s s/sparcKerN.s \
      o/EMPTY

bignum01: $(KIT)
	makekit -s$(PACKET_SIZE)k -nbignum -t"Now do 'make'" $(DIRS) $(KIT)


# build shell archives of the beta version

# If you modify the list of files in the package betakit, don't forget 
# to update betaREADME.

BETAKIT= $(KIT)  

# note we replace README by betaREADME and doc/intro by doc/betaintro
# such that the filename will be README and doc/intro in the archive, 
betabignum01: $(BETAKIT) betaREADME doc/betaintro
	mv README README.cur 
	cp betaREADME README
	mv doc/intro doc/intro.cur  
	cp doc/betaintro doc/intro 
	makekit -s$(PACKET_SIZE)k -nbignum -t"Now do 'make'" $(DIRS) $(BETAKIT)
	mv README.cur README
	mv doc/intro.cur doc/intro 


# send shell archives
SENDMAIL=/usr/lib/sendmail
SENDMAILFLAGS=
USER=nil
FULLNAME=nil
COPY=librarian@prl.dec.com
VERSION=    # the version you want to mail, could be empty (current release) or "beta"

mail: $(VERSION)bignum01
	@sh -c "if [ x$(USER) = xnil -o 'x$(FULLNAME)' = xnil ]; \
	    then echo must specify USER and FULLNAME; \
	         echo EG make USER=herve@prl FULLNAME="'\"'"J-C Herve, Digital PRL"'\"'" mail; exit 1; \
	         else :; fi"
	@touch Recipients
	@echo '' >> Recipients
	@date >> Recipients
	@echo "$(FULLNAME)" >> Recipients
	@echo '<'"$(USER)"'>' >> Recipients
	@echo "To: $(COPY)" >tosend
	@echo "Subject: BIGNUM DAEMON" >>tosend
	@echo "Jean-Christophe, I have sent the package bignum to:" >>tosend
	@echo >>tosend
	@echo "    $(FULLNAME)" >>tosend
	@echo "    $(USER)" >>tosend
	@echo >>tosend
	@echo "Thanks to register this address in your distribution list." >>tosend
	@$(SENDMAIL) $(SENDMAILFLAGS) $(COPY) <tosend
	echo To: $(USER) > sendmail_header
	cp sendmail_header tosend
	echo "Subject: BigNum package from Digital PRL" >>tosend
	ls bignum[0-9][0-9] | sed -e "s/^bignum0*/    BigNum - Part /" > _flist1
	ls bignum[0-9][0-9] | sed -e "s/^/    /" > _flist2
	ls bignum[0-9][0-9] | sed -e "s/^/    \/bin\/sh /" > _flist3
	ls bignum[0-9][0-9] | sed -e "s/^/    shar -u /" > _flist4
	/lib/cpp -Uvax -I. doc/$(VERSION)intro >intro.tosend
	sed \
		-e "s/modified_on/modified_on/g" \
		-e "/doc\/$(VERSION)intro/d" \
		-e "/.\/$(VERSION)README/d" \
		-e "s/NN/"`ls bignum?? | wc -l | sed -e "s/ //g"`"/g" \
		-e "/^INCLUDE1/r _flist1" \
		-e "/^INCLUDE2/r _flist2" \
		-e "/^INCLUDE3/r _flist3" \
		-e "/^INCLUDE4/r _flist4" \
		-e "/^INCLUDE./d" \
		<intro.tosend >>tosend
	$(SENDMAIL) $(SENDMAILFLAGS) $(USER) <tosend
	for i in `ls bignum[0-9][0-9]`; \
	    do cp sendmail_header tosend; \
	    echo $$i | sed -e "s/^bignum0*/Subject: BigNum - Part /" >>tosend; \
	    echo "# Remove all text above and including this line." >>tosend; \
	    sed -e "s/modified_on/modified_on/g" <$$i >>tosend; \
	    $(SENDMAIL) $(SENDMAILFLAGS) $(USER) <tosend; \
	    done
	rm -f sendmail_header tosend _flist[1-9] bignum[0-9][0-9] intro.tosend

