#****************************************************************************
#*																			*
#*						Makefile for the encryption library					*
#*																			*
#****************************************************************************

PROJ	= cryptlib

CFLAGS	= -c -D__UNIX__ -O -I. $(CMDC)	# Flags for compiler

LFLAGS	= -o $(PROJ) $(CMDL)	# Flags for linker

OUTPATH	=					# Where object files go (/tmp is a good place)
OBJ		= .o				# Extension for object files
EXE		=					# Extension for executables

LD		= $(CC)				# Linker (just use the C compiler)
ECHO	= echo				# Echo to screen command
MAKE	= make				# The make command

#****************************************************************************
#*																			*
#*			If no args are given, print a usage message and exit			*
#*																			*
#****************************************************************************

default:
		@$(ECHO)
		@$(ECHO) "Usage: make <system-type>"
		@$(ECHO)
		@$(ECHO) "To create the encryption library, you have to enter the Unix system type"
		@$(ECHO) "you want to build it for.  Possible options are: aix, aix370, aix386,"
		@$(ECHO) "bsd386, convex, irix, hpux, hpux-gcc, isc, linux, mint, mipsbsd, next, osf1,"
		@$(ECHO) "qnx, sun, svr4, ultrix, and uts4.  If none of the above fit, try 'make "
		@$(ECHO) "generic', and send a copy of any changes necessary to the author, "
		@$(ECHO) "pgut01@cs.auckland.ac.nz"
		@$(ECHO)

love:
		@$(ECHO) "Nicht wahr?"
		@$(ECHO)

#****************************************************************************
#*																			*
#*					Rules to build the encryption library					*
#*																			*
#****************************************************************************

$(OUTPATH)crypt$(OBJ):		crypt.h crypt.c
							$(CC) $(CFLAGS) crypt.c

$(OUTPATH)null$(OBJ):		crypt.h null.c
							$(CC) $(CFLAGS) null.c

$(OUTPATH)mdcshs$(OBJ):		crypt.h shs.h mdcshs.c
							$(CC) $(CFLAGS) mdcshs.c

$(OUTPATH)shs$(OBJ):		crypt.h shs.h mdcshs.c
							$(CC) $(CFLAGS) shs.c

$(OUTPATH)test$(OBJ):		crypt.h test.c
							$(CC) $(CFLAGS) test.c

$(PROJ)$(EXE):	$(OUTPATH)crypt$(OBJ) $(OUTPATH)mdcshs$(OBJ) \
				$(OUTPATH)null$(OBJ) $(OUTPATH)shs$(OBJ) \
				$(OUTPATH)test$(OBJ)
		@$(LD) $(LFLAGS) $(OUTPATH)crypt$(OBJ) $(OUTPATH)null$(OBJ) \
		$(OUTPATH)mdcshs$(OBJ) $(OUTPATH)shs$(OBJ) $(OUTPATH)test$(OBJ)

#****************************************************************************
#*																			*
#*						Defines for each variation of Unix					*
#*																			*
#****************************************************************************

# AIX for the RS6000: Use cc.  Differs from AIX370/386 in endianness

aix:
		@$(MAKE) $(PROJ) CMDC="-DAIX" CC="cc"

# AIX370, AIX386: Use cc

aix370:
		@$(MAKE) $(PROJ) CMDC="-DAIX370" CC="cc"
aix386:
		@$(MAKE) $(PROJ) CMDC="-DAIX386" CC="cc"

# AUX: The only Unix so bloated it has /etc.etc.etc

# BSD 386: Use cc

bsd386:
		@$(MAKE) $(PROJ) CMDC="-DBSD386" CC="cc"

# Convex: Use cc (cc has some bugs)

convex:
		@$(MAKE) $(PROJ) CMDC="-DCONVEX" CC="cc"

# Esix: Run away!!  Run away!!

# Generic: Generic BSD-ish system running gcc

generic:
		@$(MAKE) $(PROJ) CMDC="-DGENERIC" CC="gcc"

# HPUX: Use cc if you have the ANSI C compiler, otherwise use gcc
#       Need to use '-Aa -D_HPUX_SOURCE' as compiler options to get
#       C compiler into ANSI C mode with UNIX symbols.

hpux:
		@$(MAKE) $(PROJ) CMDC="-DHPUX -Aa -D_HPUX_SOURCE +O3" CC="cc"

hpux-gcc:
		@$(MAKE) $(PROJ) CMDC="-DHPUX" CC="gcc"

# Irix: Use cc with the -acpp flag for maximum ANSI-ness

irix:
		@$(MAKE) $(PROJ) CMDC="-DIRIX -acpp" CC="cc"

# ISC Unix: Use gcc

isc:
		@$(MAKE) $(PROJ) CMDC="-DISC" CC="gcc"

# Linux: Use gcc

linux:
		@$(MAKE) $(PROJ) CMDC="-DLINUX" CC="gcc"

# MiNT: Use gcc

mint:
		@$(MAKE) $(PROJ) CMDC="-DMINT" CC="gcc"

# Risc/OS 4: Use gcc

mipsbsd:
		@$(MAKE) $(PROJ) CMDC="-DMIPSBSD" CC="gcc"

# NeXT 3.0: Use cc

next:
		@$(MAKE) $(PROJ) CMDC="-O2 -DNEXT" CMDL="-object -s" CC="cc"

# OSF 1: Use gcc

osf1:
		@$(MAKE) $(PROJ) CMDC="-DOSF1" CC="gcc"

# QNX 4.x: Use cc -ml, ld -N32768 -ml

qnx:
		@$(MAKE) $(PROJ) CMDC="-ml -DPOSIX" CMDL="-N32768 -ml" CC="cc"

# Sun/Slowaris: Use gcc

sun:
		@$(MAKE) $(PROJ) CMDC="-DSUNOS" CC="gcc"

# SVR4: Use cc.  Better results can be obtained by upgrading your OS to
#       4.4 BSD.

svr4:
		@$(MAKE) $(PROJ) CMDC="-DSVR4" CC="cc"

# Ultrix: Use vcc or gcc

ultrix:
		@$(MAKE) $(PROJ) CMDC="-DULTRIX" CC="gcc"

# Amdahl UTS 4: Use cc

uts4:
		@$(MAKE) $(PROJ) CMDC="-DUTS4 -Xc", CC="cc"

#****************************************************************************
#*																			*
#*						Cleanup after make has finished						*
#*																			*
#****************************************************************************

clean:
		rm -f *.o core
