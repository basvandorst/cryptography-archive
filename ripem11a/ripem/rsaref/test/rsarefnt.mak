# This is a MAKEFILE for Microsoft's NMAKE
!include <ntwin32.mak>

# extension for object files
O = obj

# commands
CC = cl
LIB = lib32
ASM = masm

# name of temporary library script
TEMPFILE  = $(TEMP)\temp.mak
TEMPFILE2 = $(TEMP)\temp2.mak

# standard include directory
STDINCDIR = c:\mstools\include

# The places to look for include files (in order).
INCL =  -I. -I$(SRCDIR) -I$(STDINCDIR)

# Normal C flags.
CFLAGS_ALL = -W3 /G4 $(INCL) -c -DPROTOTYPES=1 -DUSE_386_ASM -DUSE_2MODEXP
CFLAGS_DEBUG = -Zi
CFLAGS_RELEASE = /Ox /Oi 
CFLAGS =  $(CFLAGS_ALL) $(CFLAGS_RELEASE)
LFLAGS = 

# The location of the common source directory.
SRCDIR = ..\source\#
SRCLIB = rsaref.lib

# The location of the demo source directory.
RDEMODIR = ..\rdemo\#

$(SRCLIB) : desc.$(O) digit.$(O) digitas.$(O) md2c.$(O) md5c.$(O) nn.$(O) prime.$(O)\
  rsa.$(O) r_encode.$(O) r_enhanc.$(O) r_keygen.$(O) r_random.$(O)\
  r_stdlib.$(O)
	$(LIB) /out:$@ desc.$(O) digit.$(O) digitas.$(O) md2c.$(O) md5c.$(O) nn.$(O) prime.$(O)\
  		rsa.$(O) r_encode.$(O) r_enhanc.$(O) r_keygen.$(O) r_random.$(O)\
  		r_stdlib.$(O)

notused:
  del $(TEMPFILE)
  del $(TEMPFILE2)
  if NOT EXIST $@ $(LIB) /out:$@;
  !echo /remove:$?  >> $(TEMPFILE)
  !echo $? >> $(TEMPFILE2)
  rem echo ;' >> $(TEMPFILE)
  echo ;' >> $(TEMPFILE2)
  $(LIB)  $@ @$(TEMPFILE)	/out:$@ 
  $(LIB)  $@ @$(TEMPFILE2) /out:$@


!INCLUDE $(SRCDIR)targets.mak
