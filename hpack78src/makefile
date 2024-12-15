#****************************************************************************
#*																			*
#*			Makefile for the Unix version of the HPACK archiver				*
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

PROJ	= hpack

CFLAGS	= -c $(DEFINES) -O -I. $(CMDC)	# Flags for compiler

LFLAGS	= -o $(PROJ) -lcurses $(CMDL)	# Flags for linker

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
		@$(ECHO) "You have to enter the Unix variant you want to build HPACK for."
		@$(ECHO) "Possible options are: aix (RS6000), aix370, aix386, irix, isc, qnx, sun,"
		@$(ECHO) "svr4, ultrix, and ultrix_old (Ultrix 3.x or earlier).  If none of the above"
		@$(ECHO) "fit, try 'make generic', and send a copy of any changes necessary to the"
		@$(ECHO) "author, pgut1@cs.aukuni.ac.nz or peterg@kcbbs.gen.nz."
		@$(ECHO)

love:
		@$(ECHO) "Nicht wahr?"
		@$(ECHO)

#****************************************************************************
#*																			*
#*								Rules to build HPACK						*
#*																			*
#****************************************************************************

$(OUTPATH)arcdir$(OBJ):		arcdir.h defs.h error.h flags.h hpacklib.h \
							system.h tags.h io/fastio.h arcdir.c
							$(CC) $(CFLAGS) arcdir.c

$(OUTPATH)arcdirio$(OBJ):	arcdir.h filehdr.h choice.h defs.h error.h \
							hpacklib.h flags.h system.h crc/crc16.h \
							crypt/crypt.h io/fastio.h io/hpackio.h \
							store/store.h arcdirio.c
							$(CC) $(CFLAGS) -DARCHIVE_TYPE=3 arcdirio.c

$(OUTPATH)archive$(OBJ):	arcdir.h filehdr.h choice.h defs.h error.h \
							filesys.h flags.h frontend.h hpacklib.h system.h \
							tags.h crypt/crypt.h io/fastio.h io/hpackio.h \
							store/store.h archive.c
							$(CC) $(CFLAGS) archive.c

$(OUTPATH)cli$(OBJ):		arcdir.h choice.h defs.h error.h filesys.h \
							flags.h frontend.h hpacklib.h language/hpaktext.h \
							system.h wildcard.h crypt/crypt.h io/fastio.h \
							cli.c
							$(CC) $(CFLAGS) cli.c


$(OUTPATH)error$(OBJ):		arcdir.h defs.h filehdr.h error.h errorlvl.h \
							flags.h frontend.h hpacklib.h system.h \
							crypt/crypt.h io/fastio.h io/hpackio.h \
							store/store.h error.c
							$(CC) $(CFLAGS) error.c

$(OUTPATH)filesys$(OBJ):	arcdir.h defs.h filehdr.h error.h flags.h \
							frontend.h hpacklib.h system.h tags.h io/fastio.h \
							io/hpackio.h filesys.c
							$(CC) $(CFLAGS) filesys.c

$(OUTPATH)frontend$(OBJ):	arcdir.h filehdr.h choice.h defs.h error.h \
							filesys.h flags.h frontend.h hpacklib.h system.h \
							tags.h wildcard.h crypt/crypt.h io/fastio.h \
							io/hpackio.h store/store.h frontend.c
							$(CC) $(CFLAGS) frontend.c

$(OUTPATH)script$(OBJ):		arcdir.h choice.h defs.h error.h filesys.h \
							flags.h frontend.h hpacklib.h \
							language/hpaktext.h system.h wildcard.h \
							io/fastio.h io/hpackio.h
							$(CC) $(CFLAGS) script.c

$(OUTPATH)tags$(OBJ):		defs.h error.h hpacklib.h system.h tags.h \
							io/fastio.h tags.c
							$(CC) $(CFLAGS) tags.c

$(OUTPATH)unix$(OBJ):		defs.h error.h flags.h frontend.h hpacklib.h \
							system.h tags.h io/hpackio.h unix.c
							$(CC) $(CFLAGS) unix.c

$(OUTPATH)viewfile$(OBJ):	arcdir.h choice.h defs.h error.h filehdr.h \
							flags.h frontend.h hpacklib.h \
							language/hpaktext.h system.h tags.h wildcard.h \
							io/fastio.h io/hpackio.h viewfile.c
							$(CC) $(CFLAGS) viewfile.c

$(OUTPATH)wildcard$(OBJ):	defs.h error.h system.h wildcard.h wildcard.c
							$(CC) $(CFLAGS) wildcard.c

$(OUTPATH)crc16$(OBJ):		defs.h crc/crc16.c
							$(CC) $(CFLAGS) crc/crc16.c

$(OUTPATH)crypt$(OBJ):		arcdir.h choice.h defs.h error.h hpacklib.h \
							filesys.h flags.h frontend.h system.h \
							crypt/crypt.h crypt/md5.h crypt/mdc.h \
							crypt/packet.h crypt/rsa.h io/fastio.h \
							io/hpackio.h crypt/crypt.c
							$(CC) $(CFLAGS) crypt/crypt.c

$(OUTPATH)md5$(OBJ):		defs.h crypt/md5.h crypt/md5.c
							$(CC) $(CFLAGS) crypt/md5.c

$(OUTPATH)mdc$(OBJ):		defs.h error.h hpacklib.h crypt/crypt.h \
							crypt/mdc.h crypt/mdc.c
							$(CC) $(CFLAGS) crypt/mdc.c

$(OUTPATH)rsa$(OBJ):		defs.h crypt/rsa.h crypt/rsa.c
							$(CC) $(CFLAGS) crypt/rsa.c

$(OUTPATH)display$(OBJ):	defs.h frontend.h hpacklib.h io/display.c
							$(CC) $(CFLAGS) io/display.c

$(OUTPATH)fastio$(OBJ):		arcdir.h defs.h error.h flags.h frontend.h \
							hpacklib.h system.h crc/crc16.h crypt/crypt.h \
							data/ebcdic.h io/fastio.h io/hpackio.h \
							io/fastio.c
							$(CC) $(CFLAGS) io/fastio.c

$(OUTPATH)hpaktext$(OBJ):	defs.h error.h errorlvl.h language/hpaktext.c
							$(CC) $(CFLAGS) language/hpaktext.c

$(OUTPATH)lza$(OBJ):		defs.h choice.h error.h flags.h hpacklib.h system.h \
							crc/crc16.h crypt/crypt.h io/fastio.h \
							io/hpackio.h lza/model.h lza/model2.h lza/model3.h \
							lza/model4.h lza/lza.c
							$(CC) $(CFLAGS) lza/lza.c

$(OUTPATH)model$(OBJ):		defs.h error.h flags.h hpacklib.h io/hpackio.h \
							lza/model.h lza/model.c
							$(CC) $(CFLAGS) lza/model.c

$(OUTPATH)model2$(OBJ):		defs.h lza/model2.h lza/model2.c
							$(CC) $(CFLAGS) lza/model2.c

$(OUTPATH)model3$(OBJ):		defs.h lza/model3.h lza/model3.c
							$(CC) $(CFLAGS) lza/model3.c

$(OUTPATH)model4$(OBJ):		defs.h lza/model4.h lza/model4.c
							$(CC) $(CFLAGS) lza/model4.c

$(OUTPATH)pack$(OBJ):		defs.h io/hpackio.h io/fastio.h lza/pack.c
							$(CC) $(CFLAGS) lza/pack.c

$(OUTPATH)unpack$(OBJ):		defs.h crc/crc16.h io/hpackio.h io/fastio.h \
							lza/model.h lza/unpack.c
							$(CC) $(CFLAGS) lza/unpack.c

$(OUTPATH)store$(OBJ):		defs.h error.h hpacklib.h system.h crc/crc16.h \
							crypt/crypt.h io/fastio.h io/hpackio.h \
							store/store.h store/store.c
							$(CC) $(CFLAGS) store/store.c

$(PROJ)$(EXE):	$(OUTPATH)arcdir$(OBJ) $(OUTPATH)arcdirio$(OBJ) \
				$(OUTPATH)archive$(OBJ) $(OUTPATH)cli$(OBJ) \
				$(OUTPATH)display$(OBJ) $(OUTPATH)error$(OBJ) \
				$(OUTPATH)fastio$(OBJ) $(OUTPATH)filesys$(OBJ) \
				$(OUTPATH)frontend$(OBJ) $(OUTPATH)hpaktext$(OBJ) \
				$(OUTPATH)script$(OBJ) $(OUTPATH)tags$(OBJ) \
				$(OUTPATH)viewfile$(OBJ) $(OUTPATH)wildcard$(OBJ) \
				$(OUTPATH)crc16$(OBJ) $(OUTPATH)crypt$(OBJ) \
				$(OUTPATH)md5$(OBJ) $(OUTPATH)mdc$(OBJ) \
				$(OUTPATH)rsa$(OBJ) $(OUTPATH)unix$(OBJ) \
				$(OUTPATH)lza$(OBJ) $(OUTPATH)model$(OBJ) \
				$(OUTPATH)model2$(OBJ) $(OUTPATH)model3$(OBJ) \
				$(OUTPATH)model4$(OBJ) $(OUTPATH)pack$(OBJ) \
				$(OUTPATH)unpack$(OBJ) $(OUTPATH)store$(OBJ)
		@$(LD) $(LFLAGS) $(OUTPATH)arcdir$(OBJ) $(OUTPATH)arcdirio$(OBJ) \
		$(OUTPATH)archive$(OBJ) $(OUTPATH)cli$(OBJ) $(OUTPATH)display$(OBJ) \
		$(OUTPATH)error$(OBJ) $(OUTPATH)fastio$(OBJ) $(OUTPATH)filesys$(OBJ) \
		$(OUTPATH)frontend$(OBJ) $(OUTPATH)hpaktext$(OBJ) \
		$(OUTPATH)script$(OBJ) $(OUTPATH)tags$(OBJ) $(OUTPATH)viewfile$(OBJ) \
		$(OUTPATH)wildcard$(OBJ) $(OUTPATH)crc16$(OBJ) $(OUTPATH)crypt$(OBJ) \
		$(OUTPATH)md5$(OBJ) $(OUTPATH)mdc$(OBJ) $(OUTPATH)rsa$(OBJ) \
		$(OUTPATH)unix$(OBJ) $(OUTPATH)store$(OBJ) $(OUTPATH)lza$(OBJ) \
		$(OUTPATH)model$(OBJ) $(OUTPATH)model2$(OBJ) $(OUTPATH)model3$(OBJ) \
		$(OUTPATH)model4$(OBJ) $(OUTPATH)pack$(OBJ) $(OUTPATH)unpack$(OBJ)

#****************************************************************************
#*																			*
#*						Defines for each variation of Unix					*
#*																			*
#****************************************************************************

# AIX for the RS6000: Use cc.  Differs from AIX370/386 in endianness

aix:
		@$(MAKE) hpack CMDC="-DAIX" CC="cc"

# AIX370, AIX386: Use cc

aix370:
		@$(MAKE) hpack CMDC="-DAIX370" CC="cc"
aix386:
		@$(MAKE) hpack CMDC="-DAIX386" CC="cc"

# AUX: The only Unix so bloated it has /etc.etc.etc

# Generic: Generic BSD-ish system running gcc

generic:
		@$(ECHO)
		@$(ECHO) "You should set up your processor endianness in defs.h before compiling"
		@$(ECHO) "HPACK if you want to use the encryption functions."
		@$(MAKE) hpack CMDC="-DGENERIC" CC="gcc"

# Irix: Use cc with the -acpp flag for maximum ANSI-ness

irix:
		@$(MAKE) hpack CMDC="-acpp -DIRIX" CC="cc"

# ISC Unix: Use gcc

isc:
		@$(MAKE) hpack CMDC="-DISC" CC="gcc"

# Linux: Use gcc

linux:
		@$(MAKE) hpack CMDC="-DLINUX" CC="gcc"

# MiNT: Use gcc

mint:
		@$(MAKE) hpack CMDC="-DMINT" CC="gcc"

# QNX 4.x: Use cc -ml, no -lcurses, ld -N32768, -ml

qnx:
		@$(MAKE) hpack CMDC="-ml -DPOSIX" CMDL="-N32768 -ml" CC="cc"

# SunOs: Use gcc.  Don't use the Sun acc which breaks the decompressor

sun:
		@$(MAKE) hpack CMDC="-DSUNOS -fno-builtin" CC="gcc"

# SVR4: Use cc

svr4:
		@$(MAKE) hpack CMDC="-DSVR4" CC="cc"

# Ultrix (4.x or newer), Ultrix_old (3.x or older): Use vcc or gcc

ultrix:
		@$(MAKE) hpack CMDC="-DULTRIX" CC="gcc"
ultrix_old:
		@$(MAKE) hpack CMDC="-DULTRIX_OLD" CC="gcc"
