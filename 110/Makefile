CC=gcc
# Architectures and defines :
# MIPS - MIPS
# ALPHA - ALPHA
# INTEL - INTEL
# Ultrasparc - SUN4U
# Sparc - SUN4M
# Motorola 68k - m68K
# GENERIC - GENERIC

#Alpha OSF/1 3.2
#CC=cc
#CFLAGS=-O3 -tune host -inline speed -migrate -DALPHA
#CFLAGS=-fast -DALPHA -DTEST_NET -DTEST -DDEBUG
#LDFLAGS=-O5 -migrate

# Intel Linux
ASMFILES=desasm.o ksasm.o
#CFLAGS=-O6 -mcpu=pentium -DSUNM
CFLAGS=-mcpu=pentium -frisc -DINTEL -DLINUX -DUSE_ASM
LDFLAGS=-static

# Intel FreeBSD
#CFLAGS=-static -O6 -m486 -DINTEL -DFREEBSD -DUSE_ASM

# PPC mkLinux
#CFLAGS=-O6 -DGENERIC -DMKLINUX
#LDFLAGS=-static

# HPUX
#CFLAGS=-O6 -DPARISC -DHPUX
#LDFLAGS=-static

# IBM POWER AIX
#CC=cc
#CFLAGS=-O3 -DAIX -DGENERIC

# Sun Solaris, ultrasparc
#CFLAGS=-O6 -msupersparc -DSUN4U
#LDFLAGS=-lsocket -lnsl
#CC=/afs/ethz.ch/bin/sun4u_55/opt/SUNWspro/bin/cc
#CFLAGS=-fast -Xa -xO5 -DSUN4U # -DTEST -DTEST_NET
#LDFLAGS=-lsocket -lnsl -fast

# Sun SunOS 4.1.4
#CFLAGS=-O6 -DSUN4C

# SGI IRIX, R4400
#CFLAGS=-O6 -DIRIX
# Compiling with cc (in a tcsh):
# foreach i (*.c)
#   cc -O3 -j -DIRIX $i
# end
# cc -o des3 *.u

# NeXT NEXTSTEP, MC68k
# CFLAGS=-O6 -DNEXTSTEP -DNeXT_m68k

des3:	descrack.o des_enc.o set_key.o cbc_enc.o main.o net.o slice.o deseval.o $(ASMFILES)
	$(CC) $(LDFLAGS) -o des3 descrack.o des_enc.o set_key.o cbc_enc.o main.o net.o slice.o deseval.o $(ASMFILES)
# This does not work on IRIX (?)
#	strip des3


descrack.o: descrack.c Makefile

des_enc.o: des_enc.c Makefile

set_key.o: set_key.c Makefile

cbc_enc.o: cbc_enc.c Makefile

main.o: main.c Makefile

net.o: net.c Makefile

cbc_enc.c: des_locl.h Makefile

des_enc.c: des_locl.h spr.h arch.h Makefile

descrack.c: arch.h des.h desc.h Makefile

main.c: desc.h net.h Makefile

net.c: net.h des.h arch.h Makefile

set_key.c: des_locl.h podd.h sk.h Makefile

deseval.c: Makefile

slice.c: Makefile

des.h: arch.h

des_locl.h: des.h

desc.h: des.h

net.h: arch.h


nice:
	rm -f *~
clean:
	rm -f *~; rm -f *.o *.u
	rm -f des3
