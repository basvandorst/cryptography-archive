#
# Stealth V2.01 beta
#

OFILES = stealth.o md5.o mpiio.o mpilib.o

LDFLAGS= 

#
# Define UNIX for a unix machine, USE_TMP to create temporary files in
# /tmp, and USE_PGPPATH to create them in the directory pointed to by
# the PGPPATH environment variable. Otherwise the will be created in the
# current directory.
#
# Define HIGHFIRST for most RISC machines, don't for 80x86
#
# Choose word size UNITx and MUNITx:
#
#  8 bit:  -DUNIT8 -DMUNIT8
# 16 bit:  -DUNIT16 -DMUNIT16
# 32 bit:  -DUNIT32 -DMUNIT32
#

CPPFLAGS=-DPORTABLE -DUNIT32 -DUNIX \
	-DUSE_PGPPATH -DHIGHFIRST -DDEBUG

CFLAGS=-g

#
# On a machine using gcc, uncomment the CC=gcc line (i.e. remove the #),
# and put a # before the CC=cc line, or take it out altogether.
#

#CC=gcc
CC=cc

stealth:	$(OFILES)
	$(CC) $(CFLAGS) -o stealth $(OFILES) $(LDFLAGS)

.c.o:	
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)
