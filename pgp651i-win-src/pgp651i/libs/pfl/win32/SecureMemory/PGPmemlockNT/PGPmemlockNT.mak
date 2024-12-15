# Microsoft Developer Studio Generated NMAKE File, Based on PGPmemlockNT.dsp
!IF "$(CFG)" == ""
CFG=PGPmemlockNT - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PGPmemlockNT - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PGPmemlockNT - Win32 Release" && "$(CFG)" != "PGPmemlockNT - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlockNT.mak" CFG="PGPmemlockNT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPmemlockNT - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPmemlockNT - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPmemlockNT - Win32 Release"

OUTDIR=.\release
INTDIR=.\release
# Begin Custom Macros
OutDir=.\release
# End Custom Macros

ALL : "$(OUTDIR)\PGPmemlock.sys"


CLEAN :
	-@erase "$(INTDIR)\pgpMemLock.obj"
	-@erase "$(INTDIR)\pgpMemLock.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PGPmemlock.exp"
	-@erase "$(OUTDIR)\PGPmemlock.lib"
	-@erase "$(OUTDIR)\PGPmemlock.sys"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Gz /MTd /W3 /O2 /I "..\.." /I "$(NT4DDKROOT)\inc" /D "WIN32" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WINNT=1 /D DEVL=1 /D FPO=1 /D _DLL=1 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zel -cbstring /QIfdiv- /QIf /GF /Gs /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\pgpMemLock.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PGPmemlockNT.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=int64.lib ntoskrnl.lib hal.lib /nologo /entry:"DriverEntry@8" /dll /incremental:no /pdb:"$(OUTDIR)\PGPmemlock.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\PGPmemlock.sys" /implib:"$(OUTDIR)\PGPmemlock.lib" /libpath:"$(NT4DDKROOT)\lib\i386\free" /subsystem:native 
LINK32_OBJS= \
	"$(INTDIR)\pgpMemLock.obj" \
	"$(INTDIR)\pgpMemLock.res"

"$(OUTDIR)\PGPmemlock.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPmemlockNT - Win32 Debug"

OUTDIR=.\debug
INTDIR=.\debug
# Begin Custom Macros
OutDir=.\debug
# End Custom Macros

ALL : "$(OUTDIR)\PGPmemlock.sys"


CLEAN :
	-@erase "$(INTDIR)\pgpMemLock.obj"
	-@erase "$(INTDIR)\pgpMemLock.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PGPmemlock.exp"
	-@erase "$(OUTDIR)\PGPmemlock.lib"
	-@erase "$(OUTDIR)\PGPmemlock.pdb"
	-@erase "$(OUTDIR)\PGPmemlock.sys"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Gz /MTd /W3 /Zi /Od /Oy /Gy /I "..\.." /I "$(NT4DDKROOT)\inc" /D "WIN32" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WINNT=1 /D DEVL=1 /D FPO=1 /D _DLL=1 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D "_DEBUG" /D "DEBUG" /D "PGP_DEBUG" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zel -cbstring /QIfdiv- /QIf /GF /Gs /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\pgpMemLock.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PGPmemlockNT.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=int64.lib ntoskrnl.lib hal.lib /nologo /entry:"DriverEntry@8" /dll /incremental:no /pdb:"$(OUTDIR)\PGPmemlock.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\PGPmemlock.sys" /implib:"$(OUTDIR)\PGPmemlock.lib" /libpath:"$(NT4DDKROOT)\lib\i386\checked" /subsystem:native 
LINK32_OBJS= \
	"$(INTDIR)\pgpMemLock.obj" \
	"$(INTDIR)\pgpMemLock.res"

"$(OUTDIR)\PGPmemlock.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("PGPmemlockNT.dep")
!INCLUDE "PGPmemlockNT.dep"
!ELSE 
!MESSAGE Warning: cannot find "PGPmemlockNT.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PGPmemlockNT - Win32 Release" || "$(CFG)" == "PGPmemlockNT - Win32 Debug"
SOURCE=.\pgpMemLock.c

"$(INTDIR)\pgpMemLock.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pgpMemLock.rc

"$(INTDIR)\pgpMemLock.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

