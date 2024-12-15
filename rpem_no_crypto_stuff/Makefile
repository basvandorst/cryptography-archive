# This is the makefile for rpem, from rpem.tar.Z on 35.8.1.6.
#
#  The following symbols are used by rpem:
#
#UNIX        Define if targetting Unix, any (?) flavor.
#USEBSD      Define if targetting BSD.
#SYSV        Define if targetting System V Unix.  Not well-tested yet.
#OS2         Define if targetting OS/2.
#HP          Define if targetting HP-UX.
#EXPORT      Define if not using DES.  I'm not sending DES code
#            outside the USA in order to cover myself legally.
#
#The following symbols are also used, but you shouldn't have to define
#them (the various compilers do that):
#
#MSDOS       Defined if targetting MS-DOS or OS/2.  Note:  Microsoft C
#            defines this automatically for both MS-DOS and OS/2.
#            Define it manually if you're using Turbo C.
#__MACH__    Defined if targetting Mach.  Note:  NeXT's cc defines
#            this automatically.
#M_I86       Defined if targetting an Intel 80x8x processor.  Note:
#            MS C defines this automatically.
#__TURBOC__  Defined if using Borland's Turbo C (automatically).
#LITTLE_ENDIAN  Define if target stores least significant byte first when
#            storing longs.  Defined automatically if M_I86 or __TURBOC__
#            is defined, but you may have to define manually in some
#            circumstances.
#            Most non-Intel architectures (like Motorola 680x0 and
#            and SPARC) are not little-endian.  VAX is little-endian;
#            MIPS generally is not except as used in DECstations.
#sun         Define if targetting SunOS; automatically defined by
#            Sun's brain-damaged cc.
#ultrix      Define if targetting VAX or MIPS ULTRIX; automatically
#            defined by DEC's compilers.
#sgi         Define if targetting Silicon Graphics running IRIX.
#            Is this a System V variant?  Need more info.
#__convexc__ Defined by Convex's C compiler.
#<others>    See zbigint.h for symbols that should be defined that
#            relate to the large integer package.  The defaults provided
#            should work for most systems.
#
#CFLAGS =  -DUNIX
#CFLAGS = -g -pg
#CC = gcc
#CFLAGS = -O -DUNIX -DUSEBSD -DHP
CFLAGS =  -DUNIX -DUSEBSD -O
DES = des.o
ROUTINES = mcrt.o mberleka.o zmisc.o megcd.o zrandom.o msqrt.o cippkg.o newdes.o zbigint.o getsys.o prencode.o usage.o blockcip.o $(DES)

all: rpem makerkey testdes testnewdes prencode

clean:
	rm *.o mrrrabts rpem makerkey prencode stripmsg bigint.msg

rpem: rpemmain.o des.o $(ROUTINES)
	$(CC) -o rpem $(CFLAGS) rpemmain.o $(ROUTINES)

# This does a basic test of rpem.  It should output a reassuring message.
testdes: rpem
	echo "Type  ninetynine  when asked for a key. "
	rpem -d -s pr99 -g -i msg99 -u albert@victoria.edu

testnewdes: rpem
	rpem -d -s pr524 -i msgnewdes -u mrr

makerkey: makerkey.o  $(ROUTINES)
	$(CC) -o makerkey $(CFLAGS) makerkey.o  $(ROUTINES)

# There are two benchmarks:  one for a reasonably-sized key (on
# fast processors this may result in a benchmark time that's too small),
# and a second with an unreasonably big public key.
# I'd appreciate it if you'd report both times to me, along with 
# the machine & OS you are using.   riordanmr@clvax1.cl.msu.edu
benchmark: rpem
	time rpem -d -i msg524 -s pr524 -u mrr >bigint.msg
	time rpem -d -i msg997 -s pr997 -u jack

# This is for people who don't trust me that rpem is really using DES!
# You must provide your own, known working version of "des" (like Sun's).
verifydes: rpem stripmsg prencode
	rpem -e -r mrr -n -v 0000000000000000 -k Patty -p pu524 -i desmsg >desmsg.msg
	stripmsg <desmsg.msg | prencode -d | des -d -k Patty
	rm desmsg.msg

prencode: tprencod.o prencode.o usage.o getopt.o
	$(CC) -o prencode $(CFLAGS) tprencod.o prencode.o usage.o getopt.o

stripmsg: stripmsg.o
	$(CC) -o stripmsg $(CFLAGS) stripmsg.o

mrrrabts: mrrrabts.o zbigint.o $(ROUTINES)
	$(CC) -o mrrrabts $(CFLAGS) mrrrabts.o  $(ROUTINES)

	
# Dependencies for File: blockcip.o :
blockcip.o :  blockcip.c blockcip.h blockcpr.h des.h           
blockcip.o : desproto.h newdes.h newdespr.h             
# Dependencies for File: cippkg.o :
cippkg.o :   blockcip.h blockcpr.h            
cippkg.o : cippkg.c cippkgpr.h msqrtpro.h newdes.h newdespr.h prcodepr.h          
cippkg.o : prencode.h typtime.h zbigint.h zmiscpro.h zproto.h           
# Dependencies for File: des.o :
des.o :    des.c            
des.o : des.h desproto.h p.h             
# Dependencies for File: desmain.o :
desmain.o : des.h desmain.c desproto.h getoptpr.h usagepro.h           
# Dependencies for File: getopt.o :
getopt.o :   getopt.c             
# Dependencies for File: getsys.o :
getsys.o :   getsys.c getsyspr.h            
# Dependencies for File: makerkey.o :
makerkey.o :   blockcip.h             
makerkey.o : blockcpr.h cippkgpr.h getsyspr.h makerkey.c newdes.h newdespr.h          
makerkey.o : p.h prcodepr.h usagepro.h zbigint.h zproto.h           
# Dependencies for File: mberleka.o :
mberleka.o : mberleka.c p.h zbigint.h zmiscpro.h zproto.h           
# Dependencies for File: mcrt.o :
mcrt.o : mcrt.c megcdpro.h p.h zbigint.h zmiscpro.h zproto.h          
# Dependencies for File: megcd.o :
megcd.o :  megcd.c megcdpro.h p.h zbigint.h zmiscpro.h          
megcd.o : zproto.h               
# Dependencies for File: mrrrabts.o :
mrrrabts.o :  cippkgpr.h mrrrabts.c p.h typtime.h zbigint.h          
mrrrabts.o : zmiscpro.h zproto.h              
# Dependencies for File: msqrt.o :
msqrt.o :  msqrt.c p.h zbigint.h zmiscpro.h zproto.h          
# Dependencies for File: nd2main.o :
nd2main.o :  nd2main.c newdespr.h uucodepr.h            
# Dependencies for File: newdes.o :
newdes.o : newdes.c newdes.h newdespr.h             
# Dependencies for File: prencode.o :
prencode.o : prcodepr.h prencode.c prencode.h             
# Dependencies for File: rpemmain.o :
rpemmain.o : blockcip.h blockcpr.h cippkgpr.h getoptpr.h getsyspr.h newdes.h          
rpemmain.o : p.h prcodepr.h rpemmain.c rpemprot.h typtime.h usagepro.h          
rpemmain.o : zbigint.h zproto.h              
# Dependencies for File: tprencod.o :
tprencod.o :  prcodepr.h tprencod.c             
# Dependencies for File: usage.o :
usage.o : usage.c               
# Dependencies for File: uucode.o :
uucode.o : uucode.c uucodepr.h              
# Dependencies for File: zbigint.o :
zbigint.o :  zbigint.c zbigint.h zbigintp.h            
# Dependencies for File: zmisc.o :
zmisc.o : zbigint.h zmisc.c zmiscpro.h zproto.h            
# Dependencies for File: zrandom.o :
zrandom.o : zbigint.h zproto.h zrandom.c             
