# Microsoft Developer Studio Generated NMAKE File, Based on PGPmemlock9x.dsp
!IF "$(CFG)" == ""
CFG=PGPmemlock9x - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PGPmemlock9x - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PGPmemlock9x - Win32 Release" && "$(CFG)" != "PGPmemlock9x - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlock9x.mak" CFG="PGPmemlock9x - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPmemlock9x - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPmemlock9x - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "PGPmemlock9x - Win32 Release"

OUTDIR=.\release
INTDIR=.\release
# Begin Custom Macros
OutDir=.\release
# End Custom Macros

ALL : "$(OUTDIR)\PGPmemlock.vxd"


CLEAN :
	-@erase "$(INTDIR)\pgpMemLock.obj"
	-@erase "$(OUTDIR)\PGPmemlock.vxd"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /Od /I "..\.." /I "$(98DDKROOT)\inc\win98" /D "BLD_COFF" /D "IS_32" /D "_X86_" /D "WIN40COMPAT" /Fo"$(INTDIR)\\" /Gs /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PGPmemlock9x.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=./release/pgpMLdevdcl.obj vxdwraps.clb /nologo /incremental:no /pdb:"$(OUTDIR)\PGPmemlock.pdb" /machine:IX86 /nodefaultlib /def:"pgpMemLock.def" /out:"$(OUTDIR)\PGPmemlock.vxd" /libpath:"$(98DDKROOT)\lib\i386\free" /vxd 
LINK32_OBJS= \
	"$(INTDIR)\pgpMemLock.obj"

"$(OUTDIR)\PGPmemlock.vxd" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   c:\_d\win98ddk\bin\win98\ml /coff /DBLD_COFF /DIS_32 /W2 /c /Cx /DMASM6 /DWIN40COMPAT /Ic:\_d\win98ddk\inc\win98 /Fo./Release/ pgpMLdevdcl.asm
	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Compiling and adding VERSIONINFO to VXD
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PGPmemlock.vxd"
   copy .\release\PGPmemlock.vxd .\release\PGPml.vxd
	c:\_d\win98ddk\bin\win98\bin16\rc -r -ic:\_d\win98ddk\inc\win98\inc16 -fo.\Release\PGPml.res PGPml.rc
	c:\_d\win98ddk\bin\win98\adrc2vxd  .\release\PGPml.vxd .\release\PGPml.res
	copy .\release\PGPml.vxd .\release\PGPmemlock.vxd
	erase .\release\PGPml.vxd
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PGPmemlock9x - Win32 Debug"

OUTDIR=.\debug
INTDIR=.\debug
# Begin Custom Macros
OutDir=.\debug
# End Custom Macros

ALL : "$(OUTDIR)\PGPmemlock.vxd"


CLEAN :
	-@erase "$(INTDIR)\pgpMemLock.obj"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PGPmemlock.exp"
	-@erase "$(OUTDIR)\PGPmemlock.ilk"
	-@erase "$(OUTDIR)\PGPmemlock.pdb"
	-@erase "$(OUTDIR)\PGPmemlock.vxd"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W4 /Zi /Od /I "..\.." /I "$(98DDKROOT)\inc\win98" /D "BLD_COFF" /D "IS_32" /D "_X86_" /D "WIN40COMPAT" /D "_DEBUG" /D "DEBUG" /D "PGP_DEBUG" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Zl /Gs /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PGPmemlock9x.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=./debug/pgpMLdevdcl.obj vxdwraps.clb /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\PGPmemlock.pdb" /debug /machine:I386 /nodefaultlib /def:"pgpMemLock.def" /out:"$(OUTDIR)\PGPmemlock.vxd" /implib:"$(OUTDIR)\PGPmemlock.lib" /libpath:"$(98DDKROOT)\lib\i386\free" /vxd 
LINK32_OBJS= \
	"$(INTDIR)\pgpMemLock.obj"

"$(OUTDIR)\PGPmemlock.vxd" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   c:\_d\win98ddk\bin\win98\ml /coff /DBLD_COFF /DIS_32 /W2 /c /Cx /DMASM6 /DWIN40COMPAT /Ic:\_d\win98ddk\inc\win98 /Fo./debug/ pgpMLdevdcl.asm
	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Compiling and adding VERSIONINFO to VXD
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PGPmemlock.vxd"
   copy .\debug\PGPmemlock.vxd .\debug\PGPml.vxd
	c:\_d\win98ddk\bin\win98\bin16\rc -r -dPGP_DEBUG=1 -ic:\_d\win98ddk\inc\win98\inc16 -fo.\debug\PGPml.res PGPml.rc
	c:\_d\win98ddk\bin\win98\adrc2vxd  .\debug\PGPml.vxd .\debug\PGPml.res
	copy .\debug\PGPml.vxd .\debug\PGPmemlock.vxd
	erase .\debug\PGPml.vxd
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

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
!IF EXISTS("PGPmemlock9x.dep")
!INCLUDE "PGPmemlock9x.dep"
!ELSE 
!MESSAGE Warning: cannot find "PGPmemlock9x.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PGPmemlock9x - Win32 Release" || "$(CFG)" == "PGPmemlock9x - Win32 Debug"
SOURCE=.\pgpMemLock.c

"$(INTDIR)\pgpMemLock.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

