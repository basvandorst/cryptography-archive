#
#  SecuDE Release 4.3 (GMD)
#
#
#    Copyright GMD Darmstadt
#
#                         NOTICE
#
#    Acquisition, use, and distribution of this module 
#    and related materials are subject to restrictions 
#    mentioned in each volume of the documentation.
#

#
#    Template of CONFIG.make for Linux
#

# ARCH selects rsa-long arithmetic assembler routines
# Possible values:
# Possible values:
# mc68apollo:   Apollo MC680x0 assembler
# mc68sun:      Sun MC680x0 assembler
# mc68munix:    PCS Munix V3 assembler
# sparc:        SUN SPARC assembler
# std:          C-routines 
# sun386:       Sun 386i assembler
# ms-dos:       MS-DOS assembler
ARCH        = std

# for SCA interface
SYSTEM	    = _BSD42_

# Number of bits of an integer:
DWLNG       = -DWLNG32

# for LITTLE_ENDIAN architecture (e.g. Intel):
#ENDIAN      = -DLITTLE_ENDIAN

# if ndbm database functions are available:
DBMVERS     = -DNDBM

# Check TOP !!:
# Path of the top-level directory of secude-4.3
TOP         = /usr/local/secude

# Check LDL!!:  Library path. See also ISODE-support below
LDL         =  -L$(TOP)/lib

# Check location of .af-db (see X.500-Support below for explanation)
AFDBFILE    = /usr/local/secude/.af-db/

# Select one of the compilers below:
CC          = gcc -ansi #-fPIC
#CC         = acc -PIC
#CC          = cc -PIC

LD          = ld
OPT         = -g
DD          =
INCL        = $(DD)../include 
INCLISODE   = -I$(ICINCLPATH) 
ICINCLPATH   = $(DD)../include/isode

DFLAGS      = $(DSCA)
IFLAGS      = -I$(INCL) $(INCLISODE) $(ISCA)  
AFLAGS      = 
CFLAGS	    = $(OPT) $(DFLAGS) -D__LINUX__ -D__MALLOC_0_RETURNS_NULL $(IFLAGS)
LFLAGS      = $(AFLAGS) $(LDL)
ARFLAGS     = ruv
BINDIR      = $(DD)../../bin

SL_OPTIONS  = -assert pure-text
SL_APPENDIX = .so
LIBDIR      = $(DD)../../lib
LIBSECUDE   = $(LIBDIR)/libsecude.a
LSECUDE     = -lsecude
LM          = -lm
LDBM        = -ldbm
LIB         = $(LSECUDE) $(LISODE) $(LDBM) $(LM)
VERSION     = 4.3

#---------SecuDE Version------------------------------------------------
#
DVERSION     = -DSECUDEVERSION=\"SecuDE-${VERSION}\"
#
#-----------------------------------------------------------------------

#---------Smartcard Support---------------------------------------------
#
# Set DSCA to -DSCA and ISCA, SCINITFILE and DSCINIT to the indicated 
# values for the use of the GMD/GAO SmartCard Application Package STARCOS 
# and the interface software contained in subdirectory sca:
#
#DSCA        = -DSCA
#ISCA        = -I$(DD)../include/sca

# default SC configuration file
#SCINITFILE  = $(TOP)/.scinit
#DSCINIT     = -DSCINIT=\"${SCINITFILE}\"
#SYSTEM      = SUN  # This is for the sca/t1 subdirectory only. 
#
# If you want to use software PSEs only, DSCA and LIBSCA must be empty.
# No additional software and hardware is required in this case,
# and the sca subdirectory is not made.
#-----------------------------------------------------------------------

#---------ISODE-Support-------------------------------------------------
#
# SecuDE needs two subsets of ISODE subroutines:
#  
# 1. Subroutines necessary for the SecuDE ASN.1 encoding/decoding 
#    functions, 
# 2. Subroutines necessary for the SecuDE X.500 DUA functionality 
#    required for the access to security related attributes (only
#    in case that X.500 support is required, see below).
#
# If you have already an ISODE-ICR1 installation on your system, you
# can use its libisode.a library when binding SecuDE utilities.
#
# For the case that you have no ICR1 installation on your system,
# SecuDE contains a subset of ICR1 sources in its src/isode sub-
# directory which comprises the first subset. With this subset it is 
# possible to install all SecuDE functions except the use of X.500 
# directories (see below) without a complete ICR1 installation.
#
# Set DISODE to -DSECISODE and unset LISODE if you have no ICR1 
# installation on your system; the SecuDE-subdirectory isode is made in 
# this case, and the corresponding functions are put into libsecude.a.
#
# If DISODE is not set and LISODE is set instead, the SecuDE subdirectory
# isode is not made, and the libraries of your ISODE installation must 
# be included, instead. Add the library path (where the ISODE libraries 
# can be found) to LDL above.
#
#LISODE      = -lisode
DISODE   = -DSECISODE
#
#-----------------------------------------------------------------------

#---------X.500-Support-------------------------------------------------
# 
# Obtaining public keys and certificates from other persons is normally
# done by X.500 directory access. This is provided by SecuDE, but
# requires a full ICR1/QUIPU installation on your system, i.e.
# the ISODE library libdsap.a is necessary.
# In this mode, utilities like psemaint, pkadd and revoke, for instance,
# interwork with an X.500 directory via X.500 DAP (Directory Access
# protocol). The X.500 directory is accessible via the af_dir_enter_* 
# and af_dir_retrieve_* functions (for instance af_dir_enter_Certificate(),
# af_dir_retrieve_Certificate()).
#
# As an alternative, SecuDE provides a local directory for that purpose.
# The local substitute is realized with ndbm in the Unix-directory .af-db,
# which can be placed anywhere in the Unix file system. It's pathname must
# be given to the programs through variable AFDBFILE in config/CONFIG.make.
# This local directory is accessible via corresponding af_afdb_enter_* and
# af_afdb_retrieve_* functions (for instance af_afdb_enter_Certificate(),
# af_afdb_retrieve_Certificate()), but no X.500 DAP is used.
#
# The decision whether an X.500 directory or the local substitute is used 
# is done either at compile time through variables DX500 and DAFDB
# in config/CONFIG.make (DX500 must be defined for the case of the 
# X.500 directory, DAFDB must be set to the pathname of the .af-db directory
# in case of the local substitute), or at run time (which makes only sense
# if both types of directories are generated at compile time, i.e. if all
# DX500 and DAFDB are defined).
#
# If both directory types are generated at compile time, public security
# information is always stored in the local substitute. If the file
# ${AFDBFILE}/X500 exists, such information is additionally stored in
# the X.500 directory. When retrieving information from a directory,
# the X.500 directory is used if ${AFDBFILE}/X500 exists, the local
# substitute is used otherwise.
#
# If X.500 is being used, DSTRONG can be used to indicate that directory
# access via DAP is done using strong authentication and signed operations.
# This requires, however, an enhanced QUIPU version (available from GMD).
# If only the standard ICR1/QUIPU version is available, DSTRONG
# must not be set.
#
# .....................................................................
# Use of a local (ndbm) database for storing and retrieving 
# public certificates (does not require ICR1 QUIPU):
#
DAFDB     = -DAFDBFILE=\"${AFDBFILE}\"
# .....................................................................
# Use of X.500 directories for storing and retrieving public 
# certificates (requires ICR1 QUIPU installation):
#
#DX500   = -DX500
#LISODE  = -lisode
#DISODE  =

#DSTRONG = -DSTRONG

# X500 requires to use the include files of the ISODE ICR1 distribution.
# Set ICINCLPATH to the location (absolute path) of the include 
# directory within the source tree of your IC installation,
# e.g. ICINCLPATH = /usr/local/ICR1/IC/isode/src/h
	
ICINCLPATH = /usr/local/ICR1/IC/isode/src/h

#-----------------------------------------------------------------------

#--------- Shared Libraries --------------------------------------------
# If you want to produce libsecude.a as shared library, set LIBSECUDESO
# to libsecude.so.43.1. Version number 43 indicates SecuDE 4.3.
# Otherwise, set LIBSECUDESO to static.
#
LIBSECUDESO = static
#LIBSECUDESO  = libsecude.so.43.1
SL_OPTIONS = -assert pure-text
#
#-----------------------------------------------------------------------

#----------- Imported .o files -----------------------------------------
# If you have imported .o files in lib/IMPORTS, set LIBIMPORTS to yes.
# Otherwise, comment it out.
#
# If you want to use SecuDE-4.3 on a Sun/4 with SunOS 4.1.1 and with
# shared libraries, set LIBIMPORTS to yes.
#
#LIBIMPORTS = yes
#
#-----------------------------------------------------------------------

#----------- Test of arithmetic routines -------------------------------
# If you are porting SecuDE to other platforms 
# Test when using C-rotines ( $ARCH == std )
DARITHTEST  = -DARITHMETIC_TEST
# Test when using assember-routines ( $ARCH != std )
#DARITHTEST  = -DARITHMETIC_TEST -DARITHMETIC_TEST_NOT_STD
#-----------------------------------------------------------------------
