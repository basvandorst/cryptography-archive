#CC  = cc
CC = cc
LD = cc
DEFS = -DUNIX -DHIGHFIRST -DBSD -DPORTABLE -DMPORTABLE -DNOTERMIO -DNOTOUCHOLAP
CFLAGS = $(OPTFLAGS) $(DEFS)
EXTRA_LIB =
LIB = -lcurses -ltermcap $(EXTRA_LIB)
PROGOBJ = main.o socket.o term.o user.o md5.o crypto.o idea.o random.o system.o passwd.o logtalk.o mpiio.o mpilib.o
# NSOBJ = ns.o getinfo.o skip.o send.o 
OBJ = $(PROGOBJ) $(OBJS_EXT)
ASM	= $(CC)	-c		# Assembler command
MAKE=/bin/make

default:
	@echo "type:"
	@echo "        make <system>"
	@echo ""
	@echo "where <system> can be:"
	@echo "        sun4, sun4cc(*), sun3gcc, sun3asm, sun3cc(*), sun386i, sunspc,"
	@echo "        sysv_386, sco-2.0, x286(*), hpux, linux, mips-ultrix, vax-ultrix,"
	@echo "        xenix386, mach_386, 386bsd, isc, isc_asm, 3b1, 3b1_asm, rs6000,"
	@echo "        bsd, bsdgcc, vax_bsd43, rt_aos4, osf, sgigcc_asm, sgigcc, irix"
	@echo "        newsgcc, newsasm, aux(*), os2, djgpp"
	@echo ""
	@echo "for targets marked with (*) you must first get unproto, see"
	@echo "setup.doc for further details"

_80386.o:	80386.S
	$(CPP) $(ASMDEF) 80386.S > _80386.s
	$(ASM) _80386.s
	rm -f _80386.s

next:
	$(MAKE) all \
	DEFS="-DNEXT -DUNIX -DHIGHFIRST -DMACH -DPORTABLE -DNOTOUCHOLAP -DNOTERMIO"
	
linux:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT="_80386.o" \
	DEFS="-DUNIX -DIDEA32 -DASM -DPOSIX"

386bsd:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT="_80386.o _zmatch.o" \
	DEFS="-O -DUNIX -DIDEA32 -DASM -DMAX_NAMELEN=255"

# Sun 3 with gcc
# change -traditional-cpp to -traditional for gcc < 2.0
sun3gcc:
	$(MAKE) all CC=gcc LD=gcc \
	DEFS="-O -traditional-cpp -DUNIX -DHIGHFIRST -DIDEA32 -DPORTABLE"

sun3asm:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT=mc68020.o \
	DEFS="-O -traditional-cpp -DUNIX -DHIGHFIRST -DIDEA32"

# Sun 3 with standard cc: compile with unproto
sun3cc: unproto/cpp
	$(MAKE) all CC=cc LD=cc \
	DEFS="-Qpath unproto -O -DUNIX -DHIGHFIRST -DIDEA32 -DPORTABLE"

# Sun 4 SPARC with gcc (tested with gcc 1.39 and newer, sparc.s can not be used
# with older versions of gcc)
# change -traditional-cpp to -traditional for gcc < 2.0
sun4gcc:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT=sparc.o \
	DEFS="-traditional-cpp -DUNIX -DHIGHFIRST -DIDEA32"

# Sun 4 SPARC with standard cc: compile with unproto
sun4cc: unproto/cpp
	$(MAKE) all CC=cc LD=cc OBJS_EXT=sparc.o \
	DEFS="-Qpath unproto -O -DUNIX -DHIGHFIRST -DIDEA32"

sun386i:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT=_80386.o \
	DEFS="-O -DUNIX -DIDEA32"

sunspc:
	$(MAKE) all CC="ccspc -B/1.8.6/sun4 -ansi -w -I/usr/include" \
	DEFS="-O -DUNIX -DIDEA32 -DHIGHFIRST -DUNIT32 -DMERRITT" \
	OBJS_EXT=sparc.o

# Sony newsos v3 for m68k, with gcc
# change -traditional-cpp to -traditional for gcc < 2.0
newsgcc:
	$(MAKE) all CC=gcc LD=gcc \
	DEFS="-O -I. -traditional-cpp -DNOTERMIO -D_BSD -DUNIX -DHIGHFIRST -DIDEA32 -DPORTABLE"

#  with asm
newsasm:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT=mc68020.s \
	DEFS="-O -I. -traditional-cpp -DNOTERMIO -D_BSD -DUNIX -DHIGHFIRST -DIDEA32"

sysv:
	$(MAKE) all CPP=/lib/cpp \
	DEFS="-O -DUNIX -DPORTABLE -DUSE_NBIO $(BYTEORDER)"

# optimized version with 80386.S
sysv_386:
	$(MAKE) all CPP="/lib/cpp -DSYSV" OBJS_EXT="_80386.o _zmatch.o" \
	DEFS="-O -DUNIX -DIDEA32 -DUSE_NBIO -DASM"

xenix386:
	$(MAKE) all CPP=/lib/cpp OBJS_EXT="_80386.o _zmatch.o" \
	DEFS="-O -DUNIX -DSVR2 -DIDEA32 -DUSE_NBIO -DASM"

# Interactive Unix SVR3/386 version 3.2 with gcc
isc:
	$(MAKE) all CC=gcc LD=gcc LDFLAGS="-lcposix" \
	DEFS="-O -DUNIX -DIDEA32 -DUSE_NBIO -DPORTABLE -DNO_ITIMER"

isc_asm:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT=_80386.o ASMDEF=-DSYSV \
	DEFS="-O -DUNIX -DIDEA32 -DUSE_NBIO -DNO_ITIMER" LDFLAGS="-lcposix"

mach_386:
	$(MAKE) all CC=gcc LD=gcc CPP=/lib/cpp OBJS_EXT=_80386.o \
	DEFS="-O -I. -DMACH -DUNIX -DIDEA32 -DNOTERMIO"

sco-3.0:
	$(MAKE) all CC=gcc LD=gcc CPP=/usr/lib/xcpp OBJS_EXT=_80386.o \
	ASMDEF=-DSYSV DEFS="-DPOSIX -Dsco -DUNIX -DIDEA32 -DUSE_NBIO -DNOTOUCHOLAP" \
	EXTRA_LIB="-lrpc -lsocket"

sco-2.0:
	$(MAKE) all CC=gcc LD=gcc CPP=/usr/lib/xcpp OBJS_EXT=_80386.o \
	ASMDEF=-DSYSV DEFS="-Dsco -O -DUNIX -DIDEA32 -DUSE_NBIO"

# Xenix 286
x286:
	$(MAKE) all CC="sh ccc.x286 -M2l" LD="cc -M2l" ASM="cc -M2l" \
	OBJS_EXT=8086.o  LDFLAGS="-F 3000" \
	DEFS="-LARGE -Ot -DUNIX -DNOPROTO -DSMALL_MEM -DDYN_ALLOC \
	-DUSE_NBIO -DSVR2"

# AIX/386 v.1.3
aix386:
	$(MAKE) all CPP="/lib/cpp -DSYSV" \
	DEFS="-O -DUNIX -DIDEA32 -DUSE_NBIO -DSYSV -DPORTABLE"

# AIX/370  (like general SysV)
aix370:
	$(MAKE) all CPP=/lib/cpp \
	DEFS="-O -DUNIX -DIDEA32 -DUSE_NBIO -DSYSV -DHIGHFIRST -DPORTABLE"

3b1:
	$(MAKE) all CC=gcc LD=gcc CPP=/usr/lib/cpp \
	DEFS="-O -I. -DUNIX -DSVR2 -DPORTABLE -DUSE_NBIO -DHIGHFIRST"

3b1_asm:
	$(MAKE) all CC=gcc LD=gcc CPP=/usr/lib/cpp OBJS_EXT=3B1_68010.o \
	DEFS="-O -I. -DUNIX -DSVR2 -DUSE_NBIO -DHIGHFIRST"

# Silicon Graphics Iris IRIX
sgigcc:
	$(MAKE) all CC=gcc LD=gcc CPP=/usr/lib/cpp \
	DEFS="-O -DUNIX -DPORTABLE -DUSE_NBIO -DHIGHFIRST"

# SGI with assembler modules
sgigcc_asm:
	$(MAKE) all CC=gcc LD=gcc CPP=/usr/lib/cpp OBJS_EXT="r3000.o r3kd.o" \
	DEFS="-O -DUNIX -DUSE_NBIO -DHIGHFIRST"

irix:
	$(MAKE) all CC=cc LD=cc EXTRA_LIB="-lsun" \
	DEFS="-DPOSIX -DUNIX -DPORTABLE -DUSE_NBIO -DHIGHFIRST -acpp -DNOTOUCHOLAP"

hpux:
	$(MAKE) all CPP=/usr/lib/cpp \
	DEFS="+DA1.0 -Wl,-a,archive -Aa +O3 +Obb5000 -DHPUX -DNOTOUCHOLAP -D_INCLUDE_POSIX_SOURCE \
	-D_INCLUDE_HPUX_SOURCE -D_INCLUDE_XOPEN_SOURCE -DHIGHFIRST -DUNIX \
	-DPORTABLE -DUSE_SELECT -DIDEA32 -DMERRITT"

# VAX Ultrix 4.2 BSD rev. 96, system #3 with gcc
vax-ultrix:
	$(MAKE) all CC=gcc LD=gcc \
	DEFS="-O -DUNIX -D__ultrix -DPORTABLE -DUSE_SELECT \
	-DSIG_IGN=1 -DSIG_DFL=0"

# DEC Ultrix 4.2 BSD with gcc with MIPSco processors
#
# To use dec's cc on mips you should
# uncomment the next two lines because you can't compile r3000.c 
#r3000.o: r3000.s
#	$(AS) -o $@ r3000.s
#
mips-ultrix:
	$(MAKE) all CC=gcc LD=gcc OBJS_EXT="r3kd.o r3000.o"\
	DEFS="$(DBG) -DUNIX -D__ultrix -DUSE_SELECT -DIDEA32 -DPOSIX -DNOTOUCHOLAP"

# RS6000 running AIX
rs6000:
	$(MAKE) all DEFS="-O -DUNIX -DNOTERMIO -DPORTABLE \
	-DMODERN -DHIGHFIRST -DIDEA32"


bsdgcc:
	$(MAKE) all CC=gcc LD=gcc \
	DEFS="-O -I. -DNOTERMIO -D_BSD -DUNIX -DPORTABLE -DUSE_NBIO $(BYTEORDER)"

bsd:
	$(MAKE) all "LD=cc -s" \
	DEFS="-O -I. -B/lib/ -DNOTERMIO -D_BSD -DUNIX -DPORTABLE -DUSE_NBIO $(BYTEORDER)"

sequent: unproto/cpp
	$(MAKE) all "LD=cc -s -j" CPP=/usr/lib/cpp \
	DEFS="-j -O -I. -Yp,unproto -DUNIX -DNOTERMIO -DPORTABLE -DUSE_NBIO $(BYTEORDER)"

# RT running bsd with gcc
rt_aos4:
	$(MAKE) all CC=gcc LD=gcc \
	DEFS="-O -DUNIX -D_BSD -DMAX_NAMELEN=255 -DPORTABLE -DHIGHFIRST -DNOTERMIO -I."

# Vax running bsd with gcc
vax_bsd43:
	$(MAKE) all CC=gcc LD=gcc \
	DEFS="-O -I. -DUNIX -D_BSD -DMAX_NAMELEN=255 -DNOTERMIO -DPORTABLE"

# The Open Software Foundation's OSF/1
osf:
	$(MAKE) all \
	DEFS="-O -I. -D_BSD -DUNIX -DNOTERMIO -DPORTABLE $(BYTEORDER)"

# A/UX 3.0 using unproto (mine is invoked as a cpp using the CFLAGS shown
# I placed the cpp in a subdirectory of /src/pgp called unproto
aux: unproto/cpp
	$(MAKE) all CC=cc \
	LDFLAGS=-lposix \
	DEFS="-B unproto/ -tp -DSVR2 \
	-DHIGHFIRST -DUNIX -DPORTABLE -DUSE_SELECT -DUPTON"

# optimized version with 80386.S for emx 0.8e, OS/2 2.0 or DOS
os2:
	$(MAKE) all PROJ=pgp.exe OBJS_EXT="_80386.o _zmatch.o" \
	CC="gcc" DEFS="-O -DOS2 -DASM -DIDEA32" \
	ASM="gcc -c" LD="gcc" LDFLAGS="pgp.def"

# DJGPP, DJ Delorie's port of GNU C to MS-DOA (thanks DJ!).  Tested with dmake.
# You may have to rehack this -- I'm not good with makefiles.
djgpp:
	$(MAKE) "OBJS_EXT=80386.o zmatch.o" CC=gcc LD=gcc \
	"DEFS=-O2 -DSMALL_MEM -DASM -DIDEA32" all
	
all:	ytalk

ytalk:	${OBJ}
	${LD} ${CFLAGS} -o ytalk ${OBJ} ${LIB}

clean:
	-rm -f *.o
	-rm -f \#*
	-rm -f ytalk

dist:	clean
	tar -cf - -C .. ytalk | gzip | uuencode ytalk.tar.z > ../ytalk.z.uu

${OBJ}:	ytalk.h talkd.h config.h
