# PGPmemlock.mak - makefile for VxD driver for secure memory locking
!IF "$(CFG)" == ""
CFG=PGPmemlock - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PGPmemlock - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PGPmemlock - Win32 Release" && "$(CFG)" != "PGPmemlock - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlock.mak" CFG="PGPmemlock - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPmemlock - Win32 Release"
!MESSAGE "PGPmemlock - Win32 Debug"
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
CC=cl.exe
LINK32=link.exe

DEF_FILE= 	.\Source\pgpMemLock.def

# edit these two lines to reflect your MASM and DDK installations
AS=c:\masm611\bin\ml
DDKPATH=c:\ddk

# edit this line to point to your 16bit version of the resource
# compiler.  MUST BE THE 16BIT VERSION!!!  The 16bit version comes
# with the Win32 SDK in the BINW16 directory

!IF  "$(CFG)" == "PGPmemlock - Win32 Release"

OBJPATH=.\Release
AFLAGS= -coff -DBLD_COFF -DIS_32 -W2 -c -Cx -DMASM6 
CFLAGS= -c -Gs -Zdpl -Od -D_X86_ -YX -W3 -DBLD_COFF -DIS_32
CPPFLAGS= -c -Gs -Zdpl -Odb2 -D_X86_ -YX -W3 -DBLD_COFF -DIS_32 
RSCFLAGS= -r 
LINK32_FLAGS= \
-machine:i386 -def:"$(DEF_FILE)" -out:"$(OBJPATH)\PGPmemlock.vxd" \
-map:"$(OBJPATH)\PGPmemlock.map" \
-vxd $(DDKPATH)\lib\vxdwraps.clb -nodefaultlib

!ELSEIF  "$(CFG)" == "PGPmemlock - Win32 Debug"

OBJPATH=.\Debug
AFLAGS= -coff -DBLD_COFF -DIS_32 -W2 -c -Cx -DMASM6 -Zd -DDEBUG
CFLAGS= -c -Gs -Zdpl -Od -D_X86_ -YX -W3 -DDEBLEVEL=1 -DBLD_COFF -DDEBUG -DIS_32
CPPFLAGS= -c -Gs -Zdpl -Odb2 -D_X86_ -YX -W3 -DDEBLEVEL=1 -DBLD_COFF -DDEBUG -DIS_32 
RSCFLAGS= -r
LINK32_FLAGS= \
-machine:i386 -def:"$(DEF_FILE)" -out:"$(OBJPATH)\PGPmemlock.vxd" \
-debug -debugtype:map -map:"$(OBJPATH)\PGPmemlock.map" \
-vxd $(DDKPATH)\lib\vxdwraps.clb -nodefaultlib

!ENDIF 

ALL : "$(OBJPATH)\PGPmemlock.vxd"

CLEAN :
	-@erase "$(OBJPATH)\PGPmemlock.exp"
	-@erase "$(OBJPATH)\pgpMemLock.obj"
	-@erase "$(OBJPATH)\pgpMLdevdcl.obj"
	-@erase "$(OBJPATH)\PGPmemlock.pdb"
	-@erase "$(OBJPATH)\PGPmemlock.lib"
	-@erase "$(OBJPATH)\PGPmemlock.map"
	-@erase "$(OBJPATH)\PGPmemlock.pch"
	-@erase "$(OBJPATH)\PGPmemlock.vxd"
	-@erase "$(OBJPATH)\pgpML.res"

"$(OBJPATH)" :
    if not exist "$(OBJPATH)/$(NULL)" mkdir "$(OBJPATH)"


AS_PROJ = $(AFLAGS) -I$(DDKPATH)\inc32 -Fo"$(OBJPATH)\\"
CC_PROJ = $(CFLAGS) -I$(DDKPATH)\inc32 -I..\.. -Fo"$(OBJPATH)\\" -Fp"$(OBJPATH)\PGPmemlock.pch"
CPP_PROJ = $(CPPFLAGS) -I$(DDKPATH)\inc32 -I..\.. -Fo"$(OBJPATH)\\"	-Fp"$(OBJPATH)\PGPmemlock.pch"
RSC_PROJ = $(RSCFLAGS) -I$(DDKPATH)\inc16

LINK32_OBJS= \
	"$(OBJPATH)\pgpMLdevdcl.obj" \
	"$(OBJPATH)\pgpMemLock.obj"	


"$(OBJPATH)\PGPmemlock.vxd" : "$(OBJPATH)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
!IF "$(RSC)" != ""
	copy $(OBJPATH)\PGPmemlock.vxd $(OBJPATH)\PGPml.vxd
	$(DDKPATH)\bin\adrc2vxd $(OBJPATH)\PGPml.vxd $(OBJPATH)\pgpML.res
	copy $(OBJPATH)\PGPml.vxd $(OBJPATH)\PGPmemlock.vxd
	erase $(OBJPATH)\PGPml.vxd
!ENDIF

.asm{$(OBJPATH)}.obj::
   $(AS) @<<
   $(AS_PROJ) $< 
<<

.c{$(OBJPATH)}.obj::
   $(CC) @<<
   $(CC_PROJ) $< 
<<

.cpp{$(OBJPATH)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.rc{$(OBJPATH)}.res::
   $(RSC) @<<
   $(RSC_PROJ) $< 
<<


!IF "$(CFG)" == "PGPmemlock - Win32 Release" || "$(CFG)" == "PGPmemlock - Win32 Debug"

SOURCE=.\Source\pgpMLdevdcl.asm

!IF  "$(CFG)" == "PGPmemlock - Win32 Release"

DEP_CPP_PGPMEMLOCK=
	
"$(OBJPATH)\pgpMLdevdcl.obj" : $(SOURCE) $(DEP_CPP_PGPMEMLOCK) "$(OBJPATH)"
	$(AS) $(AS_PROJ) $(SOURCE)

!ELSEIF  "$(CFG)" == "PGPmemlock - Win32 Debug"

DEP_CPP_PGPMEMLOCK=
	
"$(OBJPATH)\pgpMLdevdcl.obj" : $(SOURCE) $(DEP_CPP_PGPMEMLOCK) "$(OBJPATH)"
	$(AS) $(AS_PROJ) $(SOURCE)

!ENDIF 


SOURCE=.\Source\pgpMemLock.c

!IF  "$(CFG)" == "PGPmemlock - Win32 Release"

DEP_CPP_PGPMEMLOCK=\
	"..\..\pgpMemLockWin32.h"
	
"$(OBJPATH)\pgpMemLock.obj" : $(SOURCE) $(DEP_CPP_PGPMEMLOCK) "$(OBJPATH)"
	$(CC) $(CC_PROJ) $(SOURCE)

!ELSEIF  "$(CFG)" == "PGPmemlock - Win32 Debug"

DEP_CPP_PGPMEMLOCK=\
	"..\..\pgpMemLockWin32.h"
	
"$(OBJPATH)\pgpMemLock.obj" : $(SOURCE) $(DEP_CPP_PGPMEMLOCK) "$(OBJPATH)"
	$(CC) $(CC_PROJ) $(SOURCE)

!ENDIF 

!IF  "$(RSC)" != ""

SOURCE=.\Source\pgpML.rc

!IF  "$(CFG)" == "PGPmemlock - Win32 Release"

DEP_RSC_PGPMEMLOCK=\
	".\Source\pgpVer.h"
	
"$(OBJPATH)\pgpML.res" : $(SOURCE) $(DEP_RSC_PGPMEMLOCK) "$(OBJPATH)"
	$(RSC) $(RSC_PROJ) -Fo"$(OBJPATH)\pgpML.res" $(SOURCE)

!ELSEIF  "$(CFG)" == "PGPmemlock - Win32 Debug"

DEP_RSC_PGPMEMLOCK=\
	".\Source\pgpVer.h"
	
"$(OBJPATH)\pgpML.res" : $(SOURCE) $(DEP_RSC_PGPMEMLOCK) "$(OBJPATH)"
	$(RSC) $(RSC_PROJ) -Fo"$(OBJPATH)\pgpML.res" $(SOURCE)

!ENDIF 

!ENDIF

!ENDIF 

