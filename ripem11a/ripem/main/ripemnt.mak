# Nmake macros for building Windows 32-Bit apps

!include <ntwin32.mak>
RSAREF_DIR = ../rsaref/source
RSAREF_LIB = ../rsaref/test/rsaref.lib
RIPEM_ROUTINES = adduser.obj crackhed.obj derkey.obj keyder.obj getopt.obj getsys.obj hexbin.obj parsit.obj prencode.obj pubinfo.obj rdwrmsg.obj ripemmai.obj ripemsoc.obj strutil.obj keyman.obj list.obj bemparse.obj usage.obj usagemsg.obj 
LIBS = $(RSAREF_LIB)
LFLAGS_TUNE = /debug:partial /debugtype:coff \mstools\lib\cap.lib
LFLAGS_DEBUG = /debug:full /debugtype:cv
LFLAGS = $(LFLAGS_DEBUG)
CFLAGS_ALL = -DWINNT /I..\rsaref\source -Za -G4
CFLAGS_DEBUG = /Zi
CFLAGS_RELEASE = /Gs /Ot 
CFLAGS= $(CFLAGS_ALL) $(CFLAGS_RELEASE)


all: ripem.exe


ripem.exe: $(RIPEM_ROUTINES) $(RSAREF_LIB)
    $(link) $(conflags) $(LFLAGS) $(RIPEM_ROUTINES) -out:ripem.exe  $(conlibs) $(LIBS)
