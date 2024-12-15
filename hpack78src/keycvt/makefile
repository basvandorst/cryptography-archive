#****************************************************************************
#*																			*
#*		  Makefile for the Unix version of the the KEYCVT utility			*
#*																			*
#****************************************************************************

# The OS and OS mutation we are compiling under.  Currently handled (but
# not necessarily by this makefile) types are __AMIGA__, __ATARI__, __MAC__,
# __MSDOS__, __OS2__(TOPSPEED, 32BIT), __UNIX__ (AIX, AIX370, AIX386, GENERIC,
# IRIX, ISC, LINUX, MINT, POSIX, SUNOS, SVR4, ULTRIX, ULTRIX_OLD).
# Partially handled types are __IIGS__, __VMS__.

OS			= -D__UNIX__

# The various system-dependant defines from above:

DEFINES		= $(OS)

# AIX for the RS6000: Use cc.  Differs from AIX370/386 in endianness
#		AIX - Peter Gutmann, pgut1@cs.aukuni.ac.nz
# AIX386: Use cc
# AIX370: Use cc
#		AIX386, AIX370 - Peter, stud11@cc4.kuleuven.ac.be
# Irix: Use cc with the -acpp flag for maximum ANSI-ness
#		Irix 3.3.2 - Stuart Woolford, stuartw@ccu1.aukuni.ac.nz
# ISC Unix: Use gcc
#		ISC Unix SVR3.2 v2.2.1 - David Nugent, david@csource.oz.au
# Linux: Use gcc
#		Linux 0.98 - Jacques Grove, 9230971@sunvax.sun.ac.za
# MiNT: Use gcc
#		MiNT 0.98 - Stephan Neuhaus, neuhaus@informatik.uni-kl.de
# QNX 4.x: Use cc -ml, no -lcurses, ld -N32768 -ml
#		QNX 4.0 - Stuart Woolford, stuartw@ccu1.aukuni.ac.nz
# SunOs: Use gcc.  Don't use the Sun acc which breaks the decompressor
#		SunOs 4.1 - Peter Gutmann, pgut1@cs.aukuni.ac.nz
# SVR4: Use cc
#		SVR4 - Jean-loup Gailly, jloup@chorus.fr
# Ultrix: Use vcc or gcc. Use rm on cc
#		Ultrix 4.1 - Peter Gutmann, pgut1@cs.aukuni.ac.nz
#		Ultrix 3.1 - John Whittington, whitting@scl.cwru.edu
# Esix:  Run away!  Run away!

PROJ	= keycvt

CFLAGS	= -c $(DEFINES) -O -I. $(CMDC)	# Flags for compiler

LFLAGS	= -o $(PROJ) $(CMDL)			# Flags for linker

OUTPATH	=					# Where object files go (/tmp is a good place)
OBJ		= .o				# Extension for object files
EXE		=					# Extension for executables

LD		= $(CC)				# Linker (just use the C compiler)
LS		= ls -l				# Directory command
ECHO	= echo				# Echo to screen command
MAKE	= make				# The make command

#****************************************************************************
#*																			*
#*			If no args are given, print a usage message and exit			*
#*																			*
#****************************************************************************

default:
		@$(ECHO)
		@$(ECHO) "You have to enter the Unix variant you want to build KEYCVT for."
		@$(ECHO) "Possible options are: aix (RS6000), aix370, aix386, irix, isc, qnx, sun,"
		@$(ECHO) "svr4, and ultrix.  If none of the above fit, try 'make generic', and"
		@$(ECHO) "send a copy of any changes necessary to the author, pgut1@cs.aukuni.ac.nz"
		@$(ECHO) "or peterg@kcbbs.gen.nz."
		@$(ECHO)

love:
		@$(ECHO) "Nicht wahr?"
		@$(ECHO)

#****************************************************************************
#*																			*
#*								Rules to build KEYCVT						*
#*																			*
#****************************************************************************

$(OUTPATH)md5$(OBJ):		md5.h md5.c
							$(CC) $(CFLAGS) md5.c

$(OUTPATH)mdc$(OBJ):		md5.h mdc.h mdc.c
							$(CC) $(CFLAGS) mdc.c

$(OUTPATH)idea$(OBJ):		md5.h idea.c
							$(CC) $(CFLAGS) idea.c

$(OUTPATH)keycvt$(OBJ):		md5.h mdc.h keycvt.c
							$(CC) $(CFLAGS) keycvt.c

$(PROJ)$(EXE):	$(OUTPATH)md5$(OBJ) $(OUTPATH)mdc$(OBJ) \
				$(OUTPATH)idea$(OBJ) $(OUTPATH)keycvt$(OBJ)
		@$(LD) $(LFLAGS) $(OUTPATH)md5$(OBJ) $(OUTPATH)mdc$(OBJ) \
		$(OUTPATH)idea$(OBJ) $(OUTPATH)keycvt$(OBJ)

#****************************************************************************
#*																			*
#*						Defines for each variation of Unix					*
#*																			*
#****************************************************************************

# AIX for the RS6000: Use cc.  Differs from AIX370/386 in endianness

aix:
		@$(MAKE) keycvt CMDC="-DBIG_ENDIAN" CC="cc"

# AIX370, AIX386: Use cc

aix370:
		@$(MAKE) keycvt CMDC="-DLITTLE_ENDIAN" CC="cc"
aix386:
		@$(MAKE) keycvt CMDC="-DLITTLE_ENDIAN" CC="cc"

# AUX: The only Unix so bloated it has /etc.etc.etc

# Generic: Generic BSD-ish system running gcc.  Can't continue, we really
# need user intervention here.

generic:
		@$(ECHO)
		@$(ECHO) "Please set up a specific make rule with defines for your processor"
		@$(ECHO) "endianness and rerun make".
		@$(ECHO)

# Irix: Use cc with the -acpp flag for maximum ANSI-ness

irix:
		@$(MAKE) keycvt CMDC="-acpp -DBIG_ENDIAN" CC="cc"

# ISC Unix: Use gcc

isc:
		@$(MAKE) keycvt CMDC="-DLITTLE_ENDIAN" CC="gcc"

# Linux: Use gcc

linux:
		@$(MAKE) keycvt CMDC="-DBIG_ENDIAN" CC="gcc"

# MiNT: Use gcc

mint:
		@$(MAKE) keycvt CMDC="-DBIG_ENDIAN" CC="gcc"

# QNX 4.x: Use cc -ms

qnx:
		@$(MAKE) keycvt CMDC="-ms -DLITTLE_ENDIAN" CMDL="-ms" CC="cc"

# SunOs: Use gcc.

sun:
		@$(MAKE) keycvt CMDC="-DBIG_ENDIAN" CC="gcc"

# SVR4: Use cc

svr4:
		@$(MAKE) keycvt CMDC="-DBIG_ENDIAN" CC="cc"

# Ultrix: Use vcc or gcc

ultrix:
		@$(MAKE) keycvt CMDC="-DLITTLE_ENDIAN" CC="gcc"
