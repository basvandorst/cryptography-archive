#
# The command "make" produces a libprecision.a file only
#    "make bin" makes calc, pow, and factorial.
#
# -DASM 	- include machineop.s functions (not distributed) 
# -DASM_16BIT	- include 16-bit integer ASMOBJS functions (not distributed)
#
# Set TYPE to empty, -g for debug, -p for prof, -G for gprof, or -O for optim
#
CC	 = cc
TYPE	 =-g
#
# put -l to include local symbol definitions, -Aa for ANSI C
#
#CFLAGS	 = $(TYPE) -Aa -W a,-l
#CFLAGS	 = $(TYPE) -Aa -W a,-l -DASM_16BIT -DASM
ASFLAGS  = -l
CFLAGS	 = $(TYPE)
LDFLAGS  = $(TYPE)

SHELL	 = /bin/sh
LIBNAME  = libprecision
LIB 	 = $(LIBNAME)$(TYPE).a

RANDSRC	 = rand32.c
RANDOBJ  = rand32.o

#unset because I don't want factoring in the distributed calculator
#LIBF=../factor/libfactor${TYPE}.a

# unset because I don't want make to rebuild on minor mods
#HDR	 = precision.h
# unset because I don't want to use assembly code yet (see -DASM_16BIT)
#ASM16HDR = asm16bit.h

ASMOBJS = memlsrw.o memaddw.o memincw.o memsubw.o memdecw.o \
	  memmulw.o memmulw1.o memdivw.o memdivw1.o

ASMSRC  = memlsrw.s memaddw.s memincw.s memsubw.s memdecw.s	\
	  memmulw.s memmulw1.s memdivw.s  memdivw1.s


LIBSRC= \
atop.c	\
btop.c	\
errorp.c	\
itop.c	\
ltop.c	\
pabs.c	\
pneg.c	\
padd.c	\
pconst.c	\
pcmp.c	\
pdebug.c	\
pdivmod.c	\
pfloat.c	\
phalf.c	\
pio.c	\
pipow.c	\
plogb.c	\
pmax.c	\
pmin.c	\
pmul.c	\
podd.c	\
pops.c	\
ppow.c	\
ppowmod.c	\
prand.c	\
pshift.c	\
psqrt.c	\
pfact.c	\
psub.c	\
ptoa.c	\
ptob.c	\
ptoi.c	\
ptol.c	\
ptou.c	\
ptoul.c	\
ultop.c	\
utop.c \
pvec.c

HDR=	\
pcvt.h	\
pdefs.h 	\
precision.h

BINSRC=	\
calc.c	\
fastmultest.c	\
mathprof.c	\
mathtest.c	\
pfastmul.c	\
randtest.c	\
testmachineop.c	\
testptob.c	\
factorial.c	\
pow.c		\
powmod.c	\
pvinc.c

DOCSRC=	\
README		\
Documentation	\
Makefile	

SRC=	\
${DOCSRC}	\
${BINSRC}	\
${HDR}		\
${RANDSRC}	\
${LIBSRC}

OBJS =	\
plogb.o	\
ppowmod.o	\
ppow.o	\
pipow.o	\
psqrt.o	\
pfact.o	\
pio.o	\
pdebug.o	\
ptoa.o	\
atop.o	\
ptoi.o	\
ptou.o	\
ptol.o	\
ptoul.o	\
itop.o	\
utop.o	\
ltop.o	\
ultop.o	\
ptob.o	\
btop.o	\
podd.o	\
pmin.o	\
pmax.o	\
prand.o	\
pshift.o	\
phalf.o	\
pfloat.o	\
pconst.o	\
pcmp.o	\
pops.o	\
pdivmod.o	\
pmul.o	\
pabs.o	\
pneg.o	\
padd.o	\
psub.o	\
pvec.o	\
errorp.o

# unset this because no assembly (see -DASM_16BIT)
#LIBOBJS	= $(OBJS) $(ASMOBJS) $(RANDOBJ) 
LIBOBJS	= $(OBJS) $(RANDOBJ)

.c.a:
		$(CC) -c $(CFLAGS) $<
.s.a:
		$(AS) $(ASFLAGS) $<
.c.s:
		$(CC) $(CFLAGS) -S -Wc,-YE $<

lib:		$(LIB)

# for re-building a lib of a different type
libclean:	
		rm -f $(LIBOBJS)

bin:		calc pow factorial
test: 		mathtest randtest mathprof factorial pow powmod calc

testptob:	testptob.o btop.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o btop.o $(LIB)

mathtest:	mathtest.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIB)

randtest:	randtest.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIB)

calc:		calc.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o ${LIBF} $(LIB)

mathprof:	mathprof.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIB)

pow:		pow.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIB)

powmod:		powmod.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIB)

factorial:	factorial.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIB)

fastmultest:	fastmultest.o pfastmul.o $(LIB)
		$(CC) $(LDFLAGS) -o $@ $@.o pfastmul.o $(LIB)

testmachineop:	testmachineop.o machineop.o
		$(CC) $(CFLAGS) -o $@ $@.o machineop.o

mathtest.pixie, 	mathtest.Addrs:	mathtest
			pixie -o mathtest.pixie mathtest

mathtest.Counts:	mathtest.Addrs mathtest.pixie
			mathtest.pixie

mathtest.prof:		mathtest.Counts
			prof -pixie mathtest >mathtest.prof

mathprof.pixie, 	mathprof.Addrs:	mathprof
			pixie -o mathprof.pixie mathprof

mathprof.Counts:	mathprof.Addrs mathprof.pixie
			mathprof.pixie 8 4096

mathprof.prof:		mathprof.Counts
			prof -pixie mathprof >mathprof.prof

$(LIB):			$(LIBOBJS)
			ar rv $@ `lorder $? | tsort`
			ranlib $@

pmin.o:		pdefs.h
pmax.o:		pdefs.h
plogb.o:	$(HDR)
ppow.o:		$(HDR)
pipow.o:	$(HDR)
ppowmod.o:	$(HDR)
psqrt.o:	$(HDR)
pfact.o:	$(HDR)
pops.o:		pdefs.h $(HDR)
pabs.o:		pdefs.h $(HDR)
pneg.o:		pdefs.h $(HDR)
pcmp.o:		pdefs.h $(HDR)
pdivmod.o:	pdefs.h $(HDR)
pmul.o:		pdefs.h $(HDR) $(ASM16HDR)
padd.o:		pdefs.h $(HDR) $(ASM16HDR)
psub.o:		pdefs.h $(HDR) $(ASM16HDR)
podd.o:		pdefs.h $(HDR) 
prand.o:	pdefs.h $(HDR) 
pshift.o:	pdefs.h $(HDR) 
pdebug.o:	pdefs.h $(HDR)
pio.o:		pdefs.h pcvt.h $(HDR) 
ptoa.o:		pdefs.h pcvt.h $(HDR)
atop.o:		pdefs.h pcvt.h $(HDR)
ptoi.o:		pdefs.h pcvt.h $(HDR)
ptou.o:		pdefs.h pcvt.h $(HDR)
ptol.o:		pdefs.h pcvt.h $(HDR)
ptoul.o:	pdefs.h pcvt.h $(HDR)
itop.o:		pdefs.h pcvt.h $(HDR)
utop.o:		pdefs.h pcvt.h $(HDR)
ltop.o:		pdefs.h pcvt.h $(HDR)
ultop.o:	pdefs.h pcvt.h $(HDR)
ptob.o:		pdefs.h $(HDR)
btop.o:		pdefs.h $(HDR)
pfloat.o:	pdefs.h pcvt.h $(HDR)
phalf.o:	pdefs.h $(HDR) $(ASM16HDR)
pconst.o:	pdefs.h $(HDR)
errorp.o:	$(HDR)
#
# Assembly language
#
machineop.o:	machineop.s
memlsrw.o:	memlsrw.s

memaddw.o:	memaddw.s
memincw.o:	memincw.s
memsubw.o:	memsubw.s
memdecw.o:	memdecw.s

memmulw.o:	memmulw.s
memmulw1.o:	memmulw1.s
memdivw.o:	memdivw.s
memdivw1.o:	memdivw1.s

memmulw-p.o:	memmulw-p.s
memmulw1-p.o:	memmulw1-p.s
memdivw-p.o:	memdivw-p.s
memdivw1-p.o:	memdivw1-p.s

memmulw-G.o:	memmulw-p.o
		cp memmulw-p.o $@
memmulw1-G.o:	memmulw1-p.o
		cp memmulw1-p.o $@
memdivw-G.o:	memdivw-p.o
		cp memdivw-p.o $@
memdivw1-G.o:	memdivw1-p.o
		cp memdivw1-p.o $@

pfastmul.o:		pdefs.h $(HDR)
testmachineop.o:	machineop.h
fastmultest.o:		$(HDR)
mathtest.o:		pdefs.h $(HDR)
randtest.o:		$(HDR)
mathprof.o:		$(HDR)
factorial.o:		$(HDR)
pow.o:			$(HDR)

calc.o:			$(HDR)
			$(CC) $(CFLAGS) -c -I../factor calc.c

tar:		${SRC}
		tar -cvf - ${SRC} | compress >precision.tar.Z

shar:		${SRC}
		shar ${DOCSRC} ${BINSRC} ${HDR} ${RANDSRC} >precision0.shar
		shar ${LIBSRC} >precision1.shar

clean:	
		rm -f core
		rm -f testptob mathtest randtest mathprof factorial pow calc
		rm -f mathtest.o randtest.o mathprof.o testptob.o calc.o
		rm -f factorial.o
		rm -f testmachineop testmachineop.o machineop.o
		rm -f memaddw.o memincw.o memsubw.o memdecw.o
		rm -f memmulw.o memmulw1.o memdivw.o memdivw1.o
		rm -f memmulw-p.o memmulw1-p.o memdivw-p.o memdivw1-p.o
		rm -f mathprof.pixie mathprof.Addrs mathprof.Counts
		rm -f mathtest.pixie mathtest.Addrs mathtest.Counts
		rm -f pow.o powmod.o
		rm -f $(OBJS)
		rm -f $(ASMOBJS)
